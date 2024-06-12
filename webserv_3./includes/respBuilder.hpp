
#ifndef RESPBUILDER_HPP
#define RESPBUILDER_HPP


#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h> // for signal handling
#include <sys/time.h> // for timeval
#include "reqHandler.hpp"
#include "multiPlex.hpp"

class Response {
    public :
        char **env;
        int cgi_status;
        std::stringstream cgi_data;
        bool cgi_resp_start;
        int ep_fd;
        int pipfd[2];
        pid_t c_pid;
        pid_t w_pid;
        clock_t cgi_start;
        bool folder;
        bool endOfCGI;
        bool cgi_on;
        std::ifstream fl;
        int endOfResp;
        int cliSock;
        ReqHandler  *req;
        std::map<std::string, std::string> cType;
        Response( ReqHandler *_req, int _cliSock, int &ep_fd_ );
        ~Response();
        void    getMethod();
        void    execute_cgi();
        void    child_proc();
        bool    is_cgi();
        int         DELETE(std::string& path);
        std::string     cgi_response();
        std::string  read_from_a_file();
        // std::string  read_from_a_pipe();
        std::string  list_folder();
        std::string ret_folder();
        std::string get_file_ext(std::string path);
        std::string getHdResp();
        void    env_init();
        int full_access(std::string path);
        
};

#endif