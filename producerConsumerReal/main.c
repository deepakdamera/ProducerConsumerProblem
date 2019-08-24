#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>

// Semaphore declarations
sem_t mutex;
sem_t empty;
sem_t full;

// Flag definition for exiting the pthreads when upper_limit reached
int limit_flag = 0;

// Shared value definition between producers
int p_shared_value = 0;


int *buffer;
int in = 0; // to be used while inserting into buffer
int out = 0; // to be used while reading from buffer

typedef struct _data_t {

    int tid;
    int buffer_size;
    int upper_limit;

} data_t;


/* Function for producer threads */
void *produce(void *arg) {

    data_t *passedData = (data_t *) arg;

    while (1) {


        sem_wait(&empty);
        sem_wait(&mutex);

        // Critical section starts
        if (limit_flag) {
            sem_post(&mutex); // Increment mutex since we are done.
            sem_post(&empty); // Increment empty since we didn't use it.
            pthread_exit(NULL);
        } else {
            buffer[in] = p_shared_value; // Insert shared value in the buffer.
            p_shared_value++; // Increment shared value by one.
            in = (in + 1) % passedData->buffer_size;
            // Critical section ends here
            sem_post(&mutex);
            sem_post(&full);
        }
    }
}

/* Function for consumer threads */
void *consume(void *arg) {

    data_t *passedData = (data_t *) arg;

    while (1) {

        sem_wait(&full);
        sem_wait(&mutex);
        // Critical section ...

        if (passedData->upper_limit == buffer[out]) {
            if (!limit_flag) {
                limit_flag = 1;
                printf("Integer value : %d - TID consumer : %d\n", buffer[out], passedData->tid);
            }
            sem_post(&mutex);   // Increment mutex
            sem_post(&full);    // Increment full since we didn't consume anything
            sem_post(&empty);   // This is to prevent deadlocks
                                // when the exit process of all pthreads is initiated.
            pthread_exit(NULL);
        } else {
            printf("Integer value : %d - TID consumer : %d\n", buffer[out], passedData->tid);
            out = (out + 1) % passedData->buffer_size;
            // Critical section ends here
            sem_post(&mutex); // Increment mutex so others can use
            sem_post(&empty); // Increment empty since we consumed an element
        }
    }
}


int main(int argc, char *argv[]) {

    // ./a.out <buffer_size> <num_producers> <num_consumers> <upper_limit>
    unsigned int buffer_size = (unsigned int) atoi(argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);
    int upper_limit = atoi(argv[4]);

    buffer = malloc(buffer_size * sizeof(int)); // Malloc for buffer

    // Init semaphores
    sem_init(&empty, 0, buffer_size);
    sem_init(&full, 0, 0);
    sem_init(&mutex, 0, 1);

    pthread_t c_threads[num_consumers];
    pthread_t p_threads[num_producers];

    data_t p_structs[num_producers]; // struct array producers
    data_t c_structs[num_consumers]; // struct array consumers


    for (int i = 0; i < num_producers; i++) {
        p_structs[i].buffer_size = buffer_size;
        p_structs[i].tid = i;
        p_structs[i].upper_limit = upper_limit;

        pthread_create(&(p_threads[i]), NULL, &produce, ((void *) (&p_structs[i])));
    }

    for (int i = 0; i < num_consumers; i++) {
        c_structs[i].buffer_size = buffer_size;
        c_structs[i].tid = i;
        c_structs[i].upper_limit = upper_limit;

        pthread_create(&c_threads[i], NULL, &consume, ((void *) (&c_structs[i])));
    }

    for (int i = 0; i < num_producers; i++) {
        pthread_join(p_threads[i], NULL); // Join all the producer threads.
    }

    for (int i = 0; i < num_consumers; i++) {
        pthread_join(c_threads[i], NULL); // Join all the consumer threads.
    }
    return 0;
}