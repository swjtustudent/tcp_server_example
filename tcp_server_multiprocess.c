/* A basic multiprocess tcp server */
#include "tcp_common.h"
#include <time.h>

static void handler_child(int nSig)
{
    pid_t pid;

    while(( pid = waitpid( -1, NULL, WNOHANG)) > 0)
    {
        printf("wait %d success!\n", pid );
    }
}

int main(int argc, char *argv[])
{
    char buf[1024]="";
    char replybuf[2048]="";
    char *peer_addr = NULL;
    struct sockaddr_in client_addr;
    int connection_fd = -1,client_fd = -1;
    unsigned int client_addr_len = 0;
    pid_t pid = 0;
    connection_fd = tcp_server_create(8081, 1024);
    add_signal(SIGCHLD, handler_child);

    while(1)
    {
        client_fd = accept(connection_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if(client_fd < 0)
        {
            printf("client fd = %d\n", client_fd);
            error_exit("Accept");
        }
        peer_addr = inet_ntoa(client_addr.sin_addr);
        printf("accept a connection from %s\n", peer_addr);
        pid = fork();
        if (pid == 0)
        {
            /* 子进程，处理请求 */
            close(connection_fd);
            recv(client_fd, buf, 1024,0);
            printf("receive message %s\n",buf);
            sprintf(replybuf, "Receive your message [%s]", buf);;
            send(client_fd, replybuf, strlen(replybuf), 0);
            close(client_fd);
            sleep(1);
            exit(0);
        }
        else if (pid > 0)
        {
            /* 父进程 */
            close(client_fd);
            while(( pid = waitpid( -1, NULL, WNOHANG)) > 0);
        }
        else
        {
            error_exit("fork");
        }
        
    }

    close(connection_fd);
    
    return 0;
}
