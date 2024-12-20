#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <random>

using namespace std;

int a_count = 0, b_count = 0, c_count = 0;
int N, L;

sem_t sem1;

void init_lock() {
    sem_init(&sem1, 0, 1);
}

void destroy_lock() {
    sem_destroy(&sem1);
}

void *thread_function_A(void *arg) {
    while (true) {
        sem_wait(&sem1);
        if (N == 0) {
            sem_post(&sem1);
            break;
        }

        cout << "A";
        a_count++;
        if (a_count + b_count + c_count == L) {
            cout << "$" << endl;
            a_count = 0;
            b_count = 0;
            c_count = 0;
            N--;
        }

        sem_post(&sem1);
        //sleep(1);
    }
    return NULL;
}

void *thread_function_B(void *arg) {
    while (true) {
        sem_wait(&sem1);
        if (N == 0) {
            sem_post(&sem1);
            break;
        }

        cout << "B";
        b_count++;
        if (a_count + b_count + c_count == L) {
            cout << "$" << endl;
            a_count = 0;
            b_count = 0;
            c_count = 0;
            N--;
        }

        sem_post(&sem1);
        //sleep(1);
    }
    return NULL;
}

void *thread_function_C(void *arg) {
    while (true) {
        sem_wait(&sem1);
        if (N == 0) {
            sem_post(&sem1);
            break;
        }

        cout << "C";
        c_count++;
        if (a_count + b_count + c_count == L) {
            cout << "$" << endl;
            a_count = 0;
            b_count = 0;
            c_count = 0;
            N--;
        }

        sem_post(&sem1);
        //sleep(1);
    }
    return NULL;
}

int main() {
    cin >> N >> L;

    init_lock();

    pthread_t thread_A, thread_B, thread_C;

    // pthread_attr_t attr;
    // pthread_attr_init(&attr);
    // pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    // pthread_attr_setschedpolicy(&attr, SCHED_RR);

    // pthread_create(&thread_A, &attr, thread_function_A, NULL);
    // pthread_create(&thread_B, &attr, thread_function_B, NULL);
    // pthread_create(&thread_C, &attr, thread_function_C, NULL);

    pthread_create(&thread_A, NULL, thread_function_A, NULL);
    pthread_create(&thread_B, NULL, thread_function_B, NULL);
    pthread_create(&thread_C, NULL, thread_function_C, NULL);

    pthread_join(thread_A, NULL);
    pthread_join(thread_B, NULL);
    pthread_join(thread_C, NULL);

    destroy_lock();
    return 0;
}