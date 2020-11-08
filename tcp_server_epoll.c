#include <sys/epoll.h>
#include <time.h>
#include <errno.h>
#include "tcp_common.h"
#include "fd_wrapper.h"

#define MAX_BUF_LEN 1024

static void handler_child(int nSig)
{
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("wait %d success!\n", pid);
    }
}

int main(int argc, char *argv[])
{
    char buf[MAX_BUF_LEN] = "";
    char replybuf[MAX_BUF_LEN] = "";
    char *peer_addr = NULL;
    struct sockaddr_in client_addr;
    int listen_fd = -1, client_fd = -1, epfd = -1, nfds, i, pos=0,nbytes=0;
    struct epoll_event ev, events[20];
    unsigned int client_addr_len = 0;

    listen_fd = tcp_server_create(8081, 1024);
    add_signal(SIGCHLD, handler_child);

    /* create epoll fd */
    epfd = epoll_create(10);

    /* 为listen_fd注册epoll事件，ET模式 */
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);
    setnonblocking(listen_fd);

    while (1)
    {
        /* 等待epfd上的事件 */
        nfds = epoll_wait(epfd, events, 20, 500);
        /* 遍历描述符 */
        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == listen_fd)
            {
                /* 检测到连接事件， accept */
                /* 如果是ET模式，这里accept需要加循环，否则可能会丢失，因为多个连接请求同时过来的时候，可能会只触发一次 */
                client_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
		if (client_fd < 0)
                {
                    if (errno != EAGAIN && errno != ECONNABORTED   
                            && errno != EPROTO && errno != EINTR)
                    {
                        perror("Accept");
                    }

                }

		peer_addr = inet_ntoa(client_addr.sin_addr);
                printf("accept a connection from %s\n", peer_addr);
                add_read_fd(epfd, client_fd);
            }
            else if (events[i].events & EPOLLIN )
            {
                pos = 0;  
                while ((nbytes = read(client_fd, buf + pos, BUFSIZ-1)) > 0)
                {  
                    pos += nbytes;  
                }  
                if (nbytes == -1 && errno != EAGAIN) {  
                    perror("read error");  
                }
                printf("receive message %s\n", buf);
                snprintf(replybuf, pos + 100, "Receive your message [%s]", buf);
                write(client_fd, replybuf, strlen(replybuf));
		closefd(epfd, client_fd);
            }
            else if (events[i].events & EPOLLOUT )
            {

            }

        }
    }

    close(listen_fd);

    return 0;
}
