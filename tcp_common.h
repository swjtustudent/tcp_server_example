#ifndef TCP_COMMON
#define TCP_COMMON
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>

void error_exit(char *error);
int tcp_server_create(int port, int backlog);
void add_signal( int sig_no, void(*pfsignal_handle)(int));
#endif
