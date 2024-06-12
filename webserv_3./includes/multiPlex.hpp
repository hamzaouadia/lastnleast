
#ifndef MULTIPLEX_HPP
#define MULTIPLEX_HPP

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include "reqHandler.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include "respBuilder.hpp"

class Response;

class MultiPlexer {
    private:
        int epollFd;
        std::map<int, int>          serv_cli;
        std::map<int, sockaddr_in>  socknData;
    public:
        // bool pipe_closed;
        MultiPlexer( std::vector<Serv> &servers );
        ~MultiPlexer();
        std::vector<Serv>    getServBySock( int sock, std::vector<Serv> &servers );
        void    addSockToEpoll( int sockToAdd );
        void    delSockFrEpoll( int sockToDel );
        void    webServLoop( std::vector<Serv> &servers );
        void    acceptCli( int fd, std::vector<Serv> &servers, std::map<int, ReqHandler*> &reqMap );
        int     existentSockForPort( int &nport );
        int     isFdServer( int fd );
        int     spotIn( int fd, ReqHandler* obj, std::map<int, ReqHandler*> &reqMap );
        int     spotOut( int fd, ReqHandler* obj, std::map<int, Response*> &resMap, std::map<int, ReqHandler*> &reqMap );

        std::string read_from_a_pipe(int fd, bool &pipe_closed, int &c_pid, std::string &method);
};

#endif