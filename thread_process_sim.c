/*
 * Dining Philosophers Problem
 * Operating Systems Project 2
 * Created by: Victorio, Andrew, Amanda
 * Date: 5/01/2025
 * 
 * This program simulates the classic dining philosophers problem
 * where philosophers need to share forks (resources) to eat.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Define constants
#define N 5              // Number of philosophers
#define THINKING 0       // Philosopher is thinking
#define HUNGRY 1         // Philosopher is hungry
#define EATING 2         // Philosopher is eating

// Global variables for synchronization
pthread_mutex_t mutex;   // Mutex for critical sections
pthread_cond_t cond[N];  // Condition variables for each philosopher
int state[N];            // States of philosophers
pthread_t philosophers[N]; // Thread IDs
int burst_times[N];      // Eating times from processes.txt

// Function prototypes
void *philosopher(void *arg);
void take_forks(int i);
void put_forks(int i);
void test(int i);
void think(int i);
void eat(int i);

int main() {
    // Try to load or create processes.txt file
    FILE *check_file = fopen("processes.txt", "r");
    if (!check_file) {
        // File doesn't exist, create it
        printf("Creating processes.txt file...\n");
        FILE *new_file = fopen("processes.txt", "w");
        if (new_file) {
            // Added a header for the file
            fprintf(new_file, "PID Arrival Burst Priority\n");
            fprintf(new_file, "1 0 3 1\n");   // Process 1 data
            fprintf(new_file, "2 0 4 2\n");   // Process 2 data
            fprintf(new_file, "3 0 2 3\n");   // Process 3 data
            fprintf(new_file, "4 0 5 1\n");   // Process 4 data
            fprintf(new_file, "5 0 3 2\n");   // Process 5 data
            fclose(new_file);
            printf("processes.txt file created successfully.\n");
        } else {
            printf("Failed to create processes.txt file.\n");
            return 1;
        }
    } else {
        printf("Found existing processes.txt file.\n");
        fclose(check_file);
    }

    // Read process data from file
    FILE *file = fopen("processes.txt", "r");
    if (!file) {
        perror("Error opening processes.txt");
        return 1;
    }

    // Skip header line
    char buffer[100];
    fgets(buffer, sizeof(buffer), file);

    // Read process info - could add error checking here
    int pid, arrival, burst, priority;
    for (int i = 0; i < N && fscanf(file, "%d %d %d %d", &pid, &arrival, &burst, &priority) == 4; i++) {
        burst_times[i] = burst;
        // Added some debug output
        printf("Philosopher %d will eat for %d seconds\n", i, burst);
    }
    fclose(file);

    printf("\nDining Philosophers Problem Simulation\n");
    printf("=======================================\n");

    // Initialize the mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < N; i++) {
        pthread_cond_init(&cond[i], NULL);
        state[i] = THINKING;  // Everyone starts thinking
    }

    // Create philosopher threads
    int ids[N];
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        // Create the threads - added more detailed error message
        if (pthread_create(&philosophers[i], NULL, philosopher, &ids[i]) != 0) {
            printf("Failed to create philosopher thread %d. Check system resources.\n", i);
            return 1;
        }
    }

    // Wait for all philosophers to finish their meal
    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Clean up resources - destroy mutex and condition variables
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < N; i++) {
        pthread_cond_destroy(&cond[i]);
    }

    printf("\nAll philosophers have completed their meal. Simulation complete.\n");
    return 0;
}

// Each philosopher runs as a separate thread
void *philosopher(void *arg) {
    int id = *(int *)arg;
    printf("[Philosopher %d] Started\n", id);

    // Life cycle of a philosopher: think, get hungry, eat, repeat
    think(id);           // Think for a while
    take_forks(id);      // Get hungry and try to grab forks
    eat(id);             // Eat when both forks acquired
    put_forks(id);       // Put down the forks when done eating

    printf("[Philosopher %d] Finished\n", id);
    return NULL;
}

// Philosopher tries to pick up both forks
void take_forks(int id) {
    pthread_mutex_lock(&mutex);  // Enter critical section
    
    printf("[Philosopher %d] Waiting for forks...\n", id);
    state[id] = HUNGRY;  // Philosopher becomes hungry
    
    test(id);  // Try to acquire forks
    
    // Wait if can't get both forks
    while (state[id] != EATING)
        pthread_cond_wait(&cond[id], &mutex);
        
    pthread_mutex_unlock(&mutex);  // Exit critical section
}

// Philosopher puts down forks after eating
void put_forks(int id) {
    pthread_mutex_lock(&mutex);  // Enter critical section
    
    state[id] = THINKING;  // Back to thinking
    printf("[Philosopher %d] Released forks %d and %d\n", id, id, (id + 1) % N);
    
    // Let neighbors know forks are available
    test((id + N - 1) % N);  // Left neighbor
    test((id + 1) % N);      // Right neighbor
    
    pthread_mutex_unlock(&mutex);  // Exit critical section
}

// Check if philosopher can start eating
void test(int id) {
    // Can only eat if hungry and neighbors aren't eating
    if (state[id] == HUNGRY &&
        state[(id + N - 1) % N] != EATING &&  // Left neighbor not eating
        state[(id + 1) % N] != EATING) {      // Right neighbor not eating
        
        state[id] = EATING;  // Start eating
        printf("[Philosopher %d] Picked up forks %d and %d\n", id, id, (id + 1) % N);
        
        // Signal to waiting philosopher that they can eat now
        pthread_cond_signal(&cond[id]);
    }
}

// Philosopher thinks
void think(int id) {
    printf("[Philosopher %d] Thinking...\n", id);
    // Simulate thinking time - could randomize this 
    sleep(1);  // Think for 1 second
}

// Philosopher eats
void eat(int id) {
    printf("[Philosopher %d] Eating for %d seconds...\n", id, burst_times[id]);
    // Simulate eating for burst_time seconds from processes.txt
    sleep(burst_times[id]);
}
