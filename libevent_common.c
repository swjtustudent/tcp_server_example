#include "libevent_common.h"
int tcp_server_init(int port, int listen_num)
{
    int errno_save;
    evutil_socket_t listener;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1)
        return -1;

    //允许多次绑定同一个地址。要用在socket和bind之间
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

int unix_server_init(char *path, int listen_num)
{
    int errno_save;
    evutil_socket_t listener;

    listener = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener == -1)
        return -1;
    evutil_make_listen_socket_reuseable(listener);
    struct sockaddr_un sin;
    memset(&sin, 0, sizeof(sin));
    sin.sun_family = AF_UNIX;
    strcpy(sin.sun_path, path);

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

int unix_server_bind(char *path)
{
    int errno_save;
    evutil_socket_t listener;

    if (!access(path, F_OK))
    {
        printf("unlink %s\n",path);
        unlink(path);
    }

    listener = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener == -1)
        return -1;
    evutil_make_listen_socket_reuseable(listener);
    struct sockaddr_un sin;
    memset(&sin, 0, sizeof(sin));
    sin.sun_family = AF_UNIX;
    strcpy(sin.sun_path, path);

    do
    {
        if (bind(listener, (struct sockaddr *)&sin, sizeof(sin)) < 0)
            break;

        return listener;
    } while (0);

    errno_save = errno;
    evutil_closesocket(listener);
    errno = errno_save;
    return -1;
}

int tcp_connect_server(const char *server_ip, int port)
{
    int sockfd, status, save_errno;
    struct sockaddr_in server_addr;

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    status = inet_aton(server_ip, &server_addr.sin_addr);

    if (status == 0) //the server_ip is not valid value
    {
        errno = EINVAL;
        return -1;
    }

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
        return sockfd;

    status = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (status == -1)
    {
        save_errno = errno;
        close(sockfd);
        errno = save_errno; //the close may be error
        return -1;
    }

    evutil_make_socket_nonblocking(sockfd);

    return sockfd;
}

int unix_connect_server(const char *path)
{
    int sockfd, status, save_errno;
    struct sockaddr_un server_addr;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, path);

    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd == -1)
        return sockfd;

    status = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    if (status == -1)
    {
        save_errno = errno;
        close(sockfd);
        errno = save_errno; //the close may be error
        return -1;
    }

    evutil_make_socket_nonblocking(sockfd);

    return sockfd;
}