#define _REENTRANT
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <dispatch/dispatch.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h> // for sleep

typedef char bufferItem;
#define BUFFER_SIZE 15

pthread_mutex_t mutex;
dispatch_semaphore_t full;  // Use dispatch_semaphore_t for full
dispatch_semaphore_t empty; // Use dispatch_semaphore_t for empty
int counter;
bufferItem buffer[BUFFER_SIZE];

//--------------------------------------------
// insert function used by producer thread
//--------------------------------------------
void insertItem(bufferItem item) {
    if (counter < BUFFER_SIZE) {
        buffer[counter - 1] = item;
        counter++;
        return;
    } else
        printf("Error inserting item\n");
}

//--------------------------------------------
// remove function used by consumer thread
//---------------------------------------------
bufferItem removeItem() {
    if (counter > 0) {
        bufferItem itemRem;
        itemRem = buffer[counter];
        counter--;
        return itemRem;
    } else
        return -1;
}

//---------------------------------------------
// producer prototype
//---------------------------------------------
void *thread1(void *arg) {
    char newChar;
    FILE *fp;
    fp = fopen("mytest.dat", "r");

    while (1 && fscanf(fp, "%c", &newChar) != EOF) {

        // produce an item
        bufferItem currentChar = newChar;

        dispatch_semaphore_wait(empty, DISPATCH_TIME_FOREVER);
        pthread_mutex_lock(&mutex);

        insertItem(currentChar);

        pthread_mutex_unlock(&mutex);
        dispatch_semaphore_signal(full);
    }
    fclose(fp);
    dispatch_semaphore_signal(full); // Inform the consumer that the producer has finished
    pthread_exit(NULL);
}

//---------------------------------------------
// consumer thread prototype
//----------------------------------------------
void *thread2(void *arg) {
    while (1) {
        dispatch_semaphore_wait(full, DISPATCH_TIME_FOREVER);
        pthread_mutex_lock(&mutex);

        bufferItem itemPrint = removeItem();

        pthread_mutex_unlock(&mutex);
        dispatch_semaphore_signal(empty);

        if (itemPrint == -1) {
            break;  // Exit if the special character is encountered
        }

        printf("%c", itemPrint);
        fflush(stdout);

        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    pthread_t producer_thread, consumer_thread;

    empty = dispatch_semaphore_create(BUFFER_SIZE); // Initialize empty using dispatch_semaphore_create
    full = dispatch_semaphore_create(0);             // Initialize full using dispatch_semaphore_create
    counter = 0;

    fflush(stdout);
    /* Required to schedule thread independently.*/
    pthread_mutex_init(&mutex, NULL);

    pthread_t tid1[1]; // process id for thread 1
    pthread_t tid2[1]; // process id for thread 2
    pthread_attr_t attr[1]; // attribute pointer array

    pthread_attr_init(&attr[0]);
    pthread_attr_setscope(&attr[0], PTHREAD_SCOPE_SYSTEM);
    /* end to schedule thread independently */

    /* Create the threads */
    pthread_create(&tid1[0], &attr[0], &thread1, NULL);
    pthread_create(&tid2[0], &attr[0], &thread2, NULL);

    /* Wait for the threads to finish */
    pthread_join(tid1[0], NULL);
    pthread_join(tid2[0], NULL);

    printf("\n------------------------------------------------\n");
    printf("\t\t  End of simulation\n");

    dispatch_release(full); // Release full
    dispatch_release(empty); // Release empty
    pthread_mutex_destroy(&mutex);

    exit(0);
}