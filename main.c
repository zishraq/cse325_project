#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define BUFFER_LIMIT 10000

sem_t semEmpty;
sem_t semFull;

pthread_mutex_t mutexBuffer;

int buffer[BUFFER_LIMIT];
int *assorted_box;
int assorted_box_store[BUFFER_LIMIT][BUFFER_LIMIT];
int count = 0;
int candy_types_so_far = 0;
int assorted_boxes_count = 0;
int assorted_boxes_store_count = 0;

void printAndStoreAssortedBox(int *array, int size) {
    printf("Candies are no. ");
    for (int i = 0; i < size; i++) {
        assorted_box_store[assorted_boxes_store_count][i] = *(array + i);

        if (i == size - 1) {
            printf("and %d.", *(array + i));
        } else {
            printf("%d, ", *(array + i));
        }
    }

    assorted_boxes_store_count++;
    printf("\n");
}

void *producer(void *args) {
    // produce
    int x = *(int *) args + 1;

    // add to the buffer
    sem_wait(&semEmpty);
    pthread_mutex_lock(&mutexBuffer);

    buffer[count] = x;
    count++;

    printf("Produced -> candy no. %d\n", x);
    printf("\n");

    pthread_mutex_unlock(&mutexBuffer);
    sem_post(&semFull);
}

void *consumer(void *args) {
    int candy;
    int candy_types = *(int *) args;

    // remove from the buffer
    sem_wait(&semFull);
    pthread_mutex_lock(&mutexBuffer);

    candy = buffer[count - 1];
    count--;
    assorted_box[candy_types_so_far] = candy;
    candy_types_so_far++;

    if (candy_types_so_far == candy_types) {
        assorted_boxes_count += 1;
        printf("**** An Assorted box created ****\n");

        printAndStoreAssortedBox(assorted_box, candy_types);
        printf("\n");

        candy_types_so_far = 0;
    }

    // consume
    printf("Consumed <- candy no. %d\n", candy);
    printf("\n");

    pthread_mutex_unlock(&mutexBuffer);
    sem_post(&semEmpty);
}

int main(int argc, char *argv[]) {
    printf("*********************************************\n");
    printf("*                                           *\n");
    printf("*            AMHERST CANDY FACTORY          *\n");
    printf("*                                           *\n");
    printf("*********************************************\n");
    printf("\n");

    int producers, consumers, candy_types, difference, is_consumers_more = 0;

    producers = atoi(argv[1]);
    consumers = atoi(argv[2]);
    candy_types = atoi(argv[3]);
    difference = abs(producers - consumers);

    assorted_box = (int *) malloc(candy_types * sizeof(int));

    if (consumers > producers) {
        is_consumers_more = 1;
        consumers = producers;
    }

    pthread_t *producer_threads = (pthread_t *) malloc(producers * sizeof(pthread_t));
    pthread_t *consumer_threads = (pthread_t *) malloc(consumers * sizeof(pthread_t));

    pthread_mutex_init(&mutexBuffer, NULL);

    sem_init(&semEmpty, 0, producers);
    sem_init(&semFull, 0, 0);

    int i = 0, j = 0;
    while (1) {
        if (i == producers && j == consumers) {
            break;
        }

        if (i < producers) {
            int *value_to_pass = malloc(sizeof(int));
            *value_to_pass = i;

            if (pthread_create(&producer_threads[i], NULL, &producer, value_to_pass) != 0) {
                perror("Failed to create thread");
            }
            i++;
        }
        if (j < consumers) {
            int *value_to_pass = malloc(sizeof(int));
            *value_to_pass = candy_types;

            if (pthread_create(&consumer_threads[j], NULL, &consumer, value_to_pass) != 0) {
                perror("Failed to create thread");
            }
            j++;
        }
    }

    i = 0, j = 0;
    while (1) {
        if (i == producers && j == consumers) {
            break;
        }

        if (i < producers) {
            if (pthread_join(producer_threads[i], NULL) != 0) {
                perror("Failed to join thread");
            }
            i++;
        }
        if (j < consumers) {
            if (pthread_join(consumer_threads[j], NULL) != 0) {
                perror("Failed to join thread");
            }
            j++;
        }
    }

    printf("Post Production and Consumption: \n");
    printf("---------------------------------\n");
    printf("\n");

    if (is_consumers_more) {
        printf("%d candies short in production\n", difference);
    }
    else if (!is_consumers_more && difference > 0) {
        printf("%d more candies remain in the factory\n", difference);
    }
    else {
        printf("No more candies left\n");
    }

    printf("%d assorted boxes served\n", assorted_boxes_count);

    sem_destroy(&semEmpty);
    sem_destroy(&semFull);
    pthread_mutex_destroy(&mutexBuffer);

    return 0;
}
