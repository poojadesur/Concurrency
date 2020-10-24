#include<stdio.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>

#define min(a,b) (a<b?a:b)

int minpreptime;
int maxpreptime;
int minbatches;
int maxbatches;
int minstudents;
int maxstudents;
int n,m,o;

typedef struct company_struct{
    int id;
    int preptime;
    int numbatches;
    float probsuccess;
    int numvaccines;
    int setprod;   // flag that represents if company should begin reproduction 
} company_struct;

typedef struct zone_struct{
    int id;
    int slots;
    int numvaccines;
    float probsuccess;
    int beginvaccines;           //bool that represents if all of a zones slots are full or not
    int vaccinesover;
    int slotsover;
    int compid; 
    int reslot;         // bool that tells zone when to start reslotting again
    int phase;           // current number of students vaccinating at one time
    int phaseover;
}zone_struct;

typedef struct student_struct{
    int id;
    int vaccines_received;
    float probsuccess;
    int zone;
    int state; // -1 hasnt come anywhere, 1 waiting at gate, 2 being vaccinated,3 - getting tested for antibodies, 4 - either at IIIT or sent home
}student_struct;

company_struct* company;
zone_struct* zone;
student_struct* student;

int student_track;
pthread_mutex_t studentmutex;

int student_gate;
pthread_mutex_t studentgatemutex;

int companybatch[1024]; //holds number of batches each company has left currently before having to remake

pthread_mutex_t studentlock[1024];
pthread_mutex_t companylock[1024];
pthread_mutex_t zonelock[1024];

void* do_student(void *arg)
{
   int i = *((int *)arg);
    
    while(student[i].state != 4)
    {
        pthread_mutex_lock(&studentlock[i]); //necessary as other threads are accessing student state -- think!
        if(student[i].state == -1) //thread has been sent to gate
        {
            int cs = student[i].vaccines_received;
            switch(cs)
            {
                case 0: 
                    printf("\033[1;36m Student %d has arrived for their 1st round of Vaccinations.\n",i+1); 
                    break;
                case 1: 
                    printf("\033[1;36m Student %d has arrived for their 2nd round of Vaccinations.\n",i+1);
                    break;
                case 2: 
                    printf("\033[1;36m Student %d has arrived for their 3rd round of Vaccinations.\n",i+1);
                    break;
            }

            pthread_mutex_lock(&studentgatemutex);
            student_gate++;
            pthread_mutex_unlock(&studentgatemutex);

            printf("\033[1;36m Student %d is waiting to be allocated a slot on a Vaccination Zone\n\n",i+1);
            student[i].state = 1;

        }
        pthread_mutex_unlock(&studentlock[i]);

        //after being allocated a slot, student only gets vaccinated when all the slots have been filled at the zone
        if(student[i].state == 2 && zone[student[i].zone].beginvaccines == 1) 
        {
            printf("\033[1;35m Student %d on Vaccination Zone %d is being vaccinated\n\n",i+1,student[i].zone+1);
            sleep(1);
            printf("\033[1;35m Student %d on Vaccination Zone %d has been vaccinated which has success probability %f\n\n",i+1,student[i].zone+1,student[i].probsuccess);

            pthread_mutex_lock(&studentlock[i]);
            student[i].state = 3;
            pthread_mutex_unlock(&studentlock[i]);

        }

        if(student[i].state == 3)
        {
            int perc = (student[i].probsuccess) * 100;

            int check = (rand() % (100 - 0 + 1));

            if(check < perc )
            {
                printf("\033[0;31m Student %d has tested positive for antibodies and can come back to IIIT.\n\n",i+1);
                
                pthread_mutex_lock(&studentlock[i]);
                student[i].state = 4;
                pthread_mutex_unlock(&studentlock[i]);

                pthread_mutex_lock(&studentmutex);
                student_track--;
                if(student_track == 0) printf("\033[0;31m All the students have been administered a vaccine\n\n");
                pthread_mutex_unlock(&studentmutex);
            }

            else
            {
                if(student[i].vaccines_received == 3)
                {
                    printf("\033[1;31m Student %d has tested negative for antibodies. Student can be vaccinated %d more times.\n\n",i+1,3-student[i].vaccines_received);
                    printf("\033[0;31m Student %d has been administered 3 vaccines unsuccessfully and is being sent home.\n\n",i+1);

                    pthread_mutex_lock(&studentlock[i]);
                    student[i].state = 4;
                    pthread_mutex_unlock(&studentlock[i]);
                    
                    pthread_mutex_lock(&studentmutex);
                    student_track--;
                    if(student_track == 0) printf("\033[0;31m All the students have been administered a vaccine\n\n");
                    pthread_mutex_unlock(&studentmutex);
                }

                else
                {
                    printf("\033[1;31m Student %d has tested negative for antibodies. Student can be vaccinated %d more times.\n\n",i+1,3-student[i].vaccines_received);
                    
                    pthread_mutex_lock(&studentlock[i]);
                    student[i].state = -1;
                    pthread_mutex_unlock(&studentlock[i]);
                }
                
            }

            if(student_track == 0) break;

            pthread_mutex_lock(&zonelock[student[i].zone]);
            zone[student[i].zone].vaccinesover++;
            zone[student[i].zone].slotsover++;
            zone[student[i].zone].phaseover++;

            //when all students have been vaccinate at current phase, zone can look for more slots
            if(zone[student[i].zone].phaseover == zone[student[i].zone].phase)
            {
                zone[student[i].zone].beginvaccines = 0;
            }
        
            //when all students at a zone for a certain slot have all been vaccinated, allow zone to create new slots
            if(zone[student[i].zone].slotsover == zone[student[i].zone].slots)
            { 
                //when all students at a zone have been vaccinated then zone can reslot
                zone[student[i].zone].reslot = 1;
                // printf("\nhii\n");
            }
            //if all students at a zone have been vaccinated and there are no batches left, company can start producting again
            if(zone[student[i].zone].vaccinesover == company[zone[student[i].zone].compid].numvaccines)
            {
                printf("\033[1;34m Vaccination Zone Y has run out of vaccines\n\n");

                if(companybatch[zone[student[i].zone].compid]== 0)
                {
                    printf("\033[1;32m All the vaccines prepared by Pharmaceutical Company %d are emptied. Resuming production now.\n",zone[student[i].zone].compid+1);
                    company[zone[student[i].zone].compid].setprod = 1;
                }
            }
            pthread_mutex_unlock(&zonelock[student[i].zone]);

        }

        if(student[i].state == 4) break;
    }

    return NULL;

}

int zone_get_batch(int i)
{
    int received = 0;   //bool represents if zone has received a batch yet
    int compid = -1;

    while(received == 0)
    {
        if(student_track == 0) break;

        for(int j=0;j<n;j++)
        {
            if(student_track == 0) break; 

            pthread_mutex_lock(&companylock[j]);
            if(companybatch[j] != 0)
            {
                zone[i].numvaccines = company[j].numvaccines;
                zone[i].probsuccess = company[j].probsuccess;

                pthread_mutex_lock(&zonelock[i]);
                zone[i].vaccinesover = 0;
                pthread_mutex_unlock(&zonelock[i]);
            
                companybatch[j]--;

                received = 1;
                zone[i].compid = compid;
                compid = j;
                printf("\033[1;33m Pharmaceutical Company %d is delivering a vaccine batch to Vaccination Zone %d which has success probability %f\n",j+1,i+1,company[j].probsuccess);
                pthread_mutex_unlock(&companylock[j]);
                break;
            }
            pthread_mutex_unlock(&companylock[j]);
        }
    }

    printf("\033[1;33m Pharmaceutical Company %d has delivered vaccines to Vaccination zone %d, resuming vaccinations now.\n\n",compid + 1,i+1);

    return compid;

}


void* do_zone(void* arg)
{
   int i = *((int *)arg);

    while(student_track)
    {
        if(student_track == 0) break;

        int compid = zone_get_batch(i);

        //should only break after all students have been vaccinated
        while(zone[i].numvaccines)
        {
            if(student_track == 0) break;

            if(student_gate && zone[i].reslot == 1)
            {
                pthread_mutex_lock(&zonelock[i]);
                zone[i].reslot = 0;
                zone[i].slotsover = 0;

                zone[i].slots = min(8,min(student_gate,zone[i].numvaccines)); //all slots are not always filled at once ; only fill till number of students at gate
                printf("\033[1;34m Vaccination Zone %d is ready to vaccinate with %d slots. \n\n",i+1,zone[i].slots);
                zone[i].numvaccines -= zone[i].slots;
                pthread_mutex_unlock(&zonelock[i]);
    
                //while slots are left at vaccination phase dont reslot
                while(zone[i].slotsover != zone[i].slots)
                {
                    if(student_track == 0) break;

                    //start allocating slots when a vaccination phase is over
                    if(zone[i].beginvaccines == 0)
                    {
                        int slotcount = 0;  // number of kids at slots 

                        pthread_mutex_lock(&zonelock[i]);
                        zone[i].phase = 0;
                        zone[i].phaseover = 0;
                        pthread_mutex_unlock(&zonelock[i]);

                        for(int j=0;j<o;j++)
                        {
                            pthread_mutex_lock(&studentlock[j]);  
                            if(student[j].state == 1)  //if a student in waiting at the gate
                            {
                                student[j].state = 2;
                                student[j].zone = i;    //ensure a student only starts vaccinating after all the slots have been filled up -- while(student.state == 2 && zone[student.zone].filled == 1)
                                student[j].vaccines_received++;
                                student[j].probsuccess = zone[i].probsuccess;

                                slotcount++;
                                
                                pthread_mutex_lock(&zonelock[i]);
                                zone[i].phase++;   //so student knows when to signal to stop vaccinating (when all students are over)
                                pthread_mutex_unlock(&zonelock[i]);

                                pthread_mutex_lock(&studentgatemutex);
                                student_gate--;
                                pthread_mutex_unlock(&studentgatemutex);
                            
                                printf("\033[1;35m Student %d assigned a slot on the Vaccination Zone %d and waiting to be vaccinated\n",j+1,i+1);

                            }
                            pthread_mutex_unlock(&studentlock[j]);

                            if(slotcount == zone[i].slots) break;

                            //vaccination zone enters phase when students currently at gate == 0;
                            if(student_gate == 0) break;
                        }
                        
                        printf("\n");
                        printf("\033[1;34m Vaccination Zone %d entering Vaccination Phase.\n\n",i+1);

                        pthread_mutex_lock(&zonelock[i]);
                        zone[i].beginvaccines = 1;
                        pthread_mutex_unlock(&zonelock[i]);

                    }
                }

            }

            if(student_track == 0) break;
        }
    }
    return NULL;
}


void* do_company(void* arg)
{
    int i = *((int *)arg);
    
    while(student_track)
    {
        while(companybatch[i] == 0 && company[i].setprod == 1)
        {
            //making a batch of vaccines
            company[i].preptime =  (rand() % (maxpreptime - minpreptime + 1) ) + minpreptime;

            // pthread_mutex_lock(&companylock[i]);
            company[i].numbatches =  (rand() % (maxbatches - minbatches + 1)) + minbatches;
            // pthread_mutex_unlock(&companylock[i]);

            company[i].numvaccines =  (rand() % (maxstudents - minstudents + 1)) + minstudents;

            pthread_mutex_lock(&companylock[i]);
            company[i].setprod = 0;
            pthread_mutex_unlock(&companylock[i]);

            printf("\033[1;32m Pharmaceutical Company %d is preparing %d batches of vaccines which can vaccinate %d students each, have success probability %f\n\n",i + 1,company[i].numbatches,company[i].numvaccines,company[i].probsuccess);

            sleep(company[i].preptime);

            printf("\033[1;32m Pharmaceutical Company %d has prepared %d batches of vaccines which have success probability %f.\n Waiting for all the vaccines to be used to resume production\n\n",i + 1,company[i].numbatches,company[i].probsuccess);

            pthread_mutex_lock(&companylock[i]);
            companybatch[i] = company[i].numbatches;
            pthread_mutex_unlock(&companylock[i]);

        }
    }
    return NULL;
}

int main()
{
    
    printf("Number of Pharmacetutical companies: ");
    scanf("%d",&n);
    printf("Number of Vaccination zones: ");
    scanf("%d",&m);
    printf("Number of Students: ");
    scanf("%d",&o);


    company = (struct company_struct*)malloc(sizeof(struct company_struct)*n);
    zone = (struct zone_struct*)malloc(sizeof(struct zone_struct)*m);
    student = (struct student_struct*)malloc(sizeof(struct student_struct)*o);

    student_track = o;
    student_gate = 0;

    minpreptime = 2;
    maxpreptime = 5;
    minbatches = 1;
    maxbatches = 5;
    minstudents = 10;
    maxstudents = 20;

    srand(time(NULL));
    
    pthread_t zone_thread[m];
    pthread_t student_thread[o];
    pthread_t company_thread[n];

    printf("Enter success probabilities for each company respectively;\n");
    for(int i=0;i<n;i++)
    {
        float s; scanf("%f",&s);
        company[i].probsuccess = s;
        company[i].numbatches = 0;
        company[i].numvaccines = 0; 
        companybatch[i] = 0;  
        company[i].setprod = 1;
        company[i].id = i;
    }
    for(int i=0;i<m;i++)
    {
        zone[i].id = i;
        zone[i].beginvaccines = 0;  
        zone[i].reslot = 1;
        zone[i].slotsover = 0;
    }

    for(int i=0;i<o;i++)
    {
        student[i].id = i;
        student[i].state = -1; 
        student[i].vaccines_received = 0;
    }

    if(n == 0 || m == 0 || o == 0)
    {
         printf("\033[0m Please enter valid inputs\n");
         return 0;
    }

    printf("\033[0m \n\n*****STARTING SIMULATION*****\n\n\n\n");

    for(int i=0;i<n;i++)
    {
        pthread_create(&company_thread[i], NULL, do_company,(void*)(&company[i].id));
    }
    for(int i=0;i<m;i++)
    {
        pthread_create(&zone_thread[i], NULL, do_zone,(void*)(&zone[i].id));
    }
    for(int i=0;i<o;i++)
    {
        pthread_create(&student_thread[i], NULL, do_student,(void*)(&student[i].id));
    }
    
    for(int i=0;i<n;i++)
        pthread_join(company_thread[i],NULL);
    
    for(int i=0;i<m;i++)
        pthread_join(zone_thread[i],NULL);
    
    for(int i=0;i<o;i++)
        pthread_join(student_thread[i],NULL);

    printf("\033[0m \n\n*****SIMULATION OVER*****\n\n\n\n");

}