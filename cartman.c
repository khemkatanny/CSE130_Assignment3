/*********************************************************************
 *
* Copyright (C) 2020-2022 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

//Sources:
//https://man7.org/linux/man-pages/man3/pthread_detach.3.html
//https://stackoverflow.com/questions/13664671/floating-point-exception-core-dump/13664831
//https://man7.org/linux/man-pages/man3/sem_wait.3.html
//https://www.geeksforgeeks.org/use-posix-semaphores-c/
//https://www.geeksforgeeks.org/concurrency-in-operating-system/
//https://www.geeksforgeeks.org/introduction-of-deadlock-in-operating-system/

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include "cartman.h"

#define NUMBER_OF_JUNCS 17
int total_tracks = 0;
sem_t sem[NUMBER_OF_JUNCS];
pthread_t thread[100];

struct cart {
  unsigned int cart;
  enum track track;
  enum junction junction;
} cart;

//helper function
void *arrival_junc(void *arg)
{
  struct cart *to_cross = (struct cart *) arg;
  int cart = to_cross->cart;
  //printf("cart in use: %d\n", cart);
  int track = to_cross->track;
  //printf("track in use: %d\n", track);
  int junction = to_cross->junction;
  //printf("junction in use: %d\n", junction);

  int next_junction;
  if(track == junction)  //clockwise
  {
    next_junction = (junction + 1) % total_tracks;
  }
  else   //anticlockwise
  {
    next_junction = (junction + total_tracks - 1) % total_tracks;
  }
  
  if(track == junction)  //if clockwise
  {
    if(track % 2 == 0)   //if even J1->J2
    {
      //after finding the next junction, provide exclusive access
      //holding lock - if junction 1 is taken, we need to wait
      sem_wait(&sem[junction]);

      //holding lock - if the junction 2 is taken, we need to wait
      sem_wait(&sem[next_junction]);

      //reserving junction 1 for cart to move
      reserve(cart, junction);
      printf("Reserving junction: %d\n", junction);

      //reserve junction 2 for cart to move
      reserve(cart, next_junction);
      printf("Reserving next junction: %d\n", next_junction);
      printf("track is: %d\n", track);
      
      //once we have access we cross
      cross(cart, track, junction);
      printf("Cross completed between: %d %d %d %d\n", cart, track, junction, next_junction);

      //after crossing we need to release junction 1
      release(cart, junction);
      printf("Releasing junction: %d\n", junction);
      
      //after crossing we need to release junction 2
      release(cart, next_junction);
      printf("Releasing next junction: %d\n", next_junction);

      //releasing lock
      sem_post(&sem[junction]);

      //releasing lock
      sem_post(&sem[next_junction]);
    }
    else   //if odd J2->J1
    {
      //after finding the next junction, provide exclusive access
      //holding lock - if junction 1 is taken, we need to wait
      sem_wait(&sem[junction]);

      //holding lock - if junction 2 is taken, we need to wait
      sem_wait(&sem[next_junction]);

      //reserving junction 1 for cart to move
      reserve(cart, junction);

      //reserving junction 2 for cart to move
      reserve(cart, next_junction);

      //once we have access we cross
      cross(cart, track, junction);

      //after crossing we need to release junction 1
      release(cart, junction);
      
      //after crossing we need to release junction 2
      release(cart, next_junction);

      //releasing lock
      sem_post(&sem[junction]);

      //releasing lock
      sem_post(&sem[next_junction]);
    }
  }
  else   //if anticlockwise
  {
    if(track % 2 == 0)   //if even J1->J2
    {
      //after finding the next junction, provide exclusive access
      //holding lock - if junction 1 is taken, we need to wait
      sem_wait(&sem[next_junction]);

      //holding lock - if the junction 2 is taken, we need to wait
      sem_wait(&sem[junction]);

      //reserving junction 1 for cart to move
      reserve(cart, next_junction);

      //reserve junction 2 for cart to move
      reserve(cart, junction);
      
      //once we have access we cross
      cross(cart, track, junction);

      //after crossing we need to release junction 1
      release(cart, next_junction);
      
      //after crossing we need to release junction 2
      release(cart, junction);

      //releasing lock
      sem_post(&sem[next_junction]);

      //releasing lock
      sem_post(&sem[junction]);
    }
    else  //if odd J2->J1
    {
      //after finding the next junction, provide exclusive access
      //holding lock - if junction 1 is taken, we need to wait
      sem_wait(&sem[next_junction]);

      //holding lock - if junction 2 is taken, we need to wait
      sem_wait(&sem[junction]);

      //reserving junction 1 for cart to move
      reserve(cart, next_junction);

      //reserving junction 2 for cart to move
      reserve(cart, junction);

      //once we have access we cross
      cross(cart, track, junction);

      //after crossing we need to release junction 1
      release(cart, next_junction);
      
      //after crossing we need to release junction 2
      release(cart, junction);

      //releasing lock
      sem_post(&sem[next_junction]);

      //releasing lock
      sem_post(&sem[junction]);
    }
  }
  free(to_cross);
  pthread_exit(NULL);
}


/*
 * Callback when CART on TRACK arrives at JUNCTION in preparation for
 * crossing a critical section of track.
 */
void arrive(unsigned int cart, enum track track, enum junction junction) 
{
  //creating a thread for the cart
  //pthread_t thread;
  struct cart *to_cross = malloc(sizeof(struct cart));
  to_cross->cart = cart;
  to_cross->track = track;
  to_cross->junction = junction;
  pthread_create(&thread[cart], NULL, arrival_junc, (void*) to_cross);
  pthread_detach(thread[cart]);
}

/*
 * Initialise the CART Manager for TRACKS tracks.
 *
 * Some implementations will do nothing, most will initialise necessary
 * concurrency primitives.
 */
void cartman(unsigned int tracks) 
{
  for(int i = 0; i < NUMBER_OF_JUNCS; i++)
  {
    sem_init(&sem[i], 0, 1);
  }
  total_tracks = tracks;
}
