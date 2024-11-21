#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

int a_print = 0, b_print = 0, c_print = 0;
int line_count;

sem_t sem1;

void init_lock() {
    sem_init(&sem1, 0, 1);
}

void destroy_lock() {
    sem_destroy(&sem1);
}

void *thread_p(void *arg) {
    while (true) {
        sem_wait(&sem1);
        if (line_count == 0) {
            sem_post(&sem1);
            break;
        }

        cout << "A";
        a_print++;
        if (a_print >= 1 && b_print >= 1 && c_print >= 1) {
            cout << endl;
            a_print = 0;
            b_print = 0;
            c_print = 0;
            line_count--;
        }
        sem_post(&sem1);
    }
    return NULL;
}

void *thread_q(void *arg) {
    while (true) {
        sem_wait(&sem1);
        if (line_count == 0) {
            sem_post(&sem1);
            break;
        }

        cout << "B";
        b_print++;
        if (a_print >= 1 && b_print >= 1 && c_print >= 1) {
            cout << endl;
            a_print = 0;
            b_print = 0;
            c_print = 0;
            line_count--;
        }
        sem_post(&sem1);
    }
    return NULL;
}

void *thread_r(void *arg) {
    while (true) {
        sem_wait(&sem1);
        if (line_count == 0) {
            sem_post(&sem1);
            break;
        }

        cout << "C";
        c_print++;
        if (a_print >= 1 && b_print >= 1 && c_print >= 1) {
            cout << endl;
            a_print = 0;
            b_print = 0;
            c_print = 0;
            line_count--;
        }
        sem_post(&sem1);
    }
    return NULL;
}

int main() {
    cin >> line_count;

    init_lock();

    pthread_t pt_p, pt_q, pt_r;

    pthread_create(&pt_p, NULL, thread_p, NULL);
    pthread_create(&pt_q, NULL, thread_q, NULL);
    pthread_create(&pt_r, NULL, thread_r, NULL);

    pthread_join(pt_p, NULL);
    pthread_join(pt_q, NULL);
    pthread_join(pt_r, NULL);

    destroy_lock();
    return 0;
}