// OS Homework 3

// Figure 29.8 Implement Concurrent Linked List: Rewritten + hand over hand locking 
// along with benchmarking 

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#define OPS_PER_THREAD 100000 // number of operations that each thread performs 
#define INITIAL_KEYS 1000 // key range for random operations 



//Figure 29.8 Implementaion Code 
// node structure 
typedef struct node_t {
    int key;
    struct node_t *next;
} node_t;

typedef struct list {
    node_t *head; //this is pointer to first node 
    pthread_mutex_t lock; //this is single global mutex
} list_t;
//this initializes list and the and the lock
void List_Init(list_t *L) {
    L->head = NULL;
    pthread_mutex_init(&L->lock, NULL);
}
//insert at the head 
int List_Insert(list_t *L, int key) {
    node_t *new = malloc(sizeof(node_t)); //new memory for the new node 
    if (new == NULL) {
        perror("malloc");
        return -1;
    }
    new->key = key;
    pthread_mutex_lock(&L->lock); // this locks the whole list before making changes 
    new->next = L->head;
    L->head = new;
    pthread_mutex_unlock(&L->lock); //unlocks after changes 

    return 0;
}

//lookup
int List_t_Lookup(list_t *L, int key) {
    int rv = -1;
    pthread_mutex_lock(&L->lock); // locks whole list for traversal 
    node_t *curr = L->head;
    while (curr != NULL) {
        if (curr->key == key) {
            rv = 0;
            break;
        }
        curr = curr->next;
    }
    pthread_mutex_unlock(&L->lock);
    return rv;
}

// Hand Over Hand List
typedef struct node_u{ // node with lock (own lock)
    int key;
    struct node_u *next;
    pthread_mutex_t lock;
} node_u;

typedef struct{ // this is the list structure 
    node_u *head;
} list_u;
// same as before bbut this im initializes HOH list 
void List_u_Init(list_u *L){
    L->head = NULL; 
}
// insert at head but for other node 
void List_u_Insert(list_u *L, int key){
    node_u *new = malloc(sizeof(node_u));
    new->key = key;
    pthread_mutex_init(&new->lock, NULL);

    new->next = L->head;
    L->head = new;
}
// HOH lookupn 
int List_u_lookup(list_u *L, int key){
    node_u *curr = L-> head;
    if(!curr) return 0; // this gives an empty list 

    pthread_mutex_lock(&curr ->lock);

    while(curr){
        if (curr->key == key){
            pthread_mutex_unlock(&curr->lock);
            return 1;
        }
    
        node_u *next = curr->next;
        if(next)
            pthread_mutex_lock(&next->lock);
        pthread_mutex_unlock(&curr->lock);
        curr = next; 
    }
    return 0;
}

//Benchmark 
typedef struct{
    int id; 
    int threads; 
    int workload;
    list_t *lt;
    list_u *lu;
    int Use_HOH;
}thread_arg; 

void *work(void *arg){
    thread_arg *t =(thread_arg*)arg;
    for (int i = 0; i < OPS_PER_THREAD; i++){
        int r = rand()% 100;
        int key = rand() % INITIAL_KEYS;
        if(t->workload == 0){
            if(r <90){
                if(t->Use_HOH)
                    List_t_Lookup(t->lt, key);
                else
                    List_u_lookup(t->lu,key);
            }else{
                if(!t->Use_HOH)
                    List_u_Insert(t->lu, key);
            }
            }
        

        else if(t->workload == 1){
            if(r<50){
                if(t->Use_HOH)
                    List_t_Lookup(t->lt,key);
                else
                    List_u_lookup(t->lu, key);
            } else {
                if(!t->Use_HOH)
                    List_u_Insert(t->lu, key);
            }
        }
        else{
            if(t->Use_HOH)
                List_t_Lookup(t->lt, key);
            else
                List_u_lookup(t->lu, key);
    
        }
    }
    return NULL;
}
double runBenchmark(int threads, int workload, int Use_HOH){
    pthread_t tid[threads];
    thread_arg args[threads];

    list_t lt;
    list_u lu; 
    if(Use_HOH)
        List_u_Init(&lu);
    else
        List_Init(&lt);
    struct timespec start, end; 
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < threads; i++) {
        args[i].id = i;
        args[i].threads = threads; 
        args[i].workload = workload;
        args[i].Use_HOH = Use_HOH;
        args[i].lt = &lt;
        args[i].lu = &lu;

        pthread_create(&tid[i], NULL, work, &args[i]);
    }
    for (int i = 0; i < threads; i++)
        pthread_join(tid[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    return time;
}
// MAIN
int main(){
    int thread_counts[] = {1, 2, 4, 8, 16};
    int workloads[] = {0, 1, 2};

    for (int w = 0; w < 3; w++) {
        printf("\nWorkload %d\n", w);

        for (int i = 0; i < 5; i++) {
            int t = thread_counts[i];

            double lock_time = runBenchmark(t, w, 0);
            double HOH_time = runBenchmark(t, w, 1);

            printf("Threads = %d  Lock = %.4f  HOH = %.4f\n",
                    t, lock_time, HOH_time);
        }
    }

    return 0;
}


