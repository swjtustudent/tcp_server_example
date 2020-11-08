/* A basic single process/thread tcp server */
#include "tcp_common.h"
#include <time.h>

int main(int argc, char *argv[])
{
    char buf[1024]="";
    char replybuf[2048]="";
    char *peer_addr = NULL;
    struct sockaddr_in client_addr;
    int listen_fd = -1,client_fd = -1;
    unsigned int client_addr_len = 0;
    listen_fd = tcp_server_create(8081, 1024);

    while(1)
    {
	/* arg1: 监听套接字
	 * arg2: 被接受连接的远端地址结构
	 * arg3: 远端地址的长度       */
        client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if(client_fd < 0)
        {
            error_exit("Accept");
        }

        peer_addr = inet_ntoa(client_addr.sin_addr);
        printf("accept a connection from %s\n", peer_addr);
        /* arg1: 连接套接字
	 * arg2: 读缓冲区地址
	 * arg3: 读缓冲区大小
	 * arg4: 标志位 */
	recv(client_fd, buf, 1024,0);
        printf("receive message %s\n",buf);
        sprintf(replybuf, "Receive your message [%s]", buf);;
        /* arg1: 连接套接字
	 * arg2: 写缓冲区地址
	 * arg3: 写缓冲区大小
	 * arg4: 标志位 */
        send(client_fd, replybuf, strlen(replybuf), 0);
        close(client_fd);
        sleep(1);
    }

    close(listen_fd);
    
    return 0;
}
