/*********************************************************************************
Rachel Ross
CPSC380
04.01.20

Description of Program:
 * The objective of this assignment is to use semaphores to protect the critical section between two competing threads

Sources Used:
 * https://stackoverflow.com/questions/3024197/what-does-int-argc-char-argv-mean
 * I also used code from the slides from Blackboard
*********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

int main(int argc, char *argv)
{
  const int SIZE = 4096;

  int shm_fd;
  void *ptr;
  int i;
  void *thread_function(void *arg);
  sem_t bin_sem;

  int res;
  pthread_t producer_thread;
  pthread_t consumer_thread;
  void *thread_result;

/*************** PRODUCER ***************/
/* create the shared memory segment */
	shm_fd = shm_open(producer_thread, O_CREAT | O_RDWR, 0666);

	/* configure the size of the shared memory segment */
	ftruncate(shm_fd,SIZE);

	/* now map the shared memory segment in the address space of the process */
	ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}

	/**
	 * Now write to the shared memory region.
	 * Note we must increment the value of ptr after each write.
	 */

	sprintf(ptr,"%s",message0);
	ptr += strlen(message0);
	sprintf(ptr,"%s",message1);
	ptr += strlen(message1);
	sprintf(ptr,"%s",message2);
	ptr += strlen(message2);

/******************************************************************************/
/*************** CONSUMER ***************/

  	/* open the shared memory segment */
  	shm_fd = shm_open(consumer_thread, O_RDONLY, 0666);
  	if (shm_fd == -1) {
  		printf("shared memory failed\n");
  		exit(-1);
  	}

  	/* now map the shared memory segment in the address space of the process */
  	ptr = mmap(0,SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
  	if (ptr == MAP_FAILED) {
  		printf("Map failed\n");
  		exit(-1);
  	}

  	/* now read from the shared memory region */
  	printf("%s",ptr);

  	/* remove the shared memory segment */
  	if (shm_unlink(name) == -1) {
  		printf("Error removing %s\n",consumer_thread);
  		exit(-1);
  	}

/********************************************************************************/
  //initialization of semaphore & mutex
  //threads with synchronization

  /*************** MUTEX ***************/
  res = pthread_mutex_init(&work_mutex, NULL);
    if (res != 0) {
        perror("Mutex initialization failed");
        exit(EXIT_FAILURE);
    }
    res = pthread_create(&producer_thread, NULL, thread_function, NULL);
    if (res != 0) {
        perror("Thread creation failed");
        exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(&work_mutex);
    printf("Input some text. Enter 'end' to finish\n");
    while(!time_to_exit) {
        fgets(work_area, WORK_SIZE, stdin);
        pthread_mutex_unlock(&work_mutex);
        while(1) {
            pthread_mutex_lock(&work_mutex);
            if (work_area[0] != '\0') {
                pthread_mutex_unlock(&work_mutex);
                sleep(1);
            }
            else {
                break;
            }
        }
    }
    pthread_mutex_unlock(&work_mutex);
    printf("\nWaiting for thread to finish...\n");
    res = pthread_join(producer_thread, &thread_result);
    if (res != 0) {
        perror("Thread join failed");
        exit(EXIT_FAILURE);
    }
    printf("Thread joined\n");
    pthread_mutex_destroy(&work_mutex);
    exit(EXIT_SUCCESS);
}

void *thread_function(void *arg) {
    sleep(1);
    pthread_mutex_lock(&work_mutex);
    while(strncmp("end", work_area, 3) != 0) {
        printf("You input %d characters\n", strlen(work_area) -1);
        work_area[0] = '\0';
        pthread_mutex_unlock(&work_mutex);
        sleep(1);
        pthread_mutex_lock(&work_mutex);
        while (work_area[0] == '\0' ) {
            pthread_mutex_unlock(&work_mutex);
            sleep(1);
            pthread_mutex_lock(&work_mutex);
        }
    }
    time_to_exit = 1;
    work_area[0] = '\0';
    pthread_mutex_unlock(&work_mutex);
    pthread_exit(0);
}

/***********************************************************************/

  /*************** SEMAPHORE ***************

  res = sem_init(&bin_sem, 0, 0);
  if (res != 0) {
      perror("Semaphore initialization failed");
      exit(EXIT_FAILURE);
  }

  res = pthread_create(&producer_thread, NULL, thread_function, NULL);
  if (res != 0) {
      perror("Thread creation failed");
      exit(EXIT_FAILURE);
  }

  printf("Input some text. Enter 'end' to finish\n");

  while(strncmp("end", work_area, 3) != 0) {
      fgets(work_area, WORK_SIZE, stdin);
      sem_post(&bin_sem);
  }

  printf("\nWaiting for thread to finish...\n");
  res = pthread_join(producer_thread, &thread_result);
  if (res != 0) {
      perror("Thread join failed");
      exit(EXIT_FAILURE);
  }

  printf("Thread joined\n");
  sem_destroy(&bin_sem);
  exit(EXIT_SUCCESS);

  }

  void *thread_function(void *arg)
  {

  sem_wait(&bin_sem);
  while(strncmp("end", work_area, 3) != 0) {
      printf("You input %d characters\n", strlen(work_area) -1);
      sem_wait(&bin_sem);
  }

  pthread_exit(NULL);

	return 0;

}
