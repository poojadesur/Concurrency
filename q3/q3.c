#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<semaphore.h>
#include<string.h>

int k,a,e,c,t1,t2,t;

typedef struct performer_struct{
    int id;
    char name[50];
    char instrument;
    int arrivaltime;
    int state; //0 havent arrived, 1 arrived
    int stage;
    int clear;
} performer_struct;

typedef struct stage_struct{
    int id;
    char player[50]; //name of performer currently performing
    int state; // 0 nobody playing
}stage_struct;

performer_struct* performer; 
stage_struct* stage;

pthread_mutex_t performerlock[1024];
pthread_mutex_t stagelock[1024];
pthread_mutex_t semlock = PTHREAD_MUTEX_INITIALIZER; 

pthread_mutex_t player2lock = PTHREAD_MUTEX_INITIALIZER; 
pthread_cond_t stagesig[1024];

sem_t ac;
sem_t el;
sem_t doubleacel;
sem_t coord;

void *singer(void *arg)
{
    int i = *((int*)arg);

    sleep(performer[i].arrivaltime);

    pthread_mutex_lock(&performerlock[i]);
    
    if(performer[i].state == 0)
    {
        printf("\033[1;32m %s %c arrived at Srujana.\n\n",performer[i].name,performer[i].instrument);
        performer[i].state = 1;
    }
    pthread_mutex_unlock(&performerlock[i]);

    //get current time in order to time wait
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    ts.tv_sec += (t);

    int ret = sem_timedwait(&doubleacel, &ts);
    
    if(ret == -1)
    {
        pthread_mutex_lock(&performerlock[i]);
        if(performer[i].state == 1) printf("\033[1;32m %s got impatient and left Srujana el\n\n",performer[i].name);
        performer[i].state = 3;
        pthread_mutex_unlock(&performerlock[i]);
        return NULL;
    }

    for(int j=0;j<a+e;j++)
    {
        pthread_mutex_lock(&stagelock[j]);
        if(stage[j].state == 0 || stage[j].state == 1)
        {
            pthread_mutex_lock(&performerlock[i]);
            stage[j].state++;
            // if stage was previously unoccupied, save name of player playing on stage
            if(stage[j].state == 1)
            {
                strcpy(stage[j].player,performer[i].name);
                // if a singer is playing solo, other singers shouldnt join them
                stage[j].state = 3;
                //need semlock?!?!?
                pthread_mutex_lock(&semlock);
                if(j >= 0 && j <= a-1) sem_wait(&ac);
                else if(j >= a && j <= a + e -1) sem_wait(&el); 
                pthread_mutex_unlock(&semlock);
            }

            performer[i].state = 2;
            performer[i].stage = j;
            pthread_mutex_unlock(&performerlock[i]);
            pthread_mutex_unlock(&stagelock[j]);
            break;
        }
        pthread_mutex_unlock(&stagelock[j]);
    }

    if(stage[performer[i].stage].state == 3)
    {
        int dur = (rand() % (t2 - t1 + 1)) + t1;
        if(performer[i].stage >= 0 && performer[i].stage <= a-1)
        {
            printf("\033[1;34m %s performing %c at acoustic stage %d for %d seconds\n\n",performer[i].name,performer[i].instrument,performer[i].stage+1,dur);
            sleep(dur);
            pthread_mutex_lock(&stagelock[performer[i].stage]);
            stage[performer[i].stage].state = 0;
            pthread_mutex_unlock(&stagelock[performer[i].stage]);
            printf("\033[1;35m %s performance at acoustic stage %d ended\n\n",performer[i].name,performer[i].stage+1);
        }
        else if(performer[i].stage >= a && performer[i].stage <= a + e -1)
        {
            printf("\033[1;34m %s performing %c at electric stage %d for %d seconds\n\n",performer[i].name,performer[i].instrument,performer[i].stage+1,dur);
            sleep(dur);
            pthread_mutex_lock(&stagelock[performer[i].stage]);
            stage[performer[i].stage].state = 0;
            pthread_mutex_unlock(&stagelock[performer[i].stage]);
            printf("\033[1;35m %s performance at electric stage %d ended\n\n",performer[i].name,performer[i].stage+1);
        }
    }

    if(stage[performer[i].stage].state == 2)
    {
        printf("\033[1;33m %s joined %s's performance, performance extended by 2 seconds.\n\n",performer[i].name,stage[performer[i].stage].player);
        pthread_mutex_lock(&player2lock);
        pthread_cond_wait(&stagesig[performer[i].stage],&player2lock);
        pthread_mutex_unlock(&player2lock);
        // printf("\033[1;33m joint performance %s s and %s ended\n\n",performer[i].name,stage[performer[i].stage].player);
    }

    sem_post(&doubleacel);

    sem_wait(&coord);
    printf("\033[1;31m %s collecting tshirt\n\n",performer[i].name);
    sem_post(&coord);

    return NULL;

}

void* electric(void *arg)
{
    int i = *((int*)arg);

    sleep(performer[i].arrivaltime);

    pthread_mutex_lock(&performerlock[i]);
    
    if(performer[i].state == 0)
    {
        printf("\033[1;32m %s %c arrived at Srujana.\n\n",performer[i].name,performer[i].instrument);
        performer[i].state = 1;
    }
    pthread_mutex_unlock(&performerlock[i]);

    //get current time in order to time wait
    
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    ts.tv_sec += (t);

    int ret = sem_timedwait(&el, &ts);

    pthread_mutex_lock(&performerlock[i]);
    if(performer[i].clear == 1)
    {
        sem_post(&el);
        pthread_mutex_unlock(&performerlock[i]);
        return NULL;
    }
    performer[i].clear = 1;
    pthread_mutex_unlock(&performerlock[i]);
    

    if(ret == -1)
    {
        pthread_mutex_lock(&performerlock[i]);
        if(performer[i].state == 1) printf("\033[1;32m %s got impatient and left Srujana el\n\n",performer[i].name);
        // performer[i].state = 3;

        pthread_mutex_unlock(&performerlock[i]);
        return NULL;
    }

    for(int j=a;j<a+e;j++) // 0 to a-1 are ac stages a to a+e-1 are el stages
    {
        pthread_mutex_lock(&stagelock[j]);
        if(stage[j].state == 0)
        {
            pthread_mutex_lock(&performerlock[i]);
            performer[i].state = 2;
            stage[j].state = 1; // being played on by 1 person
            performer[i].stage = j;
            strcpy(stage[j].player,performer[i].name); //save name of player performing on stage j
            pthread_mutex_unlock(&performerlock[i]);
            pthread_mutex_unlock(&stagelock[j]);
            break;
        }
        pthread_mutex_unlock(&stagelock[j]);
    }

   
    int dur = (rand() % (t2 - t1 + 1)) + t1;

    printf("\033[1;34m %s performing %c at electric stage %d for %d seconds\n\n",performer[i].name,performer[i].instrument,performer[i].stage+1,dur);
    
    sleep(dur);

    //if a singer joined the stage while performer was playing
    if(stage[performer[i].stage].state == 2)
    {
        sleep(2);
        pthread_cond_signal(&stagesig[performer[i].stage]);
    }

    pthread_mutex_lock(&stagelock[performer[i].stage]);
    stage[performer[i].stage].state = 0;
    pthread_mutex_unlock(&stagelock[performer[i].stage]);

    printf("\033[1;35m %s performance at electric stage %d ended.\n\n",performer[i].name,performer[i].stage+1);

    sem_post(&el);

    sem_wait(&coord);
    printf("\033[1;31m %s collecting tshirt\n\n",performer[i].name);
    sem_post(&coord);

    return NULL;
    
}

void* acoustic(void *arg)
{
    int i = *((int*)arg);

    sleep(performer[i].arrivaltime);

    pthread_mutex_lock(&performerlock[i]);
    if(performer[i].state == 0)
    {
        printf("\033[1;32m %s %c arrived at Srujana.\n\n",performer[i].name,performer[i].instrument);
        performer[i].state = 1;
    }
    pthread_mutex_unlock(&performerlock[i]);

    //get current time in order to time wait 
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    ts.tv_sec += (t);

    int ret = sem_timedwait(&ac, &ts);

    pthread_mutex_lock(&performerlock[i]);
    if(performer[i].clear == 1)
    {
        sem_post(&ac);
        pthread_mutex_unlock(&performerlock[i]);
        return NULL;
    }
    //performer has access to some stage if clear is 1
    performer[i].clear = 1;
    pthread_mutex_unlock(&performerlock[i]);
    
    if(ret == -1)
    {
        pthread_mutex_lock(&performerlock[i]);
        if(performer[i].state == 1) printf("\033[1;32m %s got impatient and left Srujana ac\n\n",performer[i].name);
        performer[i].state = 3; //he only leaves srujana from one thread
        pthread_mutex_unlock(&performerlock[i]);
        return NULL;
    }

    for(int j=0;j<a;j++) // 0 to a-1 are ac stages a to a+e-1 are el stages
    {
        pthread_mutex_lock(&stagelock[j]);
        if(stage[j].state == 0)
        {
            pthread_mutex_lock(&performerlock[i]);
            performer[i].state = 2;
            stage[j].state = 1; // being played on by 1 person
            performer[i].stage = j;
            strcpy(stage[j].player,performer[i].name); //save name of player performing on stage j
            pthread_mutex_unlock(&performerlock[i]);
            pthread_mutex_unlock(&stagelock[j]);
            break;
        }
        pthread_mutex_unlock(&stagelock[j]);
    }

    int dur = (rand() % (t2 - t1 + 1)) + t1;

    printf("\033[1;34m %s performing %c at acoustic stage %d for %d seconds\n\n",performer[i].name,performer[i].instrument,performer[i].stage+1,dur);
    
    sleep(dur);

    //if a singer joined the stage while performer was playing
    if(stage[performer[i].stage].state == 2)
    {
        sleep(2);
        pthread_cond_signal(&stagesig[performer[i].stage]);
    }

    pthread_mutex_lock(&stagelock[i]);
    stage[i].state = 0;
    pthread_mutex_unlock(&stagelock[i]);

    printf("\033[1;35m %s performance at acoustic stage %d ended.\n\n",performer[i].name,performer[i].stage+1);

    sem_post(&ac);
    
    sem_wait(&coord);
    printf("\033[1;31m %s collecting tshirt\n\n",performer[i].name);
    sem_post(&coord);

    return NULL;

}

int main()
{
    scanf("%d %d %d %d %d %d %d",&k,&a,&e,&c,&t1,&t2,&t);

    if(c == 0)
    {
        printf("There has to be at least one club coordinator for things to run smoothly!");
        return 0;
    }

    performer = (struct performer_struct*)malloc(sizeof(struct performer_struct)*k);
    stage = (struct stage_struct*)malloc(sizeof(struct stage_struct)*(a+e));

    for(int i=0;i<k;i++)
    {
        scanf("%s %c %d",performer[i].name,&performer[i].instrument,&performer[i].arrivaltime);
        performer[i].id = i;
        performer[i].state = 0;
        performer[i].clear = 0;
    } 
    for(int i=0;i<a+e;i++)
    {
        stage[i].state = 0;
    }

    pthread_t performer_acoustic_thread[k];
    pthread_t performer_electric_thread[k];
    pthread_t performer_singer_thread[k];

    srand(time(NULL));

    sem_init(&ac,0,a); //check how to initialise semaphore value
    sem_init(&el,0,e);
    sem_init(&doubleacel,0,a+e);
    sem_init(&coord,0,c);

    //first a stages are acoustic stages and next e are electric stages (acoustic stage ids start from 1 to a and electric stage ids start from a+1 to a+e)
    
    printf("\033[0m \n\n\n\n****SIMULATION STARTING****\n\n\n\n\n");

    for(int i=0;i<k;i++)
    {
        if(performer[i].instrument == 'v' || performer[i].instrument == 's') continue;
        pthread_create(&performer_electric_thread[i],NULL,electric,(void*)&performer[i].id);
    }

    for(int i=0;i<k;i++)
    {
        if(performer[i].instrument == 'b' || performer[i].instrument == 's') continue;
        pthread_create(&performer_acoustic_thread[i],NULL,acoustic,(void*)&performer[i].id);
    }

    for(int i=0;i<k;i++)
    {
        if(performer[i].instrument != 's') continue;
        pthread_create(&performer_singer_thread[i],NULL,singer,(void*)&performer[i].id);
    }

    for(int i=0;i<k;i++)
    {
        if(performer[i].instrument == 'v' || performer[i].instrument == 's') continue;
        pthread_join(performer_electric_thread[i],NULL);
    }

    for(int i=0;i<k;i++)
    {
        if(performer[i].instrument != 's') continue;
        pthread_join(performer_singer_thread[i],NULL);
    }

    for(int i=0;i<k;i++)
    {
        if(performer[i].instrument == 'b' || performer[i].instrument == 's') continue;
        pthread_join(performer_acoustic_thread[i],NULL);
    }

    printf("\033[0m \n\n\n****SIMULATION OVER****\n\n\n\n");

}