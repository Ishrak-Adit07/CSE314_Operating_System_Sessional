#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

int iter;
int print_count = 3;
int step = 1;
int step_p = 1, step_q = 1, step_r = 1;
int p_iter, q_iter, r_iter;

sem_t sem1;

void init_lock(){
    sem_init(&sem1, 0, 1);
}

void destroy_lock(){
    sem_destroy(&sem1);
}

void *thread_p(void *arg){
    while(true){
        if(step_p){
            sem_wait(&sem1);
            for(int j = 0; j < step; j++){
                cout << "p";
            }
            print_count--;
            step_p = 0;
            if(print_count == 0){
                cout << endl;
                print_count = 3;
                step++;
                step_p = 1;
                step_q = 1;
                step_r = 1;
            }
            sem_post(&sem1);
            p_iter--;
        }
        if(p_iter == 0) break;
    }
    return NULL;
}

void *thread_q(void *arg){
    while(true){
        if(step_q){
            sem_wait(&sem1);
            for(int j = 0; j < step; j++){
                cout << "q";
            }
            print_count--;
            step_q = 0;
            if(print_count == 0){
                cout << endl;
                print_count = 3;
                step++;
                step_p = 1;
                step_q = 1;
                step_r = 1;
            }
            sem_post(&sem1);
            q_iter--;
        }
        if(q_iter == 0) break;
    }
    return NULL;
}

void *thread_r(void *arg){
    while(true){
        if(step_r){
            sem_wait(&sem1);
            for(int j = 0; j < step; j++){
                cout << "r";
            }
            print_count--;
            step_r = 0;
            if(print_count == 0){
                cout << endl;
                print_count = 3;
                step++;
                step_p = 1;
                step_q = 1;
                step_r = 1;
            }
            sem_post(&sem1);
            r_iter--;
        }
        if(r_iter == 0) break;
    }
    return NULL;
}

int main(){
    cin >> iter;
    p_iter = iter;
    q_iter = iter;
    r_iter = iter;
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