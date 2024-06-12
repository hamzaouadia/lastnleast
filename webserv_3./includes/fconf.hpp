
#ifndef FCONF_HPP
#define FCONF_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <map>
#include <deque>
#include <string.h>
#include <sstream>
#include <exception>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

int     numParse(std::string num);
bool    isDirectory(const std::string& path);
std::vector<std::string> split_uri( std::string &str );

typedef struct location {
    std::string autoidx;
    std::string get;
    std::string post;
    std::string del;
    std::string name;
    std::string root;
    std::string redirection;
    std::string index;
    std::string CGI_PHP;
    std::string CGI_PY;
    std::string CGI_BASH;
    std::string upload_path;
} s_location;

class Serv {
    public:
        int port;
        unsigned long long limit;
        std::string servName;
        std::string host;
        std::string root;
        std::string word;
        std::map<int, std::string> errorpage;
        std::map<int, std::string> defErrP;
        std::vector<Serv> myServers;
        std::vector<location> locations;
        Serv(){};
        Serv(std::vector<Serv>& _myServers);
        void set_port(std::istringstream& _port);
        void set_host(std::istringstream& _host);
        void set_servName(std::istringstream& _servName);
        void set_root(std::istringstream& _root);
        void set_limit(std::istringstream& _limit);
        void set_errorpage(std::istringstream& _errorpage);
        void set_locations( std::vector<std::string>& serv, size_t& j );
        int  locDupCheck( s_location &loc );
};

class Fconf {
    private :
        std::vector<std::string>    tmp;
        std::deque<std::string>     mydata;
        std::vector<Serv>           myServers;        
    public :
        Fconf( const char *file );
        void    getData( std::ifstream &configf );
        int     empLine( std::string line );
        void    extractServer();
        void    checkServ( std::vector<std::string> &serv );
        int     serverDupCheck( Serv &myserv );
        std::vector<Serv> getServers();
};

#endif