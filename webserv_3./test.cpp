#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
using namespace std;

vector<string> split( string s )
{
    istringstream ss( s );
    string tok;
    vector<string> v;
    while ( ss >> tok )
        v.push_back( tok );
    return v;
}

void    doIt( string &res, string p )
{
    const string s = p.substr( 0, p.size() - 1 );
    int num = stoi( s );
    for ( int i = 0 ; i < num ; i++ )
        res.push_back( p[ p.size() - 1 ] );
}

int main()
{
    string str;
    getline(cin, str);
    vector<string> g = split(str);
    string res;
    for ( int i = 0 ; i < g.size() ; i++ )
        doIt( res, g[i] );
    if ( res.size() )
        cout << res << endl;
    else
        cout << "NONE" << endl;
}