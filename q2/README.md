      
# OSN Assignment 4

# **Concurrency**

#### Pooja Desur
### 2019101112

## Question 2

Each of the n Pharmaceutical Companies, m Vaccination Zones and o Students that wait to receive a vaccine, have been represented by separate threads that are sent to functions do_company(), do_zone(), and do_student() respectively.

Each company makes a certain random amount of batches ( set using global variables minbatches and maxbatches ) that take a random amount of time to prepare ( set using global variables minpreptime and maxpreptime ) which contain a random number of vaccines ( set using global variables minstudents and maxstudents ).

- The number of students in the simulation left to be vaccinated ( whether they have been vaccinated before or not ) is kept track by global variable student_track.
- The number of students currently at the gate, waiting to be vaccinated is kept track of by global variable student_gate

Each vaccination has a probability p of working, and a student can be vaccinated upto three times before he has to be sent home, if they test negative all three times.

#### Company

While there are students left in the simulation to be vaccinated, a company will make a batch of vaccines, and only restart production if all its batches are over.
Values for a certain company are kept track of by an array of structures called 'company'.
- company structure has a value setprod which when set to 1, signals production can resume
- companybatch is a global array that keeps track of the number of batches the company made that have not been used up by a zone yet

#### Zone
An array of structures (zone_struct) called zone, keeps track of the values for each zone.
While there are students left to be vaccinated - 
- each zone busy waits in the function zone_get_batch() until it finds a company that has finished preparing a batch of vaccines. It then takes one batch from that company. The company has essentially now delivered a batch to this zone.
- as long as there are some vaccines left, the zone allocates a certain number of slots ( = min(8, vaccines it has, students at the gate) ) and signals itself as ready to fill up those slots.
- If at a certain point, there are less students at the gate than the number of slots, the vaccination phase begins with some empty slots, ( by setting beginvaccines flag of that zone to 1) that cannot be filled up until the zone's vaccination phase is over.
When the vaccination phase begins, each student in this zone is signaled to be vaccinated due to this flag.
- a zone only allocates slots again once its initial number of slots have all been filled up and vaccinated. The reslot in zone structure signals when a zone can begin slotting again.
- a zone only gets a new batch of vaccines once all its current vaccines have finished
- if at any point, the number of students in simulation is zero, the zone thread returns.

#### Student
An array of structures (student_struct) called student, keeps track of the values for each student.
Each student has four states:
- has not arrived to college yet (-1)
- is waiting at the gate to be alloted a slot (1)
- is waiting at a zone after being allocated a slot for the vaccination phase to being (2)
- is ready to be tested for antibodies(3)
- is either allowed in college or has been sent home (4)

Once a student has reached state 3, the student thread returns, and student_track decrements by one.

- The number of vaccines a student has received is kept track of by vaccines_received in the student struct.
- the student thread busy waits as long as its state is not 4, until some state change or flag has been set
- When the vaccination phase at the zone a student has been alloted to starts, the beginvaccines flag of zone is set to one, and the student is now vaccinated.
- After vaccination, the student is tested for antibodies. The probability the vaccine worked is saved in probsuccess and is unique for each vaccine which is form a certain company. (vaccines from same company have same probability). If the student tests positive for antibodies, he is allowed in college, else he is sent back to the gate or sent home if tested negative thrice.
- After each student has been tested, this means their current vaccination process is over. This the number of vaccines, slots and phase (number of students that entered a vaccination phase when all the slots were not filled at a zone) can be incremented.
- This will signal to a zone that is safe to allocate a new number of slots again (reslot set to 1) or fill up the remaining unused slots(beginvaccines set to 0) ( the current vaccination round is over ), or/and that it has ran out of vaccines, or/and if the company the vaccines were from's batches have now all been used up, the company can be signalled for reproduction (set setprod of company to 1).













