
#include "includes/respBuilder.hpp"


std::string Response::ret_folder()
{
    std::vector<std::string> rt;
    if ( req->loc_idx == -1 )
        rt = split_uri( req->myServ.root );
    else
        rt = split_uri( req->myServ.locations[req->loc_idx].root );
    size_t pos = req->request.uri.rfind(rt.back());
    std::string fldr = req->request.uri.substr(pos + rt.back().size(), req->request.uri.size());
    fldr == "" ? fldr : fldr += "/";
    return fldr;
}

std::string     Response::list_folder()
{
    std::stringstream response;
    std::stringstream res;
    std::string old_p;
    struct dirent *entry;
    DIR *dir = opendir(req->request.uri.c_str());

    if (dir == NULL)
      return "";
    response << "<!DOCTYPE html><html><head><title> listing a folder </title></head><body>";
    while ((entry = readdir(dir)) != NULL)
    {
        old_p = ret_folder();
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..")
            continue;
        response << "<div>" << "<a href=\"";
        if (req->loc_idx != -1)
            if (req->myServ.locations[req->loc_idx].name != "/")
                response << req->myServ.locations[req->loc_idx].name << "/";
        response << old_p << entry->d_name << "\">" << entry->d_name << "</a>" << "</div>";
    }
    response << "</body>";
    closedir(dir);
    endOfResp = 1;
    res << "Content-Length: " << response.str().size() << "\r\n\r\n" << response.str();
    return res.str();
}

void    Response::env_init()
{
    std::stringstream ss;
    std::stringstream ln;
    
    std::string gatewayInterface = "GATEWAY_INTERFACE=CGI/1.1";/**/
    std::string queryString      = "QUERY_STRING=";/**/
    std::string redirectstatus   = "REDIRECT_STATUS=200";/**/
    std::string requestMethod    = "REQUEST_METHOD=";/**/
    std::string scriptName       = "SCRIPT_FILENAME=";/**/
    std::string serverName       = "SERVER_NAME=localhost";/**/
    std::string serverPort       = "SERVER_PORT=";/**/
    std::string serverProtocol   = "SERVER_PROTOCOL=HTTP/1.1";/**/
    std::string pathInfo         = "PATH_INFO=";/**/
    std::string cookie           = "HTTP_COOKIE=";/**/
    std::string contentLength    = "CONTENT_LENGTH=";
    std::string contentType      = "CONTENT_TYPE=";


    if (req->request.method == "POST")
    {
        this->env = (char **)malloc(sizeof(char *) * 13);
        ln << req->content_lenght;
        ss << serverPort << req->myServ.port;
    
        this->env[0] = strdup((requestMethod + req->request.method).c_str());/**/
        this->env[1] = strdup((queryString + req->query).c_str());
        this->env[2] = strdup((redirectstatus).c_str());
        this->env[3] = strdup((scriptName + req->request.uri).c_str());
        this->env[4] = strdup((serverName).c_str());/**/
        this->env[5] = strdup(ss.str().c_str()); /**/  
        this->env[6] = strdup((gatewayInterface).c_str());/**/
        this->env[7] = strdup((serverProtocol).c_str());/**/
        this->env[8] = strdup((pathInfo + req->pathInfo).c_str());/**/
        this->env[9] = strdup((cookie + req->cookie).c_str());/**/
        this->env[10] = strdup((contentLength + ln.str()).c_str());/**/
        this->env[11] = strdup((contentType + req->cType).c_str());/**/
        this->env[12] = NULL;
    }
    else if (req->request.method == "GET")
    {
        this->env = (char **)malloc(sizeof(char *) * 11);
        ss << serverPort << req->myServ.port;

        this->env[0] = strdup((requestMethod + req->request.method).c_str());/**/
        this->env[1] = strdup((queryString + req->query).c_str());
        this->env[2] = strdup((redirectstatus).c_str());
        this->env[3] = strdup((scriptName + req->request.uri).c_str());
        this->env[4] = strdup((serverName).c_str());/**/
        this->env[5] = strdup(ss.str().c_str()); /**/  
        this->env[6] = strdup((gatewayInterface).c_str());/**/
        this->env[7] = strdup((serverProtocol).c_str());/**/
        this->env[8] = strdup((pathInfo + req->pathInfo).c_str());/**/
        this->env[9] = strdup((cookie + req->cookie).c_str());/**/
        this->env[10] = NULL;
    }

}

void Response::child_proc()
{
    char *str[3];
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = pipfd[0];
    int g = epoll_ctl( ep_fd, EPOLL_CTL_ADD, pipfd[0], &ev );
    if (g == -1)
        throw std::runtime_error("CGI execution failed");
    close(pipfd[0]);
    dup2(pipfd[1], STDOUT_FILENO);
    close(pipfd[1]);
    if (req->request.method == "POST")
    {
        if (req->pFile.is_open())
            req->pFile.close();
        int fd = open( req->fName.c_str() ,std::ios::in | std::ios::binary);
        dup2(fd, 0);
        close (fd);
    }
    if (req->request.uri.find(".php") != std::string::npos)
        str[0] = strdup("/usr/bin/php-cgi");
    else if (req->request.uri.find(".py") != std::string::npos)
        str[0] = strdup("/usr/bin/python3");
    str[1] = strdup(req->request.uri.c_str());
    str[2] = NULL;
    if (execve(str[0], str, env) == -1)
        throw std::runtime_error("CGI execution failed");
    for (int i = 0; i < 3; ++i)
    {
        if (str[i] != NULL)
            free(str[i]);
    }
    exit(127);
}

void Response::execute_cgi()
{
    char buffer[1024];
    std::string bdy;
    memset(buffer, 0, 1024);
    
    if (pipe(pipfd) == -1)
        throw std::runtime_error("CGI execution failed");

    env_init();
    c_pid = fork();
    cgi_start = clock();
    if (c_pid == -1)
        throw std::runtime_error("CGI execution failed");
    else if (c_pid == 0)
        child_proc();
    else
    {
        w_pid = waitpid(c_pid, &cgi_status, WNOHANG);
        if (w_pid == c_pid)
        {
            if (WIFEXITED(cgi_status))
                throw std::runtime_error("CGI execution failed");
            else
                throw std::runtime_error("CGI execution failed");
        }
    }
    close(pipfd[1]);
}

std::string  Response::cgi_response()
{
    std::stringstream response;
    std::string hdrs;

    const int chunkSize = 1024;
    char buffer[chunkSize];
    memset(buffer, 0, chunkSize);

    if (cgi_resp_start != true)
    {
        cgi_resp_start = true;
        response << "HTTP/1.1 ";
        if ( cgi_data.str().find("Status: ") != std::string::npos )
        {
            response << cgi_data.str().substr(cgi_data.str().find("Status: ") + 8, 3) << "\r\n";
            hdrs = cgi_data.str().substr(0, cgi_data.str().find("\r\n\r\n") + 4);
            response << hdrs;
            endOfResp = 1;
            return response.str();
        }
        else
        {
            response << "200 OK\r\n";
            if (req->request.uri.find(".py") != std::string::npos)
            {
                response << "Content-Type: text/html\r\n";
                response << "Content-Length: " << cgi_data.str().size() << "\r\n\r\n";
            }
        }
        return response.str();
    }
    cgi_data.read(buffer, chunkSize);
    if (cgi_data.gcount())
        response.write(buffer, cgi_data.gcount());
    else
        endOfResp = 1;
    return response.str();
}


std::string     Response::read_from_a_file()
{
    std::stringstream response;

    const int chunkSize = 1024;
    char buffer[chunkSize];
    memset(buffer, 0, chunkSize);

    if (!fl.is_open())
        fl.open(req->request.uri.c_str(), std::ios::binary);
    if (fl.is_open())
    {
        fl.read(buffer, chunkSize - 1);
        if (fl.gcount() > 0)
            response.write(buffer, fl.gcount());
        else
        {
            response << "\r\n";
            endOfResp = 1;
            fl.close();
        }
    }
    else
    {
        response << "This File is not Found! Please Check the your default files.";
        endOfResp = 1;
        return response.str();
    }
    return response.str();
}

std::string Response::get_file_ext(std::string path)
{
    if (folder)
        return cType["html"];
    size_t pos = path.rfind(".");

    if (pos == std::string::npos)
        return cType["txt"];
    return cType[path.substr(pos + 1, path.size())];
}


bool    Response::is_cgi()
{
    if (req->loc_idx == -1 || (req->request.uri.find(".php") == std::string::npos && req->request.uri.find(".py") == std::string::npos))
        return false;
    std::string cgi = req->request.uri.substr(req->request.uri.rfind(".") + 1, req->request.uri.size());

    std::string cgi_path = cgi == "php" ? req->myServ.locations[req->loc_idx].CGI_PHP : req->myServ.locations[req->loc_idx].CGI_PY;


    std::string path = cgi_path;

    if (req->request.uri.find(path) == 0 && req->loc_idx >= 0 && cgi_path.size())
        return true;
    return false;
}

int    Response::DELETE(const std::string& path)
{
    struct stat st;
    std::string fullPath;

    if (folder == false)
    {
        if (access(path.c_str(), W_OK) == 0)
            unlink(path.c_str());
        else
            return 1;
        return 0;
    }

    DIR* dir = opendir(path.c_str());
    if (!dir)
        return 1;

    dirent* entry;
    while ((entry = readdir(dir)))
    {
        if (std::string(entry->d_name) != "." && std::string(entry->d_name) != "..")
        {
            fullPath = path + "/" + entry->d_name;
            if (lstat(fullPath.c_str(), &st) == 0)
            {
                if (S_ISDIR(st.st_mode))
                {
                    if (access(fullPath.c_str(), W_OK) == 0)
                    {
                        if (DELETE(fullPath))
                            return 1;
                    }
                    else
                        return 1;
                }
                else
                {
                    if (access(fullPath.c_str(), W_OK) == 0)
                        unlink(fullPath.c_str());
                    else
                        return 1;
                }
            }
            else
                return 1;
        }
    }
    closedir(dir);
    if (access(path.c_str(), W_OK) == 0)
        rmdir(path.c_str());
    else
        return 1;
    return 0;
}

int root_access(std::string &path)
{
    std::vector<std::string> rt = split_uri(path);

    std::string root;

    for (size_t i = 1; i < rt.size(); ++i)
    {
        root += "/" + rt[i];
        if (access(root.c_str(), R_OK) != 0) 
            return 1;

    }
    return 0;
}

std::string Response::getHdResp()
{
    struct stat statbuf;
    std::stringstream response;
    stat( req->request.uri.c_str(), &statbuf );
    if (req->request.method == "DELETE" && req->request.status == 200)
    {
        if ( access(req->request.uri.c_str(), F_OK) )
        {
            req->uri_depon_cs( 404 );
            return response.str();
        }
        if (stat(req->request.uri.c_str(), &statbuf) == 0)
        {
            if (S_ISDIR(statbuf.st_mode))
                folder = true;
            else
                folder = false;
        }
        if ( DELETE(req->request.uri) )
            req->uri_depon_cs( 403 );
        else
        {
            response << "HTTP/1.1 " << 204 << "No Content";
            response << "\r\n\r\n";
            endOfResp = 1;
        }
        return response.str();
    }
    if (req->request.status == 301)
    {
        response << "HTTP/1.1 " << req->request.status << " Moved Permanently\r\n";
        response << "Location: " << req->myServ.locations[req->loc_idx].redirection;
        response << "\r\n\r\n";
        endOfResp = 1;
        return response.str();
    }
    if ( access(req->request.uri.c_str(), F_OK) )
        req->uri_depon_cs( 404 );
    else if ( statbuf.st_mode & S_IRUSR )
    {
        if (statbuf.st_mode & S_IFDIR)
            folder = true;
        else if( statbuf.st_mode & S_IFREG )
        {
            folder = false;
            if (is_cgi() == true && folder == false)
            {
                cgi_on = true;
                try
                {
                    execute_cgi();
                }
                catch(const std::exception& e)
                {
                    struct stat statbuf;
                    req->uri_depon_cs(500);
                    if ( access(req->request.uri.c_str(), F_OK) )
                        req->uri_depon_cs( 404 );
                    stat( req->request.uri.c_str(), &statbuf );
                    response << "HTTP/1.1 " << req->request.status << " OK\r\n";
                    response << "Content-Type: text/html\r\n";
                    response << "Content-Length: ";
                    response << statbuf.st_size;
                    response << "\r\n";
                    response << "\r\n";
                    cgi_on = false;
                    endOfResp = 0;
                }
                return response.str();
            }
        }
    }
    else
        req->uri_depon_cs( 403 );
    stat( req->request.uri.c_str(), &statbuf );
    response << "HTTP/1.1 " << req->request.status << " OK\r\n";
    response << "Content-Type: ";
    response << get_file_ext(req->request.uri) << "\r\n";
    if (folder)
        return response.str();
    response << "Content-Length: ";
    if ( access(req->request.uri.c_str(), F_OK) )
        response << 522;
    else
        response << statbuf.st_size;
    response << "\r\n\r\n";
    return response.str();
}

void    Response::getMethod()
{
    std::string hdRes = getHdResp();
    ssize_t bytesSent = send( cliSock, hdRes.c_str(), hdRes.size(), 0);
    if ( (int)bytesSent == -1 )
        endOfResp = 1;
}

Response::Response( ReqHandler *_req, int _cliSock, int &ep_fd_ )
{
    ep_fd = ep_fd_;
    cliSock = _cliSock;
    endOfResp = 0;
    req = _req;
    folder = false;
    endOfCGI = false;
    cgi_on = false;
    cgi_resp_start = false;
    cType[""] = "text/html";
    cType["php"] = "text/html";
    cType["sh"] = "text/html";
    cType["py"] = "text/html";
    cType["txt"] = "text/html";
    cType["cpp"] = "text/html";
    cType["hpp"] = "text/html";
    cType["html"] = "text/html";
    cType["jpg"] = "image/jpeg";
    cType["jpeg"] = "image/jpeg";
    cType["png"] = "image/png";
    cType["gif"] = "image/gif";
    cType["pdf"] = "application/pdf";
    cType["doc"] = "application/msword";
    cType["xls"] = "application/vnd.ms-excel";
    cType["ppt"] = "application/vnd.ms-powerpoint";
    cType["zip"] = "application/zip";
    cType["tar"] = "application/x-tar";
    cType["gz"] = "application/gzip";
    cType["wav"] = "audio/wav";
    cType["mp3"] = "audio/mpeg";
    cType["mp4"] = "video/mp4";
    cType["avi"] = "video/x-msvideo";
    cType["json"] = "application/json";
    cType["xml"] = "application/xml";
    cType["css"] = "text/css";
    cType["js"] = "application/javascript";
    cType["bmp"] = "image/bmp";
    cType["ico"] = "image/x-icon";
    cType["svg"] = "image/svg+xml";
    cType["tiff"] = "image/tiff";
    cType["woff"] = "application/font-woff";
    cType["woff2"] = "application/font-woff2";
    cType["eot"] = "application/vnd.ms-fontobject";
    cType["o"] = "application/octet-stream";

    getMethod();
}

Response::~Response()
{
    // std::cout << " Response destructor called" << std::endl;
}