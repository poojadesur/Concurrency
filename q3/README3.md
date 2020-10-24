      
# OSN Assignment 4

# **Concurrency**

#### Pooja Desur
### 2019101112

## Question 3

There are k musicians, which include singers and instrument players. There are a acoustic stages and e electric stages. All musicians can play on all stages except violinists who can only play on acoustic stages and basists who can only play on electric stages.

Each musician who can play on both stages is represented by two threads, one being sent to the acoustic() function and one to the electric() function. 

A violinist has only one thread created which is sent to acoustic(). Similarly a basist has only one thread created which is sent to electric().

Singers have only one thread created, which are sent to singer().

#### Semaphores
There are 4 semaphores used which keep track of available stages for each performer, and the coordinators that will hand out t-shirts.

- ac semaphore represents the number of acoustic stages available to play at, and is used only by musicians (not singers) who can play at an acoustic stage
- el semaphore represents the number of electric stages available to play at, and is used only by musicians (not singers) who can play at an electric stage
- doubleacel semaphore keeps track of the stages available for singers. They can either perform solo, of join any currently playing musicians performance.
- coord keeps track of the number of coordinators who are handing out t-shirts to performers who have finished performances. Singers and musicians can recieve shirts.

#### Stage IDs (Bonus)
If there are a acoustic stages and e electric stages, the first a stages are given ids, followed by the next e stages.
i.e stages with id 1 to a are acoustic stages, while stages with id a+1 to a+e are electric stages.

#### Arriving at Srujana

When a musician/singer arrives at srujana, they wait for an available stage that they can play at. (timed wait on ac,el, or doubleacel semaphores).
If they become impatient after t seconds they leave, this is implemented by sem_timedwait(). 

#### Musicians (non singers)

The values for each musician is saved in a global array of structs (performer_struct). 

- If their state is 0, they have not yet arrived at Srujana. 
- state = 1 means they are waiting at srujana
- state = 2 means they have started to play on a stage

Stages information is stored in an array of structures (stage_struct). The state of a stage represents the number of people playing on the stage.
- state = 0 signifies stage is empty and available
- state = 1 signifies one musician is playing on it, and the performance can be joined by a singer
- state = 2 signfies one musician and one singer are performing, nobody else can use this stage now

If in the case of a violinist or a basist, if no acoustic or electric stage exists respectively, the performer will wait at Srujana until they become impatient and then leave.

Once a musician who is allowed to play at both acoustic and electric stages has found a free stage, (of either type), they will not play again at the other type. This is made sure by the 'clear' variable of the performer_struct structure. If it is set to 1, that means musician has already found a vacant stage of the other type and the respective semaphore is increased by one, and the thread then returns.

Once a musician has finished performing, he checks if a singer has joined them on stage, and if so, the performance is extended by another two seconds. After the duet is over, a signal using pthread_cond_signal() and pthread_cond_wait is sent to a singer who is part of a duet to finish their performance as well.

#### Singers
Singers can either perform solo, or join an ongoing musicians performance. However, they will not join another singers performance.
Singer information is saved in the same array of structures as musicians. The states represent the same information as musicians. 
For thee state of stages, it is simiilar to musicians, but one additional state exists : 
- state = 3 means singers is performing solo (which prevents another singer from joining them)

There is no preference for a singer to choose a solo performance or joining an ongoing musician performance.

If a stage becomes available (either one where musician is performing or an empty stage) before the singer becomes impatient, a for loop is used to run over all the stages, out of which one is obviously available. The first stage that has either 0 players on it or 1 player (musician is currently playing) will be selected for the singer. The stage's state is then set to the number of players on it.

If the singer performs solo, after their performance, the stage they performed on's state will be set back to 0, marking it now available.

If a singer has joined a performance, the thread will wait using pthread_cond_wait for the other performer ( in acoustic() or electric() ) to send a signal that their joint performance is now over.

#### Collecting shirts

Both singers (BONUS) and musicians can collect shirts from coordinators once they have finished performing. Those who left impatiently will not get tshirts.
This is implemented by a coord semaphore which each thread ( basically each player ) waits on in order to collect their tshirt. After they have collected the shirt, they post onto the same semaphore.














