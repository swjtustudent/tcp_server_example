#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/util.h>
#include <event2/thread.h>
#include "thread_pool.h"

typedef struct _userdata
{
   struct event_base *base;
   struct event *ev;
   int sockfd;
   char buf[4096];
} user_data_t;

#define THREAD_COUNT 8
static thread_pool_t g_pool;
int tcp_server_init(int port, int listen_num);
void accept_cb(int fd, short events, void *arg);
void socket_read_cb(int fd, short events, void *arg);
void socket_write_cb(int fd, short events, void *arg);
void worker_cb(tasks_t *job);

int main(int argc, char **argv)
{   
    /* 初始化线程池 */
    evthread_use_pthreads();
    thread_pool_init(&g_pool, THREAD_COUNT);

    int listener = tcp_server_init(8081, 10);
    if (listener == -1)
    {
        perror(" tcp_server_init error ");
        return -1;
    }
    struct event_base *base = event_base_new();

    //添加监听客户端请求连接事件
    struct event *ev_listen = event_new(base, listener, EV_READ | EV_PERSIST,
                                        accept_cb, base);
    event_add(ev_listen, NULL);
    event_base_dispatch(base);
    /* 销毁线程池 */
    thread_pool_destroy(&g_pool);
    event_free(ev_listen);
    event_base_free(base);

    return 0;
}

void accept_cb(int fd, short events, void *arg)
{
    evutil_socket_t sockfd;

    struct sockaddr_in client;
    socklen_t len = sizeof(client);

    sockfd = accept(fd, (struct sockaddr *)&client, &len);
    //evutil_make_socket_nonblocking(sockfd);
    struct event_base *base = (struct event_base *)arg;

    //仅仅是为了动态创建一个event结构体
    user_data_t *puser_data = (user_data_t *)malloc(sizeof(user_data_t));
    if (!puser_data)
    {
      perror("malloc");
      exit(1);
    }
  
    puser_data->base = base;
    puser_data->sockfd = sockfd;
    struct event *ev = event_new(NULL, -1, 0, NULL, NULL);
    //将动态创建的结构体作为event的回调参数
    puser_data->ev = ev;
    event_assign(ev, base, sockfd, EV_READ | EV_PERSIST,
                 socket_read_cb, (void *)puser_data);

    event_add(ev, NULL);
}

void socket_write_cb(int fd, short events, void *arg)
{
    user_data_t *puser_data = (user_data_t *)arg;
    struct event *ev = puser_data->ev;
  
    write(fd, puser_data->buf, strlen(puser_data->buf));
    event_free(ev);
    close(fd);
    free(puser_data);
}

void socket_read_cb(int fd, short events, void *arg)
{
    user_data_t *puser_data = (user_data_t *)arg;
    struct event *ev = puser_data->ev;

    int len = read(fd, puser_data->buf, sizeof(puser_data->buf) - 1);

    if (len <= 0)
    {
        printf("some error happen when read\n");
        event_free(ev);
        close(fd);
        return;
    }

    event_free(ev);
    puser_data->buf[len] = '\0';

    /* add to task queue */
    tasks_t *job = (tasks_t*)malloc(sizeof(tasks_t));
    if (job == NULL)
    {
        perror("malloc");
        exit(1);
    }
  
    job->func = worker_cb;
    job->user_data = (void *)puser_data;
    thread_pool_add_task(&g_pool, job);
}

int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    evutil_socket_t listener;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1)
        return -1;

    evutil_make_listen_socket_reuseable(listener);

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(port);

    do
    {
        if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            break;

        if (listen(listener, listen_num) < 0)
            break;

        //跨平台统一接口，将套接字设置为非阻塞状态
        evutil_make_socket_nonblocking(listener);

        return listener;
    } while (0);

    errno_save = errno;
    evutil_closesocket(listener);
    errno = errno_save;
    return -1;
}

void worker_cb(tasks_t *job)
{
    user_data_t *puser_data = (user_data_t *)job->user_data;
    
    printf("recv the client msg: %s\n", puser_data->buf);
    char reply_msg[4096] = "I have recvieced the msg: ";
    sprintf(reply_msg, "%ld have received the message: ", pthread_self());
    strcat(reply_msg, puser_data->buf);
    strcpy(puser_data->buf, reply_msg);

    puser_data->ev = event_new(NULL, -1, 0, NULL, NULL);
    event_assign(puser_data->ev, puser_data->base, puser_data->sockfd, EV_WRITE | EV_PERSIST,
                 socket_write_cb, (void *)puser_data);
    event_add(puser_data->ev, NULL);
}