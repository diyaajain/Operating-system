// Name: Diya Jain
// Username: diyajain
// Student ID: U81673988

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>

// Define constants
#define SHM_KEY 400L
#define SEM_KEY 401L
#define NUM_CHILDREN 4
#define TOTAL_TARGET 11000000

// Function prototypes
int createSharedMemory();
int createSemaphore();
void P(int sem_id);
void V(int sem_id);
void process(int sem_id, int *shared_memory, int target, int process_num);

int main() {
    // Create shared memory and semaphore
    int shm_id = createSharedMemory();
    int sem_id = createSemaphore();

    // Initialize shared memory to 1
    int *shared_memory = (int *)shmat(shm_id, NULL, 0);
    *shared_memory = 1;

    // Fork child processes
    for (int i = 1; i <= NUM_CHILDREN; i++) {
        pid_t pid = fork();

        if (pid == 0) { // Child process
            int target = i * TOTAL_TARGET / NUM_CHILDREN;
            process(sem_id, shared_memory, target, i); // Call the process function

            exit(0);
        } else if (pid < 0) {
            perror("Fork error");
            exit(1);
        }
    }

    // Parent process waits for children
    int child_pid;
    for (int i = 1; i <= NUM_CHILDREN; i++) {
        child_pid = wait(NULL);
        printf("Child with ID %d has just exited.\n", child_pid);
    }

    // Cleanup and release resources
    shmdt(shared_memory);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);

    printf("End of Simulation.\n");

    return 0;
}

// Function to create shared memory segment
int createSharedMemory() {
    int shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_id < 0) {
        perror("shmget error");
        exit(1);
    }
    return shm_id;
}

// Function to create semaphore
int createSemaphore() {
    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id < 0) {
        perror("semget error");
        exit(1);
    }
    semctl(sem_id, 0, SETVAL, 1); // Initialize semaphore value to 1
    return sem_id;
}

// P operation for semaphore (wait)
void P(int sem_id) {
    struct sembuf operation = {0, -1, 0};
    if (semop(sem_id, &operation, 1) == -1) {
        perror("semop P error");
        exit(1);
    }
}

// V operation for semaphore (signal)
void V(int sem_id) {
    struct sembuf operation = {0, 1, 0};
    if (semop(sem_id, &operation, 1) == -1) {
        perror("semop V error");
        exit(1);
    }
}

// Function to perform the increment logic in the critical section
void process(int sem_id, int *shared_memory, int target, int process_num) {
    int increment = 1;

    while (*shared_memory < target) {
        P(sem_id); // Enter critical section
        if (*shared_memory < target) {
            (*shared_memory) += increment; // Increment shared memory
        }
        V(sem_id); // Exit critical section
    }

    printf("From Process %d: counter = %d.\n", process_num, *shared_memory);
}