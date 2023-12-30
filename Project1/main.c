// Name: Diya Jain
// Username: diyajain
// Student ID: U81673988

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

// Function prototypes for child processes
void process1(int* total);
void process2(int* total);
void process3(int* total);
void process4(int* total);

int main() {
  // Create a shared memory segment to store the total
  int shmid = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
  // Attach the shared memory segment
  int* total = shmat(shmid, NULL, 0);
  *total = 0; // Initialize the program total

  // Fork four child processes
  int id1 = fork();
  if (id1 == 0) {
    process1(total); // Child process 1
    exit(0);
  }

  int id2 = fork();
  if (id2 == 0) {
    process2(total); // Child process 2
    exit(0);
  }

  int id3 = fork();
  if (id3 == 0) {
    process3(total); // Child process 3
    exit(0);
  }

  int id4 = fork();
  if (id4 == 0) {
    process4(total); // Child process 4
    exit(0);
  }

  // Wait for all child processes to finish
  wait(NULL);
  printf("From Process 1: counter = %d\n", *total);
  printf("Child with ID:%d has been exited.\n", id1);

  wait(NULL);
  printf("From Process 2: counter = %d\n", *total);
  printf("Child with ID:%d has been exited.\n", id2);

  wait(NULL);
  printf("From Process 3: counter = %d\n", *total);
  printf("Child with ID:%d has be exited.\n", id3);

  wait(NULL);
  printf("From Process 4: counter = %d\n", *total);
  printf("Child with ID:%d has been exited.\n", id4);

  // Detach and remove the shared memory segment
  shmdt(total);
  shmctl(shmid, IPC_RMID, NULL);

  printf("End of Simulation\n");

  return 0;
}

// Child processes increment the shared total
void process1(int* total) {
  for (int i = 0; i < 1000000; i++) {
    (*total)++;
  }
}

void process2(int* total) {
  for (int i = 0; i < 2000000; i++) {
    (*total)++;
  }
}

void process3(int* total) {
  for (int i = 0; i < 4000000; i++) {
    (*total)++;
  }
}

void process4(int* total) {
  for (int i = 0; i < 5000000; i++) {
    (*total)++;
  }
}
