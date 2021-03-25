#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>

//gcc -Wall -pthread main.c -o main

#define COUNT 100
#define TH_NUMBER 2
#define SORT_TH_NUMBER 10

int counter = 0;
sem_t sem;
FILE *fp;
int* numbers;

struct LinearGenerator{
    // x\new\ = ( A * x\old\ + c ) % M
    int A;
    int c;
    int M;
    int x;
};

void init(struct LinearGenerator* gen,int A,int c, int M,int x)
{
    gen->A=A;
    gen->c=c;
    gen->M=M;
    gen->x=x;
}

void merge(int ar1[], int ar2[], int m, int n)
{
    for (int i = n - 1; i >= 0; i--)
    {
        int j, last = ar1[m - 1];

        for (j = m - 2; j >= 0 && ar1[j] > ar2[i]; j--) {
            ar1[j + 1] = ar1[j];
        }

        if (j != m - 2 || last > ar2[i])
        {
            ar1[j + 1] = ar2[i];
            ar2[i] = last;
        }
    }
}

int nextNumber(struct LinearGenerator* gen)
{
    int xNew = (gen->A*gen->x+gen->c) % gen->M;
    gen->x=xNew;
    return xNew;
}

void *genThread(void *inputGen){
    int i=COUNT/TH_NUMBER;
    int number;
    struct LinearGenerator* gen=(struct LinearGenerator*) inputGen;

    while (i--) {
        number=nextNumber(gen);

        sem_wait(&sem);
        fprintf(fp,"%d\n",number);
        numbers[counter]=number;
        counter++;
        sem_post(&sem);
    }

    return NULL;

}

int cmpfunc (const void * a, const void * b) {
    return ( *(int*)a - *(int*)b );
}

void *sortThread(void *vp){
    int range=*((int*)vp);

    qsort(numbers+(COUNT*range)/SORT_TH_NUMBER, 10, sizeof(int), cmpfunc);
    return NULL;

}

int main() {
    struct LinearGenerator gen,gen2;
    pthread_t t1,t2;

    pthread_t* sorting_threads=malloc(sizeof(pthread_t)*SORT_TH_NUMBER);

    numbers=(int*)malloc(sizeof(int)*COUNT);

    if(sem_init(&sem,0,1)!=0){
        perror("Semaphore init failed\n");
        return 1;
    }

    fp=fopen("./numbers","w+");

    init(&gen,69065,1,1000,15);
    pthread_create(&t1,NULL,genThread,(void*)&gen);

    init(&gen2,96066,2,1000,515);
    pthread_create(&t2,NULL,genThread,(void*)&gen2);

    pthread_join(t1,NULL);
    pthread_join(t2,NULL);

    fclose(fp);



    for(int i=0;i<SORT_TH_NUMBER;i++)
    {
        int* arg = malloc(sizeof(int));
        *arg=i;
        pthread_create(&sorting_threads[i],NULL,sortThread,arg);
    }

    for(int i=0;i<SORT_TH_NUMBER;i++)
    {
        pthread_join(sorting_threads[i],NULL);
    }

    //    qsort(numbers, COUNT, sizeof(int), cmpfunc);

    int table_elems=10;

    for (int i = 0; i < COUNT/SORT_TH_NUMBER - 1;i++)
    {
        merge(numbers,numbers+(i+1)*10,table_elems,10);
        table_elems+=10;
    }

    for(int i=0;i<COUNT;i++)
    {
        printf("%d\n",numbers[i]);
    }

    return 0;
}
