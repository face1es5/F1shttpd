#include <iostream>
using namespace std;
class Server{
public:
    Server();
    void startup(int port);
    int getPort();
    int getSock();
    static void accept_request(int client);
    static string get_line(int sock);
    static void serve_file(int client,string path);
    static void execute_cgi(int client,string path,string method,string query);
    static void not_found(int client,string path);
    static void unimplemented(int client);
    static void send_before_entity(int client,int status);
    static void bad_request(int clinet);
    static void cannot_exec(int clinet);
private:
    int port;
    int _sock;
};