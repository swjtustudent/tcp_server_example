#ifndef LIBEVENT_COMMON
#define LIBEVENT_COMMON
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <event.h>
#include <event2/util.h>
int tcp_server_init(int port, int listen_num);
int unix_server_init(char *path, int listen_num);
int tcp_connect_server(const char *server_ip, int port);
int unix_connect_server(const char *path);
int unix_server_bind(char *path);
#endif