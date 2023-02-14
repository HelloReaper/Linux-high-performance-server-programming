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

int main(int argc , char *argv[]){
    if(argc<=2){
        printf("usage : %s ip_address port_number\n",basename(argv[0]));
        return 1;
    }
    const char* ip=argv[1];
    int port=argv[2];

    /*创建IPV4专用socket地址*/
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);

    /*创建socket*/
    int sock=socket(PF_INET,SOCK_STREAM,0);
    assert(sock>=0);

    /*绑定socket与socket地址*/
    int ret=bind(sock,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    /*监听socket,监听队列最大为5+1*/
    ret=listen(sock,5);
    assert(ret!=-1);

    /*暂停20秒以等待客户端连接和相关操作(掉线或退出)完成*/
    sleep(20);
    struct sockaddr_in client;
    socklen_t client_addrlength=sizeof(client);
    int connfd=accept(sock,(struct sockaddr*)&client,&client_addrlength);
    if(connfd<0)
        printf("errno is :%d \n",errno);
    else{
        /*接收连接成功则打印出客户端的IP地址和端口号*/
        char remote[INET_ADDRSTRLEN];
        printf("connected with ip: %s and port: %d \n",
            inet_ntop(AF_INET,&client.sin_addr,remote,INET_ADDRSTRLEN),ntohs(client.sin_port));
        close(connfd);
    }
    

    return 0;
}
