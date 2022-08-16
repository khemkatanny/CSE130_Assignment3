/*********************************************************************
 *
* Copyright (C) 2020-2022 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

//Sources:
//https://man7.org/linux/man-pages/man3/sem_destroy.3.html
//http://man7.org/linux/man-pages/man7/sem_overview.7.html 

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "manpage.h"

#define THREADS_FOR_PARA 7

//creating 7 threads
pthread_t threads[THREADS_FOR_PARA];
//declaring a semaphore
sem_t sem[THREADS_FOR_PARA];

void *thread_func(void *arg)
{
  int pid = getParagraphId();  //gives us which paragraph is picked up

  if(pid > 0)
  {
    sem_wait(&sem[pid - 1]); //locking the semaphore if needed (that is why we do -1 here)
  }

  showParagraph();  //prints the paragraph picked up by getParagraphID

  if(pid < THREADS_FOR_PARA - 1)
  {
    sem_post(&sem[pid]); //after its done it releases the signal
  }

  //we now destroy the semaphore
  sem_destroy(&sem[pid]);

  pthread_exit(NULL);
}

/*
 * Create a thread for each of seven manpage paragraphs and have them synchronize
 * their invocations of showParagraph() so the manual page displays as follows:
 *
 --------- 8< ---------------------------------------------------
 
A semaphore S is an unsigned-integer-valued variable.
Two operations are of primary interest:
 
P(S): If processes have been blocked waiting on this semaphore,
 wake one of them, else S <- S + 1.
 
V(S): If S > 0 then S <- S - 1, else suspend execution of the calling process.
 The calling process is said to be blocked on the semaphore S.
 
A semaphore S has the following properties:

1. P(S) and V(S) are atomic instructions. Specifically, no
 instructions can be interleaved between the test that S > 0 and the
 decrement of S or the suspension of the calling process.
 
2. A semaphore must be given an non-negative initial value.
 
3. The V(S) operation must wake one of the suspended processes. The
 definition does not specify which process will be awakened.

 --------- 8< ---------------------------------------------------
 *
 * As supplied, shows random single messages.
 */
void manpage() 
{
  //initializing the semaphore
  for(int i = 0; i < THREADS_FOR_PARA; i++)
  {
    sem_init(&sem[i], 0, 1);
    sem_wait(&sem[i]);
  }

  for(int i = 0; i < THREADS_FOR_PARA; i++)
  {
    pthread_create(&threads[i], NULL, thread_func, NULL);
  }

  //waiting for threads and joining
  for(int i = 0; i < THREADS_FOR_PARA; i++)
  {
    pthread_join(threads[i], NULL);
  }

  for(int i = 0; i < THREADS_FOR_PARA; i++)
  {
    sem_destroy(&sem[i]);
  }

  pthread_exit(NULL);
}
