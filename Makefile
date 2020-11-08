TARGETS=tcp_server \
	tcp_client \
	tcp_client_multithread \
	tcp_server_multiprocess \
	tcp_server_epoll \
	tcp_reactor \
	tcp_reactor_threadpool \
	tcp_reactor_libevent \
	tcp_reactor_libevent_threadpool \
	tcp_simreactor_libevent_threadpool

CC=gcc
COMMON_DEP=tcp_common.c fd_wrapper.c

all:${TARGETS}

CFLAGS=-Wall -g

tcp_server:tcp_server.c ${COMMON_DEP}
	${CC} -o $@ $^ ${CFLAGS}

tcp_server_multiprocess:tcp_server_multiprocess.c ${COMMON_DEP}
	${CC} -o $@ $^ ${CFLAGS}

tcp_server_epoll:tcp_server_epoll.c ${COMMON_DEP}
	${CC} -o $@ $^ ${CFLAGS}

tcp_server_epoll_threadpool:tcp_server_epoll_threadpool.c ${COMMON_DEP}
	${CC} -o $@ $^ ${CFLAGS}

tcp_client:tcp_client.c ${COMMON_DEP}
	${CC} -o $@ $^ ${CFLAGS}

tcp_client_multithread:tcp_client_multithread.c ${COMMON_DEP}
	${CC} -o $@ $^ -lpthread ${CFLAGS}

tcp_reactor:tcp_reactor.c
	${CC} -o $@ $^ ${CFLAGS}

tcp_reactor_threadpool:tcp_reactor_threadpool.c
	${CC} -o $@ $^ -lpthread ${CFLAGS}

tcp_simreactor_libevent_threadpool:tcp_simreactor_libevent_threadpool.c thread_pool.c
	${CC} -o $@ $^ `pkg-config --libs libevent` -levent_pthreads -lpthread ${CFLAGS}

tcp_reactor_libevent_threadpool:tcp_reactor_libevent_threadpool.c thread_pool.c
	${CC} -o $@ $^ `pkg-config --libs libevent` -levent_pthreads -lpthread ${CFLAGS}

tcp_reactor_libevent:tcp_reactor_libevent.c
	${CC} -o $@ $^ `pkg-config --libs libevent` ${CFLAGS}

.PHONY:
clean:
	rm -rf ${TARGETS}
