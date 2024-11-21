#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include <iomanip>
#include <chrono>

#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

using namespace std;

sem_t out_lock;
sem_t pdo, qdo, rdo;


int N;
int tc = 0;

void tc_handle()
{
    tc++;
    if (tc == 3)
    {
        cout << endl;
        tc = 0;
        sem_post(&pdo);
        sem_post(&qdo);
        sem_post(&rdo);
        // cout<<"reset\n";
    }
}

void initialize_system()
{
    sem_init(&out_lock, 0, 1);

    sem_init(&pdo, 0, 1);
    sem_init(&qdo, 0, 1);
    sem_init(&rdo, 0, 1);
}

void destroy()
{
    sem_destroy(&out_lock);

    sem_destroy(&pdo);
    sem_destroy(&qdo);
    sem_destroy(&rdo);
}

void *p_activities(void *args)
{
    for (int i = 0; i < N; i++)
    {

        sem_wait(&pdo);
        sem_wait(&out_lock);
        for (int j = 0; j <= i; j++)
        {
            cout << "p";
        }

        tc_handle();

        sem_post(&out_lock);
    }
    return NULL;
}

void *q_activities(void *args)
{
    for (int i = 0; i < N; i++)
    {
        sem_wait(&qdo);
        sem_wait(&out_lock);
        for (int j = 0; j <= i; j++)
        {
            cout << "q";
        }

        tc_handle();

        sem_post(&out_lock);
    }
    return NULL;
}

void *r_activities(void *args)
{
    for (int i = 0; i < N; i++)
    {
        sem_wait(&rdo);
        sem_wait(&out_lock);
        for (int j = 0; j <= i; j++)
        {
            cout << "r";
        }

        tc_handle();

        sem_post(&out_lock);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage: ./a.out N" << endl;
        return 0;
    }
    N = stoi(argv[1]);
    initialize_system();

    pthread_t p_t;
    pthread_t q_t;
    pthread_t r_t;

    pthread_create(&p_t, NULL, p_activities, NULL);
    pthread_create(&q_t, NULL, q_activities, NULL);
    pthread_create(&r_t, NULL, r_activities, NULL);

    pthread_join(p_t, NULL);
    pthread_join(q_t, NULL);
    pthread_join(r_t, NULL);

    destroy();

    return 0;
}