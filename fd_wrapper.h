#ifndef FDWRAPPER_H
#define FDWRAPPER_H

#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

int setnonblocking( int fd );
void add_read_fd( int epollfd, int fd );
void add_write_fd( int epollfd, int fd );
void closefd( int epollfd, int fd );
void removefd( int epollfd, int fd );
void modfd( int epollfd, int fd, int ev );

#endif
