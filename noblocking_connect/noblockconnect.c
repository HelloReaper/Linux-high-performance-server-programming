/* 非阻塞connect方式，使得用户能够同时发起多个连接并一起等待 */
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define BUFFER_SIZE 1023

int setnoblocking(int fd)
{
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

/* 超时连接函数，参数分别是服务器IP地址、端口号和超时时间（毫秒）。函数成功时返回已经处于连接状态的socket，失败时则返回-1 */
int unblock_connect(const char* ip,int port,int time)
{
    int ret=0;
    /* 创建一个socket */
    struct sockaddr_in address;
    bzero(&address,sizeof(address)); /* socket地址清零 */
    address.sin_family=AF_INET;/* IPV4地址簇 */
    inet_pton(AF_INET,ip,&address.sin_addr);/* 字符串的ip地址转换为整形的ip地址存储在address.sin_addr中 */
    address.sin_port=htons(port);/* 端口号本地字节序转换为网络字节序 */

    int sockfd=socket(PF_INET,SOCK_STREAM,0);/* IPV4协议簇，SOCK_STREAM字节流表示使用的是TCP协议 */
    int fdopt=setnoblocking(sockfd);/* 设置文件描述符为非阻塞 */
    ret=connect(sockfd,(struct sockaddf*)&address,sizeof(address));
    if(ret==0)
    {
        /* 如果连接成功，则恢复sockfd的属性，并立即返回之 */
        printf("connect with server immediately\n");
        fcntl(sockfd,F_SETFL,fdopt);
        return sockfd;
    }
    else if(errno!=EINPROGRESS)
    {
        /* 如果连接没有立即建立，那么只有当errno */
        printf("unblock connect not support\n");
        return -1;
    }

    fd_set readfds;
    fd_set write_fds;
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(sockfd,&write_fds);

    timeout.tv_sec=time;
    timeout.tv_usec=0;

    ret=select(sockfd+1,NULL,&write_fds,NULL,&timeout);
    if(ret<=0)
    {
        /* select 超时或者出错，立即返回 */
        printf("connect time out\n");
        close(sockfd);
        return -1;
    }

    if(!FD_ISSET(sockfd,&write_fds))
    {
        printf("no events on sockfd found\n");
        close(sockfd);
        return -1;
    }

    int error=0;
    socklen_t length=sizeof(error);
    /* 调用getsockopt来获取并清除sockfd上的错误 */
    if(getsockopt(sockfd,SOL_SOCKET,SO_ERROR,&error,&length)<0)
    {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }
    /* 错误号不为0表示连接出错 */
    if(error!=0)
    {
        printf("connection failed after select with the error : %d\n",error);
        close(sockfd);
        return -1;
    }
    /* 连接成功 */
    printf("connection ready after select with the socket: %d\n",sockfd);
    fcntl(sockfd,F_SETFL,fdopt);
    return sockfd;
}

int main(int argc,char* argv[]){
    if(argc<=2)
    {
        printf("usage: %s ip_address port_number\n",basename(argv[0]));
        return 1;
    }
    const char* ip=argv[1];
    int port=atoi(argv[2]);

    int sockfd=unblock_connect(ip,port,10);
    if(sockfd<0)
        return 1;
    close(socket);
    return 0;
}

/* 改方法存在基础移植性问题。首先，非阻塞的socket可能导致connect始终失败。其次select对处于EINPROGESS状态下的socket可能不起作用，
最后，对于出错的socket，getsockopt在有些系统（比如linux）上返回-1,而有些系统（UNIX）上返回0。这些问题没有统一的解决方法。 */