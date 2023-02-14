/*
    接收带外数据
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#define BUF_SIZE 1024

int main(int argc,char* argv[]){
    if(argc<=2){
        printf("usage :%s ip_address port_number\n",basename(argv[0]));
        return 1;
    }
    const char* ip=argv[1];
    int port=atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);

    int sock=socket(PF_INET,SOCK_STREAM,0);
    assert(sock>=0);

    int ret=bind(sock,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret=listen(sock,5);
    assert(ret!=-1);

    struct sockaddr_in client;
    socklen_t client_addrlength=sizeof(client);
    int connfd=accept(sock,(struct sockaddr*)&client,&client_addrlength);
    if(connfd<0)
        printf("error is: %d\n",errno);
    else{
        close(STDOUT_FILENO);//关闭标准输出文件描述符，STDOUT_FILENO，其值是一
        dup(connfd);//dup返回系统中最小的可用文件描述符，所以其返回值实际上是一，即标准输出文件描述符的值
        printf("abcd\n");//因此，服务器标准输出的内容即printf调用的输出将直接发送到与客户端连接的socket上，这就是CGI服务器工作原理

        close(connfd);

    }
    close(sock);

    return 0;
}