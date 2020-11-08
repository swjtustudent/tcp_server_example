#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "thread_pool.h"

#define LL_ADD(item, list) do {\
    item->prev = NULL;         \
    item->next = list;         \
    list = item;               \
} while(0)

#define LL_REMOVE(item, list) do {                            \
    if (item->prev != NULL) item->prev->next = item->next;    \
    if (item->next != NULL) item->next->prev = item->prev;    \
    if (list == item) list = item->next;                      \
    item->prev = item->next = NULL;                           \
} while(0)

static void *worker_cb(void *arg) {
    workers_t *worker = (workers_t*)arg;

    while (1) {
        pthread_mutex_lock(&worker->pool->jobs_mtx);

        while (worker->pool->waiting_jobs == NULL) {
            if (worker->terminate) break;
            pthread_cond_wait(&worker->pool->jobs_cond, &worker->pool->jobs_mtx);
        }

        if (worker->terminate) {
            pthread_mutex_unlock(&worker->pool->jobs_mtx);
            break;
        }

        tasks_t *job = worker->pool->waiting_jobs;
        if (job != NULL) {
            LL_REMOVE(job, worker->pool->waiting_jobs);
        }

        pthread_mutex_unlock(&worker->pool->jobs_mtx);

        if (job == NULL) continue;
        job->func(job);
    }

    free(worker);
    pthread_exit(NULL);
}

int thread_pool_init(thread_pool_t *pool, int numWorkers) {

    if (numWorkers < 1) numWorkers = 1;
    memset(pool, 0, sizeof(thread_pool_t));

    pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
    memcpy(&pool->jobs_cond, &blank_cond, sizeof(pool->jobs_cond));

    pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
    memcpy(&pool->jobs_mtx, &blank_mutex, sizeof(pool->jobs_mtx));

    int i = 0;
    for (i = 0;i < numWorkers;i ++) {
        workers_t *worker = (workers_t*)malloc(sizeof(workers_t));
        if (worker == NULL) {
            perror("malloc");
            return 1;
        }

        memset(worker, 0, sizeof(workers_t));
        worker->pool = pool;

        //printf("pthread_create --> %d\n", i);
        int ret = pthread_create(&worker->thread, NULL, worker_cb, (void *)worker);
        if (ret) {

            perror("pthread_create");
            free(worker);

            return 1;
        }

        LL_ADD(worker, worker->pool->workers);
    }

    return 0;
}


void thread_pool_destroy(thread_pool_t *pool) {
    workers_t *worker = NULL;

    for (worker = pool->workers;worker != NULL;worker = worker->next) {
        worker->terminate = 1;
    }

    pthread_mutex_lock(&pool->jobs_mtx);

    pool->workers = NULL;
    pool->waiting_jobs = NULL;

    pthread_cond_broadcast(&pool->jobs_cond);
    pthread_mutex_unlock(&pool->jobs_mtx);
}

void thread_pool_add_task(thread_pool_t *pool, tasks_t *job) {

    pthread_mutex_lock(&pool->jobs_mtx);

    LL_ADD(job, pool->waiting_jobs);

    pthread_cond_signal(&pool->jobs_cond);
    pthread_mutex_unlock(&pool->jobs_mtx);

}

#ifdef THREAD_POOL_MAIN
#define MAX_THREAD        80
#define COUNTER_SIZE        1000

void my_counter(tasks_t *job) {

    int index = *(int*)job->user_data;

    printf("index : %d, selfid : %lu\n", index, pthread_self());

    free(job->user_data);
    free(job);
}

int main(int argc, char *argv[]) {

    thread_pool_t pool;

    thread_pool_init(&pool, MAX_THREAD);

    int i = 0;
    for (i = 0;i < COUNTER_SIZE;i ++) {
        tasks_t *job = (tasks_t*)malloc(sizeof(tasks_t));
        if (job == NULL) {
            perror("malloc");
            exit(1);
        }

        job->func = my_counter;
        job->user_data = malloc(sizeof(int));
        *(int*)job->user_data = i;

        thread_pool_add_task(&pool, job);
    }

    getchar();
    printf("\n");
    return 0;
}
#endif
