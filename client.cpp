#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

void print_response(int sock){
    char ch=0;
    int i=0;
    int n;
    char buf[1024];
    
    do{
        n=recv(sock,&ch,1,0);
        buf[i++]=ch;
    }while(n>0);
    buf[i]=0;
    cout<<buf;
}

int main(){
    int sock;
    char c;
    struct sockaddr_in addr;
    sock=socket(AF_INET,SOCK_STREAM,0);
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    addr.sin_port=htons(10086);
    if (connect(sock,(struct sockaddr*)&addr,sizeof(addr))){
        cerr<<"open connection failed."<<endl;
        exit(1);
    }
    string req="GET /?name=dick HTTP/1.0\r\nfuck\r\n\r\n";
    send(sock,req.c_str(),req.size(),0);
    print_response(sock);
    close(sock);
    return 0;
}