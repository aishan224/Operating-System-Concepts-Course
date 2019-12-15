//Cited book page 191
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define N 5

enum STATE {
    THINKING, HUNGRY, EATING
};
enum STATE current_state[N];

pthread_t dphtid[N];     //tids
pthread_attr_t attr[N];
pthread_mutex_t mutex;  //only one, I had set mutex[N], but result went wrong!
pthread_cond_t conditons[N];
int dphid[N] = {0, 1, 2, 3, 4};  //philosophers' ids

void *runner(void *param);
void pickup_forks(int i);
void return_forks(int i);
void process(int i);

int main(){

    //to create random thinking time and eating time
    srand(time(NULL)); 
    int i;

    /*initialization*/
    /*initialize attributes*/
    for (i = 0; i < N; i++){
        pthread_attr_init(&attr[i]);
    }

    /*initialize threads*/
    for (i = 0; i < N; i++){
        pthread_create(&dphtid[i], &attr[i], runner, &dphid[i]);
        //at the same time, set its current state to THINKING;
        current_state[i] = THINKING;
    }
    /*wait threads*/
    for (i = 0; i < N; i++){
        pthread_join(dphtid[i], NULL);
    }

    /*initialize condition variables and mutex*/
    pthread_mutex_init(&mutex, NULL);
    for (i = 0; i < N; i++){
        pthread_cond_init(&conditons[i], NULL);
    }
}

void *runner(void *param){

    int id = *(int *)param; //get the value of param
    while (1){
        int thinking_time = rand() % 100;
        sleep(thinking_time / 10);
        pickup_forks(id);

        int eating_time = rand() % 100;
        sleep(eating_time / 10);
        return_forks(id);
        
    }
}

void pickup_forks(int i){
    /*No.i is hungry, call this function*/
    //set its current state
    current_state[i] = HUNGRY;
    printf("No.%d --> Hungry, waiting to eat\n", i);

    //test whether get enough chops
    process(i);

    // when no.i is hungry but there is no chops available, just wait
    pthread_mutex_lock(&mutex);
    if(current_state[i] != EATING){
        pthread_cond_wait(&conditons[i], &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void return_forks(int i){
    //No.i has eaten 
    current_state[i] = THINKING;
    printf("No.%d --> End eating\n", i);

    //Check if the person next to him needs chopsticks, 
    //Or the result will always is the fixed two man eating or thinking(for example 2 and 4 eating or thinking in turns)
    process((i+4) % 5);  //right one
    process((i+1) % 5);  //left one
}

void process(int i){

    pthread_mutex_lock(&mutex);
    if ((current_state[i] == HUNGRY) && (current_state[(i+1)%5] != EATING) && (current_state[(i+4)%5] != EATING)){
        //if his right and left are both not eating
        current_state[i] = EATING;
        printf("No.%d --> Eating\n", i);
        pthread_cond_signal(&conditons[i]);
    }
    pthread_mutex_unlock(&mutex);
}
