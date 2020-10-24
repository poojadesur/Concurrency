#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

int *threaded_arr;

int tot = 8;
int chunk;
int remaining;
int n;

pthread_mutex_t lock;

int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 


void selection_sort(int arr[], int n)
{
    int i, j, min_idx;  

    for (i = 0; i < n-1; i++)  
    {    
        min_idx = i;  
        for (j = i+1; j < n; j++)  
        if (arr[j] < arr[min_idx])  
            min_idx = j;  
            
        swap(&arr[min_idx], &arr[i]);  
    }  
}

void merge(int arr[], int l, int m, int r) 
{ 
    int n1 = m - l + 1; 
    int n2 = r - m; 
  
    int leftarr[n1], rightarr[n2]; 
  
    for(int i = 0; i < n1; i++) 
        leftarr[i] = arr[l + i]; 
    for(int j = 0; j < n2; j++) 
        rightarr[j] = arr[m + 1 + j]; 
  
    int i = 0;  
    int j = 0;  
    int k = l; 
      
    while (i < n1 && j < n2) 
    { 
        if (leftarr[i] <= rightarr[j])  
        { 
            arr[k] = leftarr[i]; 
            i++; 
        } 
        else 
        { 
            arr[k] = rightarr[j]; 
            j++; 
        } 
        k++; 
    } 
  
    while (i < n1)  
    { 
        arr[k] = leftarr[i]; 
        i++; 
        k++; 
    } 
  
    while (j < n2) 
    { 
        arr[k] = rightarr[j]; 
        j++; 
        k++; 
    } 
} 

void concurrestMS(int a[], int l , int h)
{
    int length = (h - l + 1);

    if(length < 5)
    {
        selection_sort(a + l, length);
        return;
    }

    pid_t leftpid,rightpid; 
    leftpid = fork(); 
    
    if (leftpid==0) 
    { 
        concurrestMS(a,l,l+length/2-1); 
        exit(0); 
    } 
    else
    {
        rightpid = fork();     
        if(rightpid==0) 
        { 
            concurrestMS(a,l+length/2,h); 
            exit(0); 
        } 
    }

    int status; 
  
    // Waiting for all the child processes to finish 
    waitpid(leftpid, &status, 0); 
    waitpid(rightpid, &status, 0); 
    merge(a, l, l+length/2-1, h);

}

void mergesort(int a[], int l, int r)
{
    int length = (r - l + 1);

    if(length < 5)
    {
        selection_sort(a+l,length);
        return ;
    }

    int mid = (l + r) / 2;
    if(l < r)
    {
        mergesort(a,l,mid);
        mergesort(a,mid+1,r);

        merge(a,l,mid,r);
    }
}

void* threadedMS(void* input)
{
    int i = *((int *)input);

    int l = (i) * (chunk);
    int r = (i + 1) * (chunk) - 1;

    if(i == (tot - 1))
    {
         r += remaining;
    }

    if(l > r)
        return NULL;

    int length = (r - l + 1);

    if(length < 5)
    {
        selection_sort(threaded_arr+l,length);
        return NULL;
    }
 
    mergesort(threaded_arr,l,r);
    
}

void merge_final(int arr[] , int num , int ele)
{
	for(int i=0;i<num;i = i+2)
	{
		int l = i*(chunk * ele);
		int r = (i+2)*(chunk * ele) - 1;
		int mid = l + (chunk * ele) - 1;
		
        if(r >= n)r = n-1;

		merge(arr , l , mid , r);
	}

	if((num /2) >= 1) merge_final(arr , num/2 , ele*2);
}

void runsorts(int n)
{

    //normal mergesorted array
    int normal[n];

    // concurrent mergesorted array gets saved in concurrent
    int *concurrent = shareMem(sizeof(int) * (n + 1));

    //threaded mergesorted array
    threaded_arr = (int *)malloc(sizeof(int) * n);
    
    for(int i=0;i<n;i++)
    {
        scanf("%d",&concurrent[i]);
        normal[i] = concurrent[i];
        threaded_arr[i] = concurrent[i];
    }

    struct timespec ts;
    long double start,end;

    //NORMAL :

    printf("\n\nNormal Merge sort\n\n");
    printf("The array before sorting is:\n");
    for(int i=0;i<n;i++)printf("%d ",normal[i]);
    printf("\n\n");

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    start = ts.tv_nsec / (1e9) + ts.tv_sec;

    mergesort(normal,0,n-1);

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    end = ts.tv_nsec / (1e9) + ts.tv_sec;

    printf("Mergesort ran normally time = %Lf\n\n", end - start);

    printf("The array after sorting is:\n");
    for (int i = 0; i < n; i++)printf("%d ", normal[i]);
    printf("\n\n");

    //CONCURRENT : 

    printf("\n\nConcurrent Merge Sort:\n\n");
    printf("The array before sorting is:\n");
    for(int i=0;i<n;i++)printf("%d ",concurrent[i]);
    printf("\n\n");

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    start = ts.tv_nsec / (1e9) + ts.tv_sec;

    concurrestMS(concurrent,0,n-1);

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    end = ts.tv_nsec / (1e9) + ts.tv_sec;

    printf("Mergesort using concurrency time = %Lf\n\n", end - start);

    printf("The array after sorting:\n");
    for (int i = 0; i < n; i++)printf("%d ", concurrent[i]);
    printf("\n\n");

    //THREADED:

    chunk = n / tot;
    remaining = n % tot;

    pthread_t threads[tot];

    printf("\n\nThreaded Merge sort\n\n");
    printf("The array before sorting is:\n");
    
    for(int i=0;i<n;i++)printf("%d ",threaded_arr[i]);
    printf("\n\n");

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    start = ts.tv_nsec / (1e9) + ts.tv_sec;

    for (int i = 0; i < tot; i++) 
    {
        pthread_create(&threads[i], NULL, threadedMS,(void*)(&i));  
    }

    for (int i = 0; i < tot; i++) 
    {    
        pthread_join(threads[i], NULL); 
    }

    merge_final(threaded_arr,tot,1);

    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) perror("Clock get time error");
    end = ts.tv_nsec / (1e9) + ts.tv_sec;

    printf("Mergesort with threads time = %Lf\n\n", end - start);

    printf("The array after sorting:\n");
    for (int i = 0; i < n; i++)printf("%d ", threaded_arr[i]);
    printf("\n\n");

}

int main()
{
    scanf("%d",&n);
    runsorts(n);
}