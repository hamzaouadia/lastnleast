
#ifndef REQHANDLER_HPP
#define REQHANDLER_HPP

#include "fconf.hpp"
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <string.h>
#include <dirent.h>
#include <netinet/in.h>
#include <fcntl.h>

// extern int read_size;
std::string decodeUri( std::string str );
int checkUrirPath( std::string &str );
int checkUri( std::string &str );
int find_me( char c );
int hexaToDeci( std::string &s );

typedef struct reqFinalForm {
    int         status;
    std::string method;
    std::string uri;
} s_reqFinalForm;

class ReqHandler {
    
    public:
        std::vector<std::string>  reqHds;
        std::ofstream   pFile;
        std::string     body_string;
        std::string     rest;
        std::string     non_body_str;
        std::string     value;
        std::string     cType;
        std::string     host;
        std::string     cookie;
        std::string     query;
        std::string     pathInfo;
        std::string     fName;
        double  clock_out;
        bool    passedOnce;
        bool    bodyStartFound;
        int     g;
        int     end_of_chunk;
        int     loc_idx;
        int     endOfRead;
        int     read_size;
        unsigned long long  chunk_size;
        unsigned long long  size_counter;
        unsigned long long  bigScounter;
        unsigned long long  content_lenght;
        unsigned long long  bytes_red;
        
        Serv                                myServ;
        std::vector<Serv>                   servs;
        s_reqFinalForm                      request;
        std::vector<std::string>            req;
        std::map<std::string, std::string>  reqHeaders;

        // methods
        ReqHandler();
        ReqHandler( std::vector<Serv> &_myServ );
        ~ReqHandler();
        std::string fNameGenerator();
        s_location  getLocationByName( std::string &str );
        std::string getFullUri( std::vector<std::string> &spl_uri, std::string &root, int i );
        Serv        getServer();
        void        pFileOpener();
        void        deleteFile();
        void        cLenght_post( std::string &str );
        void        tChunked_post( std::string &str );
        void        checkBuff( char *buff, size_t bytes );
        void        nextBuff( char *buff, size_t bytes );
        void        reqStrToVec( std::string &line );
        void        fillReqHeaders();
        void        countBodyBytes( std::string &str );
        void        parse_request();
        void        uri_depon_cs( int code );
        void        isAllowed( s_location &loc, std::vector<std::string> &splited_uri, int i );
        void        checkRetIdx();
        void        getFinalUri( std::string str );
        void        storeQuery();
        int         getHeaderVal( std::string key, std::string &val );
        int         parseHeaders();
        int         iStillValid();
        int         isLocation( std::string &str );
};

#endif