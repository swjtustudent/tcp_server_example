#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

struct addr_info {
    char saddr[128];
    int port;
};

static void *thread_handle(void *arg)
{
    int sock;
    int len=0;
    struct sockaddr_in sin;
    char sendbuf[1024];
    char recvbuf[1024];
    pthread_t tid =pthread_self();

    struct addr_info *paddr_info = (struct addr_info *)arg;

    printf("in thread %lu\n", (unsigned long)tid);
    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        perror("socket\n");
        pthread_exit(NULL);
    }

    bzero(&sin, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(paddr_info->port);
    sin.sin_addr.s_addr = inet_addr(paddr_info->saddr);
    bzero(&(sin.sin_zero), 8);

    if (connect(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        printf("connect error\n");
        close(sock);
        pthread_exit(NULL);
    }

    sprintf(sendbuf, "How do you do, I'm %lu", (unsigned long)tid);
    
    len = write(sock, sendbuf, strlen(sendbuf));
    printf("%lu send out message>>>>>>>>>>>>>>>>>[%s], len=%d\n",(unsigned long)tid, sendbuf, len);
    read(sock, recvbuf, 1024);
    printf("%lu Recieved message =<<<<<<<<<<<<<<<<<<<[%s]\n", (unsigned long)tid, recvbuf);
    close(sock);
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc <= 3)
    {
        printf("usage: %s ip_address port_number thread_num\n", argv[0]);
        return 1;
    }
    const char *ip = argv[1];
    int port = atoi(argv[2]);
    int thread_num = atoi(argv[3]);
    pthread_t thread_ids[1024];
    struct addr_info addrinfo;
    int n=0;

    strcpy(addrinfo.saddr, ip);
    addrinfo.port=port;

    for (n = 0; n < thread_num; n++)
    {
        pthread_create(&thread_ids[n], NULL, thread_handle, (void *)&addrinfo);
        usleep(100);
    }

    for (n = 0; n < thread_num; n++)
    {
        pthread_join(thread_ids[n], NULL);
    }

    return 0;
}
