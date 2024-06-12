
#include <cstring>
#include "includes/multiPlex.hpp"

std::string randomStr()
{
    std::string s = "abcdefghijklmnopqrstuvwxyzZYXWVUTSRQPONMLKJIHGFEDCBA0123456789";
    int         sSize = sizeof(s) - 1;
    std::string ret;
    srand( static_cast<unsigned int>(time(0)));
    for ( int i = 0 ; i < 8 ; i++ )
        ret.push_back( s[rand() % sSize]);
    return ret;
}

void    lcs_num( std::vector<Serv> servers )
{
    for ( size_t i = 0 ; i < servers.size() ; i++ )
    {
        int n = 0;
        for ( size_t j = 0 ; j < servers[i].locations.size() ; j++ )
            n++;
        // std::cout << "server " << i + 1 << " has : " << n << " location" << std::endl;
    }
}

int main( int ac, char **av )
{        
    try
    {
        std::vector<Serv> servers;
        if ( ac > 2 )
            std::cerr << "wrong number of arguments!" << std::endl;
        else if ( ac == 2 )
        {
            Fconf   configFile( av[1] );
            servers = configFile.getServers();
            if ( !servers.size() )
                throw std::runtime_error( "No correct server found, re-check your config file" );
            lcs_num ( servers );
            MultiPlexer mplex( servers );
            mplex.webServLoop( servers );
        }
        else
        {
            std::string name = randomStr() + ".config";
            std::ofstream f( name.c_str() );
            if ( !f.is_open() )
                throw std::runtime_error( "Error oppening the default config file" );
            f << "server{\n";
            f << "port 5555\n";
            f << "host 127.0.0.1\n";
            f << "servername reda1\n";
            f << "root ../../Desktop/www\n";
            f << "limit 1000000\n";
            f << "}\n";
            f.close();
            Fconf   configFile( name.c_str() );
            servers = configFile.getServers();
            MultiPlexer mplex( servers );
            mplex.webServLoop( servers );
        }
    }
    catch( const std::exception& e )
    {
        std::cerr << e.what() << std::endl;
    }
    return 1;
}