# Producer-Consumer-Semaphore
Using semaphore to implement the Producer/Consumer problem, which states that: One or more producers are generating data and  placing these in a buffer. A single consumer is taking items out of the buffer one  at a time. Only one producer or consumer may access the buffer  at any one time. The problem lies in ensuring that the producer won’t try to add data into the buffer if its full,  and that the consumer won’t try to  remove data from an empty buffer.

You are required to write a C program to solve the following synchronization problem using POSIX and “semaphore.h” libraries.

N mCounter threads count independent incoming messages in a system and another thread mMonitor gets the count of threads at time intervals of size t1, and then resets the counter to 0. The mMonitor then places this value in a buffer of size b, and a mCollector thread reads the values from the buffer.

Any thread will have to wait if the counter is being locked by any other thread. Also, the mMonitor and mCollector threads will not be able to access the buffer at the same time or to add another entry if the buffer is full.

Assume that the messages come randomly to the system, this can be realized if the mCounter threads sleep for random times, and their activation (sleep time ends) corresponds to an email arrival. Similarly, the mMonitor and mCollector will be activated at random time intervals.


**Dividing the problem**

It becomes much easier if we divided up the problem into two sub-problems

- Problem 1

Threads included:
  - The N mCounter threads.
  - The mMonitor thread.

Shared resources:
An integer to count messages.

- Problem:
When a mCounter thread grants access to the counter, it should add one to it.
When the mMonitor thread grants access to the counter, it should reset it to 0 and save its value to use it later.
Only one thread should be able to access the shared counter.


- Problem 2

Threads included:
  - The mMonitor thread.
  - The mCollector thread.

- Shared resources:
A buffer that should be implemented using a FIFO queue.

- Problem:
It’s a bounded buffer producer/consumer problem. 
mMonitor is the producer. It enqueues the value that was saved from the
previous problem into the buffer.
mCollector is the consumer it takes the data out of the buffer.


**Program output**

The output shows the behavior of the threads. Each thread should print a certain output when a particular event happens:

- mCounter:
At time of activation (sleep time end): Counter thread %I%: received a message
Before waiting: Counter thread %I%: waiting to write
After increasing the counter: Counter thread %I%: now adding to counter, counter value=%COUNTER%

- mMonitor :
Before waiting to read the counter: Monitor thread: waiting to read counter
After reading the counter value: Monitor thread: reading a count value of %COUNTER%
After writing in the buffer: Monitor thread: writing to buffer at position %INDEX%
If the buffer is full: Monitor thread: Buffer full!!

- mCollector :
After reading from the buffer: Collector thread: reading from the buffer at position %INDEX%
If the buffer is empty: Collector thread: nothing is in the buffer!
