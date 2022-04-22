
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>

typedef struct Buddy {
    pid_t pid;
    size_t size;
    int valid; /* 1 for valid (occupied) 0 for else */
} Buddy;

#define MIN_REQ 128
#define MAX_REQ 4096

#define MAX_SEG 262144

// function declaration
void normalize_size(int* reqsizeaddr) {
    int norm = 1;
    while((*reqsizeaddr) > norm || norm < MIN_REQ) {
        norm = norm * 2;
    }

    (*reqsizeaddr) = norm;
}

// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.
#define SHM_NAME "/eray1"
int shm_fd;

// Define semaphore(s)
#define SEM_NAME "/eray2"
sem_t* sem;

#define SEM_PROC "/eray3"
sem_t* sem_proc;

// Define your stuctures and variables.
Buddy* head;
void* shm_content;
int segsize;

#define MAX_PROCESS_COUNT 10

void* nextious(Buddy* cur){
    return (void *)cur + cur->size;
}

void* previous(Buddy* cur){
    return (void *)cur - cur->size;
}

Buddy* buddyAlgo(int reqsize, int segsize) {
    Buddy* cur = head;
    int min = MAX_SEG;
    Buddy* prev = previous(cur); // cur->prev;
    int length = 0;
    while(length < segsize) {              //finds first node with min size 

        if(reqsize <= cur->size && cur->size < min && !cur->valid ) {
           min = cur->size;
        }

        length += cur->size;
        cur = nextious(cur);
    }
    
    if(min == MAX_SEG && nextious(head)) { return NULL; }
    cur = head;

    length = 0;
    while (length < segsize) {
        if (min == cur->size && cur->valid == 0) {
            while(cur->size > reqsize) {
                Buddy b = { getpid(), cur->size / 2, 0, };
                memcpy( (void*)cur + cur->size / 2, &b, sizeof(Buddy));
                cur->size = cur->size / 2;
            }
            cur->valid = 1;
            return cur;
        }
        prev = cur;
        length += cur->size;
        cur = nextious(cur); //  cur->next;
    }
    return NULL;
}

int sbmem_init(int segmentsize)
{
    shm_unlink(SHM_NAME);

    sem_unlink(SEM_NAME);
    sem_unlink(SEM_PROC);
    
    sem = sem_open(SEM_NAME, O_RDWR | O_CREAT, 0666, 1);
    sem_proc = sem_open(SEM_PROC, O_RDWR | O_CREAT, 0666, MAX_PROCESS_COUNT);

    // * Create shm for memory management
    if ((shm_fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666)) < 0) {
        perror("Failed to create shm\n");
        sem_destroy(sem);
        return -1;
    }
    size_t temp_shm_size = sizeof(int) + segmentsize;

    ftruncate(shm_fd, temp_shm_size);
    void* segsizeloc = mmap(0, temp_shm_size , PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    memcpy(segsizeloc, &segmentsize, sizeof(int));

    head = segsizeloc + sizeof(int);

    //bura yeni geldi
    head->size = segmentsize;
    head->valid = 0;
    head->pid = 0;

    return (0); 
}

int sbmem_remove()
{
    sem_destroy(sem);
    shm_unlink(SHM_NAME);

    return (0); 
}

// node|segment
// ! SIZE|PROCESS_TABLE|MEMORY|
// buddy nodes are included in the memory
int sbmem_open()
{
    sem = sem_open(SEM_NAME, O_RDWR, 0666, 1);
    sem_wait(sem);
    if ((shm_fd = shm_open(SHM_NAME, O_RDWR, 0666)) < 0) {
        // BU SATIR BİR ŞAHESERDİR LÜTFEN DOKANMAYINIZ *shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
        //                                                             ^ 
        //                                                             |
        //                                                             |
        // ^_^ The pinnacle of software engineering achieved by the one and only prodigy himself

        perror("Failed to create shm\n");
        sem_destroy(sem);
        return -1;
    }
    

    sem_proc = sem_open(SEM_PROC, O_RDWR, 0666, MAX_PROCESS_COUNT);
                                                                    //   kebab case
                                                                   //      s
    int avail_proc_count;                                         //     s          
    sem_getvalue(sem_proc, &avail_proc_count);                   //o-O-o-8-----¯\(｡◕‿‿◕｡)マ
    if (avail_proc_count == 0){                                 //  ~~~~~~~                 
        return -1;
    }
    sem_wait(sem_proc);

    size_t temp_shm_size = sizeof(int);
    // get segsize
    shm_content = mmap(0, temp_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_content == MAP_FAILED) { printf("SHM MAP FAILED\n"); return -1; }
    memcpy(&segsize, shm_content, sizeof(int));

    temp_shm_size += segsize;
    shm_content = mmap(0, temp_shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    head = shm_content + sizeof(int);
    sem_post(sem);

    return (0); 
}

void* sbmem_alloc (int reqsize)
{
    if(reqsize > MAX_REQ) {printf("CAN'T ALLOC MORE THAN 4096!!\n"); return NULL;}
    if(reqsize < MIN_REQ) {printf("CAN'T ALLOC LESS THAN 128!!\n"); return NULL;}

    sem_wait(sem);
    
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    //eski yeri
    void* result = NULL;
    reqsize += sizeof(Buddy);
    int original_size = reqsize; 
    normalize_size(&reqsize);

    for (size_t i = 0; i < MAX_PROCESS_COUNT; i++) {

        Buddy* new;
        if(!(new = buddyAlgo(reqsize, segsize))){ //search ilk bulduğu node'u dönüyo yoksa null
            return NULL;
        }

        result = (void*) new + sizeof(Buddy);
        break;
    }

    sem_post(sem);
    return result;
}

typedef struct Stack{
    struct Stack* prev;
    struct Stack* next;
    int element;
}Stack;

Stack* sp = NULL;

Stack* push(int i){
    Stack* temp = sp;
    sp = malloc(sizeof(Stack));
    sp->prev = NULL;
    sp->next = NULL;
    sp->element = i;
    if(temp){
        sp->prev = temp;
        temp->next = sp;
    }
    return sp;
}

void pop(){
    Stack* temp = sp;
    sp = sp->prev;
    free(temp);
}

void sbmem_free (void *p)
{
    sem_wait(sem);

    p -= sizeof(Buddy);
    Buddy* cur = head;
    Buddy* prev = NULL;
    Stack* psp = NULL;

    
    int length = 0;
    while(length < segsize){
        sp = push(cur->size);

        if( (cur == p && psp && (void*)prev >= head && sp->element == psp->element && cur->size == prev->size && prev->valid == 0) //kendisi p, prev 0
            ||  ((void*)prev >= head && prev == p && sp->element == psp->element && cur->size == prev->size && cur->valid == 0)  ){ //prev p, kendisi 0
                //birleştir
                do{
                    prev->size = prev->size * 2;
                    prev->valid = 0;
                    cur->valid = 0;
                    cur = prev;
                    prev = previous(prev);
                    p = (void*)cur;
                } while(cur && (void *)prev >= head  && (prev->size) == cur->size && prev->valid == 0);
        }

        else if(( (void *)prev >= head && psp && prev == p && (cur->size != prev->size || sp->element != psp->element)) //single
                || ( (void *)prev >= head && psp && prev == p && cur->size == prev->size && sp->element == psp->element && cur->valid == 1) // prev 1 p, cur 1
             ){
                prev->valid = 0;
        }
        else if(( (void *) prev >= head && psp && cur == p && cur->size == prev->size && sp->element == psp->element && prev->valid == 1)){ // prev 1 cur 1 p 
                cur->valid = 0;
        }

        while (sp && psp && sp->element == psp->element) { //stack işlemleri
            Stack* temp = sp;  
            sp = psp;
            sp->element = sp->element * 2;
            psp = psp->prev;
            free(temp);
        }

        psp = sp;
        prev = cur;
        if(length >= segsize){cur = head;}
        length += cur->size;
        cur = nextious(cur);
    }

    while(sp){
        pop();
    }

    
    sem_post(sem);
}

int sbmem_close()
{
    sem_post(sem_proc);
    return (0); 
}
