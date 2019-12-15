
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // for usleep()
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h> // for shm
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/syscall.h>  // get pid
#include <pthread.h>
#include <math.h>    // use exp
#include <semaphore.h>

#define SIZE sizeof(int) * 20

typedef struct params{
    int *ptr; //point to the beginning of shm
    int argv; //argv;
    int count; // number of product
    int current; //current product

    sem_t *empty; //number of empty shm
    sem_t *full;  //number of full shm

    pthread_mutex_t mutex;
    //pthread_cond_t condition;
} params;

void *producer(params *params){

    srand(time(NULL));
    pid_t pid = (int)syscall(__NR_getpid); //id of process
    pid_t tid = (int)syscall(__NR_gettid); //id of thread

    double rate;
    double sleeping_time;

    while(1){
        
        rate = (rand() % 10) / 10.0;  //rate
        //sleeping_time = params->argv * exp(params->argv * random * -1) * 1000000;
        sleeping_time = (log(params->argv *1.0 / rate) / params->argv) * 1000000;
        usleep(sleeping_time);

        pthread_mutex_lock(&params->mutex);
        sem_wait(params->empty);
        params->ptr[params->current] = rand();   // the random value producted
        params->count += 1;

        printf("procID is [%d] | threadID is [%d] | value is [%d]\n", pid, tid, params->ptr[params->current]);
        
        params->current  = (params->current + 1) % 20; //when current = 20, set it to 0;
        
        sem_post(params->full);
        pthread_mutex_unlock(&params->mutex);
    }
}

int main(int argc, char **argv){

    sem_t *empty; 
    empty = sem_open("empty", O_CREAT, 0666, 20); //initialize to 20
    sem_t *full;
    full = sem_open("full", O_CREAT, 0666, 0); // initialize to 0

    const char *shm_name = "shm";
    int shm_fd;
    int state;
    void *addr; // the address of shm

    /*create named shm shared memory*/
    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    //printf("%d\n", shm_fd);
    if (shm_fd == -1){
        printf("shm failed!\n");
        return -1;
    }

    /*at the same time, we should call ftruncate function to set its size*/
    state = ftruncate(shm_fd, SIZE);
    if (state == -1){
        printf("ftruncate failed!\n");
        return -1;
    }

    /*map the shared memory in the address addr*/
    addr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (addr == MAP_FAILED){
        printf("map failed!\n");
        return -1;
    }

    params *param = (params*)malloc(sizeof(params));
    param->argv = atoi(argv[1]);
    param->ptr = (int*)addr;
    param->empty = empty;
    param->full = full;
    param->count = 0;
    param->current = 0;

    pthread_t tid[3]; //three threads
    pthread_attr_t attr[3];

    /*initialization*/
    int i;
    for (i = 0; i < 3; i++){
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], (void *)producer, param);
    }

    for (i = 0; i < 3; i++){
        pthread_join(tid[i], NULL);
    }

    return 0;
}

