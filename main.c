#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#define BUFFER_LIMIT 1000000

sem_t sem_empty;
sem_t sem_full;
sem_t mutex;

int buffer[BUFFER_LIMIT];
int *assorted_box;
int **assorted_box_store;
int insert_index = 0;
int extract_index = 0;
int assorted_boxes_count = 0;
int assorted_boxes_store_count = 0;


void printAndStoreAssortedBox(int *array, int size) {
    printf("**** An Assorted box created ****\n");
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
    sem_wait(&sem_empty);
    sem_wait(&mutex);

    int candy = insert_index + 1;
    buffer[insert_index] = candy;

    printf("Produced -->> candy no. %d\n", candy);
    printf("\n");

    insert_index++;

    sem_post(&mutex);
    sem_post(&sem_full);
    free(args);
}

void *consumer(void *args) {
    int candy;
    int candy_types = *(int *) args;

    sem_wait(&sem_full);
    sem_wait(&mutex);

    candy = buffer[extract_index];

    assorted_box[extract_index % candy_types] = candy;

    printf("Consumed <<-- candy no. %d\n", candy);
    printf("\n");

    if (extract_index % candy_types == candy_types - 1) {
        assorted_boxes_count += 1;

        printAndStoreAssortedBox(assorted_box, candy_types);
        printf("\n");
    }

    extract_index++;

    sem_post(&mutex);
    sem_post(&sem_empty);
    free(args);
}

int main(int argc, char *argv[]) {
    printf("*********************************************\n");
    printf("*                                           *\n");
    printf("*           AMHERST CANDY FACTORY           *\n");
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

    int expected_assorted_boxes = consumers / candy_types;

    assorted_box_store = (int **) malloc(expected_assorted_boxes * sizeof(int *));

    for (int i = 0; i < expected_assorted_boxes; i++) {
        assorted_box_store[i] = (int *) malloc(candy_types * sizeof(int));
    }

    pthread_t *producer_threads = (pthread_t *) malloc(producers * sizeof(pthread_t));
    pthread_t *consumer_threads = (pthread_t *) malloc(consumers * sizeof(pthread_t));

    sem_init(&sem_empty, 0, producers);
    sem_init(&sem_full, 0, 0);
    sem_init(&mutex, 0, 1);

    int i = 0, j = 0;

    while (1) {
        if (i == producers && j == consumers) {
            break;
        }

        if (i < producers) {
            if (pthread_create(&producer_threads[i], NULL, &producer, NULL) != 0) {
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
    else if (difference > 0) {
        printf("%d candies remain in the factory\n", difference);
    }
    else {
        printf("No more candies left in the factory\n");
    }

    printf("%d assorted boxes served with %d candies\n", assorted_boxes_count, assorted_boxes_count * candy_types);
    printf("%d candies consumed but not boxed\n", consumers - assorted_boxes_count * candy_types);

    sem_destroy(&sem_empty);
    sem_destroy(&sem_full);
    sem_destroy(&mutex);
    free(assorted_box);
    free(assorted_box_store);
    free(producer_threads);
    free(consumer_threads);

    return 0;
}
