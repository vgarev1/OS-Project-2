#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 5
#define THINKING 0
#define HUNGRY 1
#define EATING 2

pthread_mutex_t mutex;
pthread_cond_t cond[N];
int state[N];
pthread_t philosophers[N];
int burst_times[N] = {3, 4, 2, 5, 3}; // Hardcoded burst times instead of reading from file

void *philosopher(void *arg);
void take_forks(int i);
void put_forks(int i);
void test(int i);
void think(int i);
void eat(int i);

int main() {
    printf("Dining Philosophers Problem Simulation\n");
    printf("---------------------------------------\n");
    printf("Using hardcoded process times: [3, 4, 2, 5, 3]\n");

    // Initialize mutex and condition variables
    pthread_mutex_init(&mutex, NULL);
    for (int i = 0; i < N; i++) {
        pthread_cond_init(&cond[i], NULL);
        state[i] = THINKING;
    }

    // Create philosopher threads
    int ids[N];
    for (int i = 0; i < N; i++) {
        ids[i] = i;
        if (pthread_create(&philosophers[i], NULL, philosopher, &ids[i]) != 0) {
            printf("Failed to create philosopher thread %d\n", i);
            return 1;
        }
    }

    // Wait for all philosophers to finish
    for (int i = 0; i < N; i++) {
        pthread_join(philosophers[i], NULL);
    }

    // Clean up resources
    pthread_mutex_destroy(&mutex);
    for (int i = 0; i < N; i++) {
        pthread_cond_destroy(&cond[i]);
    }

    printf("All philosophers have completed their meal.\n");
    return 0;
}

// Philosopher thread function
void *philosopher(void *arg) {
    int id = *(int *)arg;
    printf("[Philosopher %d] Started\n", id);

    think(id);
    take_forks(id);
    eat(id);
    put_forks(id);

    printf("[Philosopher %d] Finished\n", id);
    return NULL;
}

// Functions to manage forks and state
void take_forks(int id) {
    pthread_mutex_lock(&mutex);
    printf("[Philosopher %d] Waiting for forks...\n", id);
    state[id] = HUNGRY;
    test(id);
    while (state[id] != EATING)
        pthread_cond_wait(&cond[id], &mutex);
    pthread_mutex_unlock(&mutex);
}

// Function to release forks and test neighbors
void put_forks(int id) {
    pthread_mutex_lock(&mutex);
    state[id] = THINKING;
    printf("[Philosopher %d] Released forks %d and %d\n", id, id, (id + 1) % N);
    test((id + N - 1) % N);  // Left neighbor
    test((id + 1) % N);      // Right neighbor
    pthread_mutex_unlock(&mutex);
}

// Function to test if a philosopher can eat
void test(int id) {
    if (state[id] == HUNGRY &&
        state[(id + N - 1) % N] != EATING &&
        state[(id + 1) % N] != EATING) {
        state[id] = EATING;
        printf("[Philosopher %d] Picked up forks %d and %d\n", id, id, (id + 1) % N);
        pthread_cond_signal(&cond[id]);
    }
}

// Functions to simulate thinking and eating
void think(int id) {
    printf("[Philosopher %d] Thinking...\n", id);
    sleep(1);  // Think for 1 second
}

// Function to simulate eating
void eat(int id) {
    printf("[Philosopher %d] Eating for %d seconds...\n", id, burst_times[id]);
    sleep(burst_times[id]);  // Eat for burst_time seconds
}
