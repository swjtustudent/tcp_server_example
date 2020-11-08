#include "tcp_common.h"

void add_signal( int sig_no, void(*pfsignal_handle)(int))
{
    struct sigaction signal_action;

    memset(&signal_action, '\0', sizeof(signal_action));
    signal_action.sa_handler = pfsignal_handle;
    signal_action.sa_flags |= SA_RESTART;
    sigfillset(&signal_action.sa_mask);
    if (sigaction(sig_no, &signal_action, NULL) == -1)
    {
        fprintf(stderr, "add signal %d failed\n", sig_no);
        exit(EXIT_FAILURE);
    }
}

void error_exit(char *error)
{
    fprintf(stderr, "%s, %d, %s\n", __FILE__, __LINE__, error);
    exit(1);
}

int tcp_server_create(int port, int backlog)
{
    int ret = 0;
    /* arg1: domain, 可指定为IPV4：AF_INET, IPV6: AF_INET6, UNIX: PF_UINIX */
    /* arg2: socket类型，TCP: SOCK_STREAM, UDP:SOCK_DGRAM */
    /* arg3: 协议版本，0代表默认使用默认协议（这里是IPV4）*/
    int connection_fd = socket(AF_INET, SOCK_STREAM, 0);
    if( connection_fd < 0)
    {
        error_exit("socket!");
    }

    /* 声明和填充socket地址结构 */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    /* 设置地址重用 */
    int option = 1;
    ret = setsockopt(connection_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(int));
    if (ret)
        error_exit("set socket option reuse addr");

    /* 绑定端口 */
    /* arg1：连接套接字 */
    /* arg2：sockaddr地址结构 */
    /* arg3: sockaddr长度 */
    ret = bind(connection_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) 
        error_exit("bind");

    /* arg1：连接套接字 */
    /* arg2：Linux 2.2以后，表示处于established状态的socket的上限 */
    ret = listen(connection_fd, backlog);
    if (ret < 0)
        error_exit("listen");

    return connection_fd;
}
