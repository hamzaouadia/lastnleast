
#include "includes/fconf.hpp"

std::vector<Serv> Fconf::getServers()
{
    if ( myServers.size() )
        return myServers;
    else
    {
        std::cerr << "Error Config file has no working Server" << std::endl;
        std::cerr << "Run server 0 arg for default config file" << std::endl;
        exit (1);
    }
}

int Fconf::empLine( std::string line )
{
    for ( int i = 0; line[i] ; i++ )
        if ( !std::isspace( line[i] ) )
            return 0;
    return 1;
}

int commentLine( std::string line )
{
    size_t i = 0;
    for ( ; i < line.size() ; i++ )
        if ( !std::isspace( line[i] ) )
            break;
    if ( line[i] == '#' )
        return 1;
    return 0;
}

int    hostCheck( std::string &str )
{
    int dot = 0;
    for ( size_t i = 0 ; i < str.size() ; i++ )
        if ( str[i] == '.' )
            dot++;
    if ( dot > 3 || str[0] == '.' )
        return 0;
    std::vector<std::string> g;
    std::istringstream ss( str );
    std::string tok;
    while ( std::getline( ss, tok, '.' ) )
        g.push_back( tok );
    if ( g.size() < 4 )
        return 0;
    for ( size_t i = 0 ; i < g.size() ; i++ )
    {
        if ( g[i].size() > 3 )
            return 0;
        for ( size_t j = 0 ; j < g[i].size() ; j++ )
            if ( !std::isdigit( g[i][j] ) )
                return 0;
        if ( atoi( g[i].c_str() ) > 255 || atoi( g[i].c_str() ) < 0 )
            return 0;
    }
    return 1;
}

int numParse( std::string num )
{
    for ( int i = 0 ; num[i] ; i++ )
        if ( !std::isdigit( num[i] ) )
            return (-1);
    return ( std::atoi( num.c_str() ) );
}

std::vector<std::string> split_uri( std::string &str )
{
    std::vector<std::string>    ret;
    std::stringstream           ss( str );
    std::string                 elem;

    while ( std::getline( ss, elem, '/' ) )
    {
        if ( !elem.size() )
            continue;
        ret.push_back( elem );
    }
    return ret;
}

bool isDirectory(const std::string& path)
{
    struct stat statResult;
    if ( stat(path.c_str(), &statResult) == 0 )
        return S_ISDIR(statResult.st_mode);
    return false;
}

int     Fconf::serverDupCheck( Serv &myserv )
{
    if ( !myServers.size() )
        return 0;
    std::vector<Serv>::iterator it = myServers.begin();
    for (  ; it != myServers.end() ; it++ )
        if ( it->port == myserv.port && it->host == myserv.host && it->servName == myserv.servName )
            return 1;
    return 0;
}

int find_root_loc( std::vector<s_location> &g )
{
    size_t i = 0;
    for (  ; i < g.size() ; i++ )
        if ( g[i].name == "/" )
            return 1;
    return 0;
}

void    root_loc_ifndef( Serv &myserv )
{
    if ( !myserv.locations.size() || !find_root_loc( myserv.locations ) )
    {
        s_location g;
        g.name = "/";
        g.get = "off";
        g.post = "off";
        g.del = "off";
        g.root = myserv.root;
        myserv.locations.push_back( g );
    }
}

void Fconf::checkServ( std::vector<std::string> &serv )
{
    Serv myserv(myServers);
    myserv.limit = 100;
    std::map<std::string, int> flag_s;
    flag_s.insert(std::make_pair("port", 0));
    flag_s.insert(std::make_pair("host", 0));
    flag_s.insert(std::make_pair("servername", 0));
    flag_s.insert(std::make_pair("root", 0));
    std::string word;

    try
    {
        for ( size_t j = 0 ; j < serv.size() ; j++ )
        {
            std::istringstream stream( serv[j]);
            stream >> word;
            if ( word == "port" && !flag_s[word] )
            {
                myserv.set_port(stream);
                flag_s[word] = 1;
            }
            else if ( word == "host" && !flag_s[word] )
            {
                myserv.set_host(stream);
                flag_s[word] = 1;
            }
            else if ( word == "servername" && !flag_s[word] )
            {
                myserv.set_servName(stream);
                flag_s[word] = 1;
            }
            else if ( word == "root" && !flag_s[word] )
            {
                myserv.set_root(stream);
                flag_s[word] = 1;
            }
            else if ( word == "limit" && !flag_s[word] )
            {
                myserv.set_limit(stream);
                flag_s[word] = 1;
            }
            else if ( word == "errorpage" )
                myserv.set_errorpage(stream);
            else if (word == "location")
            {
                try
                {
                    myserv.set_locations(serv, j);
                }
                catch(const std::exception& e)
                {
                    while ( word != ")" && j < serv.size() ) // if this is not found it keep going and break the other location
                    {
                        std::istringstream adv( serv[j] );
                        adv >> word;
                        j++;
                    }
                    std::cerr << e.what() << std::endl;
                    j--;
                }
            }
            else
                throw( std::out_of_range("Invalid element in this server or repeated element"));
        }
        if ( !flag_s["port"] || !flag_s["host"] || !flag_s["root"] )
            throw( std::out_of_range("Element missing in a server"));
        // default params should be set before pushing the server in case something is missing
        if ( !serverDupCheck( myserv ) )
        { // if myserv doesnt exist
            root_loc_ifndef( myserv );
            myServers.push_back( myserv );
            // std::cout << "all good the server is pushed to the container of servers" << std::endl;
        }
    }
    catch ( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
    }
}

void    Fconf::extractServer()
{
    int start = 0;
    if ( !mydata.size() )
        throw ( std::out_of_range("Empty file?") );
    if ( mydata.back() != "}" )
        throw ( std::out_of_range( "Error syntax config file" ) );
    while ( mydata.size() > 0 )
    {
        if ( !start )
        {
            if ( mydata.front() != "server{" )
                throw ( std::out_of_range( "Error syntax config file" ));
            start = 1;
            mydata.pop_front();
            continue ;
        }

        if ( mydata.front() == "}" )
        {
            checkServ( tmp );
            start = 0;
            mydata.pop_front();
            while ( tmp.size() > 0 )
                tmp.pop_back();
            continue ;
        }
        tmp.push_back( mydata.front() );
        mydata.pop_front();
    }
}

void    Fconf::getData( std::ifstream &configf )
{
    std::string line;
    if ( configf.peek() == EOF )
    {
        std::cerr << "file empty" << std::endl;
        return ;
    }
    while ( std::getline( configf, line ) )
    {
        
        if ( empLine( line ) || commentLine( line ) )
            continue;

        mydata.push_back( line );
    }
    try {
        extractServer();
    }
    catch ( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
    }
}

void    dontGoBeforeMe( std::string &r, std::string &locr, int s )
{
    std::vector<std::string>    vr = split_uri( r );
    std::vector<std::string>    vlocr = split_uri( locr );
    if ( !vr.size() )
        return;
    if ( (!vlocr.size() && vr.size()) || (vlocr.size() < vr.size()) )
    {
        if ( !s )
            locr = "";
        else
            locr = r;
        return;
    }
    for ( size_t i = 0 ; i < vr.size() ; i++ )
    {
        if ( vr[i] != vlocr[i] )
        {
            if ( !s )
                locr = "";
            else
                locr = r;
            return;
        }
    }
}

void    uploadPvalidation( std::string &locr, std::string &upl, std::string &p )
{

    if ( !upl.size() )
    {
        p = "off";
        return;
    }
    if ( upl[0] != '/' )
        upl = "/" + upl;
    std::string str = locr + upl;
    char    rPath[PATH_MAX];
    if ( realpath( str.c_str(), rPath ) != NULL )
    {
        upl = rPath;
        dontGoBeforeMe( locr, upl, 1 );
    }
    else
        p = "off";
}

void    cgiPathsValidation( std::string &r, std::string &cgi )
{
    if ( !cgi.size() )
        return;
    if ( cgi[0] != '/' )
        cgi = "/" + cgi;
    std::string str = r + cgi;
    char rPath[PATH_MAX];
    if ( realpath( str.c_str(), rPath ) != NULL )
    {
        cgi = rPath;
        dontGoBeforeMe( r, cgi, 0 );
    }
    else
        cgi = "";
}

Fconf::Fconf ( const char *file )
{
    std::ifstream configf;
    configf.open( file );
    if ( !configf.is_open() )
    {
        std::cerr << "Failed to open" << std::endl;
        exit(1);
    }
    getData( configf );
    for ( size_t i = 0 ; i < myServers.size() ; i++ )
    {
        for ( size_t j = 0 ; j < myServers[i].locations.size() ; j++ )
        {
            dontGoBeforeMe( myServers[i].root,
                            myServers[i].locations[j].root, 1 );

            uploadPvalidation(  myServers[i].locations[j].root,
                                myServers[i].locations[j].upload_path,
                                myServers[i].locations[j].post );
            
            cgiPathsValidation( myServers[i].locations[j].root,
                                myServers[i].locations[j].CGI_PHP );

            cgiPathsValidation( myServers[i].locations[j].root,
                                myServers[i].locations[j].CGI_PY );
        }
    }
    configf.close();
}








// Location Class functions down below

Serv::Serv(std::vector<Serv>& _myServers): myServers(_myServers)
{
    // defErrP[400] = "../../Desktop/webServ2.6/errPages/err400.html";
    // defErrP[401] = "../../Desktop/webServ2.6/errPages/err401.html";
    // defErrP[402] = "../../Desktop/webServ2.6/errPages/err402.html";
    // defErrP[403] = "../../Desktop/webServ2.6/errPages/err403.html";
    // defErrP[404] = "../../Desktop/webServ2.6/errPages/err404.html";
    // defErrP[405] = "../../Desktop/webServ2.6/errPages/err405.html";
    // defErrP[406] = "../../Desktop/webServ2.6/errPages/err406.html";
    // defErrP[407] = "../../Desktop/webServ2.6/errPages/err407.html";
    // defErrP[408] = "../../Desktop/webServ2.6/errPages/err408.html";
    // defErrP[409] = "../../Desktop/webServ2.6/errPages/err409.html";
    // defErrP[410] = "../../Desktop/webServ2.6/errPages/err410.html";
    // defErrP[411] = "../../Desktop/webServ2.6/errPages/err411.html";
    // defErrP[412] = "../../Desktop/webServ2.6/errPages/err412.html";
    // defErrP[413] = "../../Desktop/webServ2.6/errPages/err413.html";
    // defErrP[414] = "../../Desktop/webServ2.6/errPages/err414.html";
    // defErrP[415] = "../../Desktop/webServ2.6/errPages/err415.html";
    // defErrP[416] = "../../Desktop/webServ2.6/errPages/err416.html";
    // defErrP[417] = "../../Desktop/webServ2.6/errPages/err417.html";
    // defErrP[418] = "../../Desktop/webServ2.6/errPages/err418.html";
    // defErrP[421] = "../../Desktop/webServ2.6/errPages/err421.html";
    // defErrP[422] = "../../Desktop/webServ2.6/errPages/err422.html";
    // defErrP[423] = "../../Desktop/webServ2.6/errPages/err423.html";
    // defErrP[424] = "../../Desktop/webServ2.6/errPages/err424.html";
    // defErrP[425] = "../../Desktop/webServ2.6/errPages/err425.html";
    // defErrP[426] = "../../Desktop/webServ2.6/errPages/err426.html";
    // defErrP[428] = "../../Desktop/webServ2.6/errPages/err428.html";
    // defErrP[429] = "../../Desktop/webServ2.6/errPages/err429.html";
    // defErrP[431] = "../../Desktop/webServ2.6/errPages/err431.html";
    // defErrP[500] = "../../Desktop/webServ2.6/errPages/err500.html";
    // defErrP[501] = "../../Desktop/webServ2.6/errPages/err501.html";
}

int checkRealPath( std::string &str, int p )
{
    char    rPath[PATH_MAX];
    if ( realpath( str.c_str(), rPath ) != NULL )
    {
        char    rPwd[PATH_MAX];
        if ( getcwd( rPwd, sizeof( rPwd ) ) == NULL )
        {
            std::cerr << "getcwd Error" << std::endl;
            return 0;
        }
        std::string s = rPath;
        std::string pwd = rPwd;
        // std::vector<std::string> vc = split_uri( s );
        // std::vector<std::string> vcPwd = split_uri( pwd );
        if ( p && !isDirectory( s ) ) //|| vc.size() < 3 || vc.front() != "nfs" )
            return 0;
        if ( s == rPwd )
            return 0;
        // if ( (vc.size() == vcPwd.size() && vc.back() == vcPwd.back())
        //     || (vc.size() > vcPwd.size() && vc[vcPwd.size() - 1] == vcPwd.back())
        //     || (vc.size() < vcPwd.size()) )
        //     return 0;
        // if ( vc.size() >= 3 && vcPwd.size() >= 3 )
        // {
        //     for ( int i = 0 ; i < 3 ; i++ )
        //         if ( vc[i] != vcPwd[i] )
        //             return 0;
        // }
        // else
        //     return 0;
        str = rPath;
        return 1;
    }
    else
        return 0;
}

void Serv::set_errorpage(std::istringstream& _errorpage)
{
    int error;
    std::string page;
    _errorpage >> error >> page >> word;
    if ( page == "" || _errorpage || !checkRealPath( page, 0 ) )
        throw (std::out_of_range("Invalid errorpage in this server"));
    this->errorpage[error] = page;
}


void Serv::set_limit(std::istringstream& _limit)
{
    _limit >> this->limit;
    _limit >> word;
    if ( this->limit >= ULLONG_MAX )
    {
        this->limit = 10000;
        return ;
    }
    if ( this->limit <= 0 || _limit )
        throw (std::out_of_range("Invalid limit in this server"));
}

void    Serv::set_port( std::istringstream& _port )
{
    _port >> this->port;
    _port >> word;
    if ( this->port < 1024 || this->port > 49151 || _port )
        throw (std::out_of_range("Invalid port in this server"));
}

void    Serv::set_host(std::istringstream& _host)
{
    _host >> this->host;
    _host >> word;
    if (_host || !this->host.size() || !hostCheck( this->host ) ) //
        throw (std::out_of_range("Invalid host in this server"));
}

void    Serv::set_servName( std::istringstream &_servName )
{
    _servName >> this->servName;
    _servName >> word;
    if (_servName || !this->servName.size() ) // || !alNumStr( this->servName )
        throw (std::out_of_range("Invalid servername in this server"));
}

void    Serv::set_root(std::istringstream& _root)
{
    _root >> this->root;
    _root >> word;
    if (_root || !this->root.size() || !checkRealPath( this->root, 1 ) )
        throw (std::out_of_range("Invalid root in this server"));
}

int Serv::locDupCheck( s_location &loc )
{
    for( size_t i = 0 ; i < locations.size() ; i++ )
        if ( locations[i].name == loc.name )
            return 0;
    return 1;
}

std::string    loc_set_help( std::map<std::string, int>& flag_s, std::istringstream& tmp, std::string &word, int f )
{
    std::string str;
    std::string rest;
    flag_s[word] = 1;
    tmp >> str;
    tmp >> rest;
    if ( !f )
    {
        if (tmp || !str.size())
            throw (std::out_of_range("Invalid " + word + " in this location"));
    }
    else
    {
        if (tmp || !str.size() || (str != "on" && str != "off"))
            throw (std::out_of_range("Invalid " + word + " in this location"));
    }
    return str;
}

int    locNameCheck( std::string &str )
{
    if ( !str.size() )
        return 0;
    if ( str[0] != '/' )
        return 0;
    for ( size_t i = 1 ; i < str.size() ; i++ )
    {
        if ( str[i] == '/' )
            return 0;
    }
    return 1;
}

void    Serv::set_locations( std::vector<std::string>& serv, size_t& j )
{
    std::istringstream _location(serv[j++]);
    s_location save;
    save.post = "off";
    save.del = "off";
    save.get = "off";
    _location >> word;
    _location >> save.name;
    if ( !locNameCheck( save.name ) )
        throw (std::out_of_range("Invalid location name"));
    _location >> word;
    if ( word != "(" )
        throw (std::out_of_range("Invalid input in this location"));
    _location >> word;
    if ( _location )
        throw (std::out_of_range("Invalid location in this location"));
    std::map<std::string, int>flag_s;
    flag_s.insert(std::make_pair("get", 0));
    flag_s.insert(std::make_pair("post", 0));
    flag_s.insert(std::make_pair("delete", 0));
    flag_s.insert(std::make_pair("auto_index", 0));
    flag_s.insert(std::make_pair("root", 0));
    flag_s.insert(std::make_pair("redirection", 0));
    flag_s.insert(std::make_pair("index", 0));
    flag_s.insert(std::make_pair("CGI_path", 0));
    flag_s.insert(std::make_pair("upload_path", 0));
    while ( j < serv.size() && word != ")" )
    {
        // std::cout << "line : " << serv[j] << std::endl;
        std::istringstream tmp( serv[j++] );
        tmp >> word;
        if (word == "root" && !flag_s[word])
        {
            save.root = loc_set_help( flag_s, tmp, word, 0 );
            if ( !checkRealPath( save.root, 1 ) )
                throw ( std::out_of_range("the root in this location : " + save.name + " is Invalid !") );
            // if ( !save.upload_path.size() )
            //     save.upload_path = save.root;
        }
        else if (word == "get" && !flag_s[word])
            save.get = loc_set_help( flag_s, tmp, word, 1 );
        else if (word == "post" && !flag_s[word])
            save.post = loc_set_help( flag_s, tmp, word, 1 );
        else if (word == "delete" && !flag_s[word])
            save.del = loc_set_help( flag_s, tmp, word, 1 );
        else if (word == "auto_index" && !flag_s[word])
            save.autoidx = loc_set_help( flag_s, tmp, word, 1 );
        else if (word == "redirection" && !flag_s[word])
            save.redirection = loc_set_help( flag_s, tmp, word, 0 );
        else if (word == "index" && !flag_s[word])
            save.index = loc_set_help( flag_s, tmp, word, 0 );
        else if (word == "CGI_PHP" && !flag_s[word])
            save.CGI_PHP = loc_set_help( flag_s, tmp, word, 0 );
        else if (word == "CGI_PY" && !flag_s[word])
            save.CGI_PY = loc_set_help( flag_s, tmp, word, 0 );
        else if (word == "CGI_BASH" && !flag_s[word])
            save.CGI_BASH = loc_set_help( flag_s, tmp, word, 0 );
        else if (word == "upload_path" && !flag_s[word])
        {
            save.upload_path = loc_set_help( flag_s, tmp, word, 0 );
            if ( !checkRealPath( save.upload_path, 1 ) && save.root.size() )
                save.upload_path = save.root;
                // char    rPwd[PATH_MAX];
                // getcwd( rPwd, sizeof( rPwd ) );
                // save.upload_path = rPwd;
        }
        else if (word != ")")
            throw (std::out_of_range("Invalid element in this location"));
    }
    if (!flag_s["root"] || word != ")")
        throw (std::out_of_range("Missing element in this location"));
    if ( !locDupCheck( save ) )
        throw (std::out_of_range("duplicated location"));
    this->locations.push_back(save);
    j--;
}