
#include "includes/reqHandler.hpp"

// TOOLS START

std::string ReqHandler::fNameGenerator()
{
    std::map<std::string, std::string> cType_ext;
    cType_ext["text/plain"] = ".txt";
    cType_ext["text/html"] = ".html";
    cType_ext["text/css"] = ".css";
    cType_ext["application/javascript"] = ".js";
    cType_ext["application/json"] = ".json";
    cType_ext["application/xml"] = ".xml";
    cType_ext["application/pdf"] = ".pdf";
    cType_ext["application/msword"] = ".doc";
    cType_ext["application/vnd.ms-excel"] = ".xls";
    cType_ext["application/vnd.ms-powerpoint"] = ".ppt";
    cType_ext["application/zip"] = ".zip";
    cType_ext["application/x-tar"] = ".tar";
    cType_ext["application/gzip"] = ".gz";
    cType_ext["image/jpeg"] = ".jpg";
    cType_ext["image/png"] = ".png";
    cType_ext["image/gif"] = ".gif";
    cType_ext["image/bmp"] = ".bmp";
    cType_ext["audio/wav"] = ".wav";
    cType_ext["audio/mpeg"] = ".mp3";
    cType_ext["video/mp4"] = ".mp4";
    cType_ext["video/x-msvideo"] = ".avi";
    
    std::string s = "abcdefghijklmnopqrstuvwxyzZYXWVUTSRQPONMLKJIHGFEDCBA0123456789";
    int         sSize = sizeof(s) - 1;
    
    srand( static_cast<unsigned int>(time(0)));
    for ( int i = 0 ; i < 30 ; i++ )
        fName.push_back( s[rand() % sSize]);
    std::map<std::string, std::string>::iterator it = cType_ext.find( cType );
    if ( it != cType_ext.end() )
        fName += it->second;
    if ( loc_idx == -1 )
        fName = myServ.root + "/" + fName + ".txt";
    else
        fName = myServ.locations[loc_idx].upload_path + "/" + fName + ".txt";
    return fName;
}

void    ReqHandler::pFileOpener()
{
    if ( bodyStartFound )
    {
        std::string fName = fNameGenerator();
        pFile.open( fName.c_str(), std::ios::binary );//std::ios::out | 
        if ( !pFile.is_open() )
        {
            std::cerr << "Error could not open file to post !"<< std::endl;
            exit(1);
        }
        if ( value == "chunked" )
            tChunked_post( body_string );
        else
            cLenght_post( body_string );
    }
}

long long getSize( std::string str )
{
    if ( !str.size() )
        return -1;
    std::istringstream ss( str );
    long long ret;
    ss >> std::hex >> ret;
    // if ( ret == 0 )
    //     std::cerr << "getSize : " <<str << std::endl;
    // std::cerr << "ret here : " << ret << std::endl;
    return ret;
}

int getPos( std::string &str )
{
    char seq[3] = "\r\n";
    size_t pos = str.find( seq );
    if ( pos != std::string::npos )
        return pos;
    else
        return -1;
}

void    ReqHandler::deleteFile()
{
    if ( std::remove( fName.c_str() ) != 0 )
        std::cerr << "Error deleting the posted file" << std::endl;
}

int ReqHandler::iStillValid()
{
    if (access(fName.c_str(), F_OK) != 0)
    {
        std::cerr << "File does not exist." << std::endl;
        return 0;
    }
    if (access(fName.c_str(), W_OK) != 0)
    {
        std::cerr << "No write permission for the file." << std::endl;
        return 0;
    }
    return 1;
}

// TOOLS END

void        ReqHandler::cLenght_post( std::string &str )
{
    // std::cerr << "in CLENGHT" << std::endl;
    // std::cerr << str << std::endl;
    if ( !iStillValid() )
        return ( uri_depon_cs( 409 ) );
    // std::cout << "str.size() in cLenght POST : " << str.size() << std::endl;
    // std::string cgi = request.uri.substr(request.uri.rfind(".") + 1, request.uri.size());
    //     // std::cerr << "print CGI : " << cgi << std::endl;
    // if ( cgi != "php" && cgi != "py" )
    // {
    //     endOfRead = 1;
    //     request.status = 201;
    //     return ;
    // }
    unsigned long long old_bytes_red = bytes_red;
    bytes_red += str.size();
    // std::cout << "          ++++++++++++++" << std::endl;
    // std::cout << "cont_len = " << content_lenght << std::endl;
    // std::cout << "before increm bytes_red = " << old_bytes_red << std::endl;
    // std::cout << "bytes_red = " << bytes_red << std::endl;
    // std::cout << "          ++++++++++++++" << std::endl;

    if ( bytes_red > myServ.limit )
    {
        deleteFile();
        return ( uri_depon_cs( 413 ) );
    }
    if ( bytes_red > content_lenght )
    {
        std::string tmp = str.substr( 0, content_lenght - old_bytes_red );
        pFile.write( tmp.c_str(), tmp.size() );
    }
    else
        pFile.write( str.c_str(), str.size() );
    if ( bytes_red >= content_lenght )
    {
        endOfRead = 1;
        request.status = 201;
        std::string cgi = request.uri.substr(request.uri.rfind(".") + 1, request.uri.size());
        // std::cerr << "print CGI : " << cgi << std::endl;
        if ( cgi != "php" && cgi != "py" )
            request.uri = "../../Desktop/webServ2.6/success.html";
    }
}

void        ReqHandler::tChunked_post( std::string &str )
{
    std::cerr << "in CHUNKED" << std::endl;
    std::cerr << str << std::endl;
    while ( 50 )
    {
        if ( !iStillValid() )
            return ( uri_depon_cs( 409 ) );
        // std::string cgi = request.uri.substr(request.uri.rfind(".") + 1, request.uri.size());
        // // std::cerr << "print CGI : " << cgi << std::endl;
        // if ( cgi != "php" && cgi != "py" )
        // {
        //     endOfRead = 1;
        //     request.status = 201;
        //     return ;
        // }
        if ( g )
        {
            g = 0;
            str.erase( 0, 2 );
            if ( !str.size() )
                return;
        }
        if ( !end_of_chunk )
        {
            int p = getPos( str );
            if ( p == -1 )
                return ( uri_depon_cs( 500 ) );
            chunk_size = getSize( str.substr( 0, p ) );
            if ( (int)chunk_size == -1 )
                return ( uri_depon_cs( 500 ) );
            if ( !chunk_size )
            {
                endOfRead = 1;
                request.status = 201;
                std::string cgi = request.uri.substr(request.uri.rfind(".") + 1, request.uri.size());
                // std::cerr << "print CGI : " << cgi << std::endl;
                if ( cgi != "php" && cgi != "py" )
                    request.uri = "../../Desktop/webServ2.6/success.html";
                return;
            }
            p += 2;
            str.erase( 0, p );
            read_size = 1024;
        }
        size_counter += str.size();
        bigScounter += size_counter;
        if ( bigScounter > myServ.limit )
        {
            deleteFile();
            return ( uri_depon_cs( 413 ) );
        }
        if ( size_counter < chunk_size )
        {
            pFile.write( str.c_str(), str.size() );
            str.erase( 0, str.size() );
            if ( chunk_size - size_counter < 1023 )
                read_size = chunk_size - size_counter + 1;
            end_of_chunk = 1;
            return ;
        }
        else if ( size_counter > chunk_size )
        {
            std::string s = str.substr( 0, chunk_size );
            pFile.write( s.c_str(), s.size() );
            str.erase( 0, chunk_size );
            g = 1;
            end_of_chunk = 0;
            size_counter = 0;
        }
        else
        {
            pFile.write( str.c_str(), str.size() );
            str.clear();
            g = 1;
            end_of_chunk = 0;
            size_counter = 0;
            read_size = 1024;
            return ;
        }
    }
}
