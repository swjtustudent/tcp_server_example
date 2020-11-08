#ifndef THREAD_POOL
#define THREAD_POOL
#include <pthread.h>

struct thread_pool_t;
typedef struct workers_t {
    pthread_t thread;
    int terminate;
    struct thread_pool_t *pool;
    struct workers_t *prev;
    struct workers_t *next;
} workers_t;

typedef struct tasks_t {
    void (*func)(struct tasks_t *job);  //user callback
    void *user_data;                    //user data
    struct tasks_t *prev;
    struct tasks_t *next;
} tasks_t;

typedef struct thread_pool_t {
    struct workers_t *workers;           //workers
    struct tasks_t *waiting_jobs;        //task queue
    int total_jobs;                      //total tasks
    int free_jobs;                       //free tasks
    pthread_mutex_t jobs_mtx;
    pthread_cond_t jobs_cond;
} thread_pool_t;

int thread_pool_init(thread_pool_t *pool, int pool_size);
void thread_pool_add_task(thread_pool_t *pool, tasks_t *job);
void thread_pool_destroy(thread_pool_t *pool);

#endif
