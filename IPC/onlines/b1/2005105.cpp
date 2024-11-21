#include<iostream>
#include<semaphore.h>
#include<pthread.h>
#include<unistd.h>

using namespace std;

sem_t dash_sem,plus_sem;

void* dash_func(void* arg){
    int n = *(int*)arg;
    int limit = (n+1)/2;
    for(int i=n-1;i>=limit;i--){
        sem_wait(&dash_sem);
        for(int j=0; j<=i;j++){
            cout<<"_";
        }
        sem_post(&plus_sem);
    }
    pthread_exit(NULL);
}

void* plus_func(void* arg){
    int n = *(int*)arg;
    int limit = n/2;
    for(int i=1;i<=limit;i++){
        sem_wait(&plus_sem);
        for(int j=1; j<=i;j++){
            cout<<"+";
        }
        cout<<endl;
        sem_post(&dash_sem);
    }
    pthread_exit(NULL);
}

int main(){
    sem_init(&dash_sem, 0, 1);
    sem_init(&plus_sem, 0, 0);
    pthread_t dash_thread, plus_thread;
    int n;
    cout<<"Enter a number: ";
    cin>>n;
    pthread_create(&dash_thread, NULL, dash_func, (void*)&n);
    pthread_create(&plus_thread, NULL, plus_func, (void*)&n);

    pthread_join(dash_thread, NULL);
    pthread_join(plus_thread, NULL);

    return 0;
}