#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <threadpool.h>

#define THREAD_POOL_SIZE 8
#define QUEUE_SIZE 100

void *thread_function(void *arg){

    threadpool_t *pool = (threadpool_t*) arg;

    while(1){

        pthread_mutex_lock(&(pool->lock));

        while(pool->queued == 0 && !pool->stop){
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if(pool->stop){
            pthread_mutex_unlock(&(pool->lock));
            pthread_exit(NULL);
        }

        task_t task = pool->task_queue[pool->queue_front];
        pool->queue_front = (pool->queue_front + 1) % QUEUE_SIZE;
        pool->queued--;

        pthread_mutex_unlock(&(pool->lock));

        (*(task.fn))(task.arg);

    }

    return NULL;
}


void threadpool_init(threadpool_t *pool){
    pool->queued = 0;
    pool->queue_front = 0;
    pool->queue_back = 0;
    pool->stop = 0;

    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->notify), NULL);

    for(int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(&(pool->threads[i]), NULL, thread_function, pool);
    }
}


void threadpool_destroy(threadpool_t *pool){
    pthread_mutex_lock(&(pool->lock));
    pool->stop = 1;
    pthread_cond_broadcast(&(pool->notify));
    pthread_mutex_unlock(&(pool->lock));

    for(int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_join(pool->threads[i], NULL);
    }

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->notify));


}