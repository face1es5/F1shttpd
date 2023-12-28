#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cctype>
#include "httpd.h"

#define MaxCharNum 1024
#define SERVER_STRING "Server: F1SHhttpd/0.0\r\n"
#define ROOTPATH "html"
#define S_ISXUSR(m) (((m) & S_IFMT) == S_IXUSR)
#define S_ISXGRP(m) (((m) & S_IFMT) == S_IXGRP)
#define S_ISXOTH(m) (((m) & S_IFMT) == S_IXOTH)

using namespace std;

/**
 * @def print error message and exit.
 * @param s msg to print
*/
void error_die(const string s){
    perror(s.c_str());
    exit(1);
}

/**
 * @def do nothing...
*/
Server::Server(){}

/**
 * @brief get server's port
 * @return port
*/
int Server::getPort(){
    return this->port;
}

/**
 * @brief return server socket's file descriptor
 * @return socket fd
*/
int Server::getSock(){
    return this->_sock;
}

/**
 * @brief init socket and start a process of listening on a specified port.
 * @param port the port to listen on, if port is 0, then allocate a port dynamically.
*/
void Server::startup(int port){
    //assign port to class variable
    this->port=port;

    int socketfd=0; //socket descriptor
    struct sockaddr_in server;    //internet socket address
    /**
     * @param domain AF_INET - ipv4 protocols
     * @param socket_type STREAM - sequenced, reliable, two-way, connection-based.(tcp, i think)
     * @param protocol if 0, select protocol automatically according to socket type(here is tcp).
    */
    socketfd=socket(AF_INET,SOCK_STREAM,0);  //create a net socket with tcp/ip protocol.
    if (-1 == socketfd)   //create socket failed.
        error_die("socket create failed.");

    //init socket struct var
    memset(&server,0,sizeof(server));
    server.sin_family=AF_INET;
    server.sin_port=htons(port);  //convert port to net byte order(big endian)
    server.sin_addr.s_addr=htonl(INADDR_ANY);   //accept connections to any of the machine's ip addresses

    //bind specified address to socket
    if (bind(socketfd,(struct sockaddr*)&server,sizeof(server))<0){
        error_die("bind failed.");
    } 
    //modify port dynamically
    if (0 == port){
        unsigned int addrlen=sizeof(server);
        if (getsockname(socketfd,(struct sockaddr*)&server,&addrlen)<0){
            error_die("getsockname failed.");
        }
        //assign to class variable
        this->port=ntohs(server.sin_port);
    }

    //listening on this port.
    if (-1 == listen(socketfd,10))
        error_die("listen failed.");

    this->_sock=socketfd;
}

/**
 * @brief read line from socket
 * linefeed can be \\n,\\r or \\r\\n, this function will make last valid char of return string is \\n
 * and terminate it with \\0.
 * if read a blank line, .e.g \\n, \\r, \\r\\n, it will returns "\\n"
 * @param sock socket fd
 * @return string to store data
*/
string Server::get_line(int sock){
    int i=0;
    char buf[MaxCharNum];   //stroe data from socket
    char c=0;   //store char from socket temporarily
    int n;
    while(i<MaxCharNum && c!='\n'){
        n=recv(sock,&c,1,0);
        if (1 == n){
            if (c=='\r'){
                //peek followed char
                n=recv(sock,&c,1,MSG_PEEK);
                if (n==1 && c=='\n') //pop '\n' from socket
                    n=recv(sock,&c,1,0);
                else
                    c='\n';
            }
            buf[i++]=c;
        }else{  //no more char or error
            c='\n';
        }
    }
    buf[i]='\0'; //truncate str
    return string(buf);
}

/**
 * @brief send start line, response headers and a blank line \\r\\n
 * @param client client 's socket fd
 * @param status status code
*/
void Server::send_before_entity(int client, int status){
    string buf;
    //set start line
    switch(status){
        case 200:
            buf="HTTP/1.0 200 OK\r\n";
            break;
        case 404:
            buf="HTTP/1.0 404 Not Found\r\n";
            break;
        case 501:
            buf="HTTP/1.0 501 Not Implemented\r\n";
            break;
        default:
            buf="HTTP/1.0 200 OK\r\n";
            break;

    }
    //send start line
    send(client,buf.c_str(),buf.size(),0);
    //send header
    buf=string(SERVER_STRING);
    send(client,buf.c_str(),buf.size(),0);
    buf="Content-Type: text/html\r\n";
    send(client,buf.c_str(),buf.size(),0);
    buf="\r\n";
    send(client,buf.c_str(),buf.size(),0);
}

/**
 * @brief send 501 unimplemented msg to client
 * @param client client's socket fd
*/
void Server::unimplemented(int client){
    send_before_entity(client,501);
    string buf="<html><head><title>NOT IMPLEMENTED</title></head>";
    send(client,buf.c_str(),buf.size(),0);
    buf="<body>This method is not implemented.</body></html>";
    send(client,buf.c_str(),buf.size(),0);
}

/**
 * @brief send 404 not found page to client
 * @param client client 's socket fd
 * @param path path of requested file
*/
void Server::not_found(int client,string path){
    string buf;
    send_before_entity(client,404);
    buf="<html><head><title>404 not found</title></head>\
<body>The requested URL "+path+" was not found is this server.</body>\
</html";
    send(client,buf.c_str(),buf.size(),0);
}

/**
 * @brief execute cgi program
 * TODO: implement it
*/
void Server::execute_cgi(int client,string path,string method,string query_str){

}

/**
 * @brief send file to client by socket
 * @param client client socket fd
*/
void Server::send_response(int client,string path){
    string buf;
    fstream resource(path,ios::in);
    //read and discard request head
    do{
        buf=get_line(client);
    }while(buf.size()>0 && buf!="\n");
    
    if (!resource.is_open())
        not_found(client,path);
    else{
        send_before_entity(client,200);
        for(;getline(resource,buf);){
            buf+="\n";
            send(client,buf.c_str(),buf.size(),0);
        }
    }
    resource.close();
}

/**
 * @brief deal with request
 * @param client the socket connected to the client
*/
void Server::accept_request(int client){
    string buf;
    string method;
    string url;
    string path;
    string query="";

    struct stat st;
    bool cgi=false; //if request carries with data, becomes true, .e.g http://xxx.com/xxx?xxx or post request.

    buf=Server::get_line(client);

    int i=0,j=0;

    //read request method
    while(!isspace(buf[j]) && j<MaxCharNum-1){
        j++;
    }
    method=buf.substr(i,j-i);
    buf[j++]='\0';
    i=j;

    //check request method
    if (strcasecmp(method.c_str(),"GET") && strcasecmp(method.c_str(),"POST"))
        unimplemented(client);

    if (!strcasecmp(method.c_str(),"POST"))
        cgi=true;   //if method is post
    
    //ignore whitespace
    while(isspace(buf[j]) && j<MaxCharNum-1){
        i++;j++;
    }
    
    //read request path
    while(!isspace(buf[j]) && i<MaxCharNum-1)
        j++;
    url=buf.substr(i,j-i);
    buf[j++]='\0';
    i=j;
    
    //extract query string from url
    if (!strcasecmp(method.c_str(),"GET")){
        int pos=url.find('?');
        if (string::npos != pos){
            cgi=true;
            query=url.substr(pos+1,url.size()-pos-1);
            url=url.substr(0,pos);  //truncate url
            path=url;   //and reassign path
        }
    }

    path=ROOTPATH+url;
    if ('/' == path.back()) //request default index.html 
        path+="index.html";

    if (-1 == stat(path.c_str(),&st)){  //requested file not found.
        //read and discard request head
        do{
            buf=get_line(client);
        }while(buf.size()>0 && buf!="\n");
        not_found(client,path);
    }else{
        if (S_ISDIR(st.st_mode))    //it's directory
            path+="/index.html";
        if (S_ISXUSR(st.st_mode) ||
            S_ISXGRP(st.st_mode) ||
            S_ISXOTH(st.st_mode)
        )
            cgi=true;
        if (cgi)
            execute_cgi(client,path,method,query);
        else
            send_response(client,path);
    }

    close(client);
}

int main(){
    Server httpd;
    struct sockaddr_in client;
    socklen_t client_len=sizeof(client);
    pthread_t newthread;
    //start the server
    httpd.startup(10086);
    cout<<"the httpd server is starting up, listening on port:"<<httpd.getPort()<<endl;


    //...
    while(1){
        int client_sock=accept(httpd.getSock(),(struct sockaddr*)&client,&client_len);
        if (-1 == client_sock)
            error_die("accept failed.");
        //create a new thread to deal with request
        if (pthread_create(&newthread,0,(void*(*)(void*))Server::accept_request,(void*)(long)client_sock) < 0)
            cerr<<"create thread failed."<<endl;
    }

    close(httpd.getSock());
    
    return 0;
}