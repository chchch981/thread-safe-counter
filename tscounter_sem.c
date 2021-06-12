#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>


//#define PATH "/home"


#define MAX_BUF 200

union semun {
    int val;
    struct semid_ds* buf;
    ushort* array;
};

typedef struct __counter_t {
    int value;
    //pthread_mutex_t lock;
    union semun arg;
    int semid;
    //struct sembuf s;

} counter_t;

unsigned int loop_cnt;
counter_t counter;


void init(counter_t* c, key_t key, char* argv[]) {
    
    c->semid = semget(key, 1, 0600 | IPC_CREAT);
    printf("semid :  %d\n", c->semid);
    if (c->semid < 0) {
        perror(argv[0]);
        exit(1);
    }

    c->arg.val = 1;
    semctl(c->semid, 0, SETVAL, c->arg);


    c->value = 0;

    printf("init\n");
   // pthread_mutex_init(&c->lock, NULL);
}

void lock(counter_t* c) {
    
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = -1;
    s.sem_flg = 0;
    semop(c->semid, &s, 1);
    //printf("locking --\n");
}

void unlock(counter_t* c) {
    
    struct sembuf s;
    s.sem_num = 0;
    s.sem_op = 1;
    s.sem_flg = 0;
    semop(c->semid, &s, 1);
    //printf("unlocking --\n");
}


void increment(counter_t* c) {
   // pthread_mutex_lock(&c->lock);
    lock(&counter);


    c->value++;
   // pthread_mutex_unlock(&c->lock);

    unlock(&counter);
}

//void decrement(counter_t* c) {
//    pthread_mutex_lock(&c->lock);
//    c->value--;
//    pthread_mutex_unlock(&c->lock);
//}

int get(counter_t* c) {
    //pthread_mutex_lock(&c->lock);
    lock(c);

    int rc = c->value;
    //pthread_mutex_unlock(&c->lock);
    unlock(c);

    return rc;
}

void* mythread(void* arg)
{
    char* letter = arg;
    int i;

    printf("%s: begin\n", letter);
    for (i = 0; i < loop_cnt; i++) {
        //lock(&counter);
        increment(&counter);
       // unlock(&counter);
        
    }
    printf("%s: done\n", letter);
    return NULL;
}



int main(int argc, char* argv[])
{
    clock_t start1, end1;
    float res1;
    key_t key;
    

    char path[MAX_BUF];

    getcwd(path, MAX_BUF);
    printf("Current working directory: %s\n", path);

    key = ftok(path, 'z');
    if (key < 0) {
        perror(argv[0]);
        exit(1);
    }

    //semid = semget(key, 1, 0600 | IPC_CREAT);
    //printf("semid :  %d\n", semid);
    //if (semid < 0) {
    //    perror(argv[0]);
    //    exit(1);
    //}

    loop_cnt = atoi(argv[1]);
    
    init(&counter, key, argv);

    pthread_t p1, p2;

    start1 = clock();
    printf("main: begin [counter = %d]\n", get(&counter));
    

    pthread_create(&p1, NULL, mythread, "A");
    pthread_create(&p2, NULL, mythread, "B");
    // join waits for the threads to finish
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    printf("main: done [counter: %d] [should be: %d]\n", get(&counter), loop_cnt * 2);

    end1 = clock();
    res1 = (float)(end1 - start1) / CLOCKS_PER_SEC;
    printf("time : %f ", res1);
    return 0;
}
