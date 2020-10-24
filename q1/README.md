      
# OSN Assignment 4

# **Concurrency**

#### Pooja Desur
### 2019101112

## Question 1

### Concurrent Merge-Sort
Merge sort can be ran utilizing processes, by allowing each proccess to sort an array segment. For array segments of size < 5 , selection sort was implemented. 
MergeSort using multi threading was also done.

##### Concurrent Multiprocess Mergesort
The left and right half of the original array is given to two separate processes. A new left and right process is created using fork() system call. Both children are waited for using waitpid until they return back after sorting. The two parts are then merged using the regular merge function of a merge sort.

##### Concurrent Multi threaded Mergesort
The array is sorted by using 8 threads, giving each thread 1/8th of the array each to sort. These parts are then merged after joining all threads.


The running times were compared for both on sizes ranging from **n = 1** to **n = 50000** (each is an average of 15 runs)

##### For n = 1:
- Normal Merge sort took on average **0.000028** seconds
- Concurrent Multiprocess Merge sort took on average **0.000030** seconds
- Concurrent Multi threaded Mergesort took on average **0.001000** seconds

##### For n = 10:
- Normal Merge sort took on avergae **0.000032** seconds
- Concurrent Multiprocess Merge sort took on average **0.000439** seconds
- Concurrent Multi threaded Mergesort took on average **0.000950** seconds

##### For n = 100:
- Normal Merge sort took on average **0.000081** seconds
- Concurrent Multiprocess Merge sort took on average **0.000356** seconds
- Concurrent Multi threaded Mergesort took on average **0.000840** seconds

##### For n = 1000:
- Normal Merge sort took on average **0.000589** seconds
- Concurrent Multiprocess Merge sort took on average **0.000354** seconds
- Concurrent Multi threaded Mergesort took on average **0.000320** seconds

##### For n = 10000:
- Normal Merge sort took on average **0.003825** seconds
- Concurrent Multiprocess Merge sort took on average **0.000358** seconds
- Concurrent Multi threaded Mergesort took on average **0.001860** seconds

##### For n = 50000:
- Normal Merge sort took on average **0.014772** seconds
- Concurrent Multiprocess Merge sort took on average **0.000553** seconds
- Concurrent Multi threaded Mergesort took on average **0.009600** seconds

---
##### Multiprocess
It is observed that after **n = 1000** , the concurrent merge sort started taking less time than the normal merge sort.
 Below 1000 the **time for forking and creating processes became a limiting factor for the concurrent merge sort**. But as n increased, the **speed provided by concurrency (many processes working simultaneously) from the concurrent mergesort** made it more effecient and sorting took place faster. 


##### Multi threaded
The unusually high times for 1 sized array in both multi threaded sorts is due to the large over head of creating a thread. 
It gets faster till 1000 sized array as concurrent threads are working together on the array. But after that , since each thread is running normal mergesort the time again increases. 
The recursive multi threaded sort increases after n = 1000 because after a certain point , the large number of threads created stop working in parallel(due to limited number of cpu cores). So the process has a lot of threads , and is therefore a very heavy process using up all the cores which becomes very slow(atleast on the laptop tested)
(Please note the times measured of multi threaded part are very machine specific)

#### Files 
- q1.c
- README.md








