// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 27.04.2019
// Implementation file of the buffer abstract data type

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include "buffer.h"

static buffer_t* buffer; // Buffer
static int bufferSize; // Buffer size that initialized from pCp arguments
static int bufferCurrentSize = 0; // Current size
static pthread_mutex_t  bufferLock = PTHREAD_MUTEX_INITIALIZER; // Buffer mutex lock
static int bufferInputIndex = 0; // Where to add
static int bufferOutputIndex = 0; // Where to get
static int isDone = 0;
static pthread_cond_t items = PTHREAD_COND_INITIALIZER; // Condition variable for the filled slots of the buffer
static pthread_cond_t emptySlots = PTHREAD_COND_INITIALIZER; // Condition variable for the empty slots of the buffer

// Removes an item from the buffer and put it into *item_ptr
int getItem(buffer_t* item_ptr) {
	int error;
	if ((error = pthread_mutex_lock(&bufferLock))) 
		return error;
	// While producer is still working and buffer is empty, the thread waits
	while ((bufferCurrentSize <= 0) && !error && !isDone)
		error = pthread_cond_wait(&items, &bufferLock);
	if (error) {
		pthread_mutex_unlock(&bufferLock);
		return error;
	}
	// If buffer is empty after the producer is finished, cancel the operation
	if (isDone && (bufferCurrentSize <= 0)) {
		pthread_mutex_unlock(&bufferLock); 
		return ECANCELED;
	}
	// Perform the get operation, set output index and decrease the current size
	*item_ptr = buffer[bufferOutputIndex];
	bufferOutputIndex = (bufferOutputIndex + 1) % bufferSize;
	bufferCurrentSize--;
	if ((error = pthread_cond_signal(&emptySlots))) {
		pthread_mutex_unlock(&bufferLock);
		return error;
	}
	return pthread_mutex_unlock(&bufferLock);
}

// Inserts item into the buffer
int putItem(buffer_t item) {
	int error;
	if ((error = pthread_mutex_lock(&bufferLock)))
		return error;
	// While producer is still working and buffer is empty, the thread waits
	while ((bufferCurrentSize >= bufferSize) && !error && !isDone)
		error = pthread_cond_wait (&emptySlots, &bufferLock);
	if (error) {
		pthread_mutex_unlock(&bufferLock);
		return error;
	}  
	// If consumers is finished, don't put any item to the buffer.
	if (isDone) {
		pthread_mutex_unlock(&bufferLock); 
		return ECANCELED; 
	} 
	// Perform the set operation, set input index and increase the current size
	buffer[bufferInputIndex] = item;
	bufferInputIndex = (bufferInputIndex + 1) % bufferSize;
	bufferCurrentSize++;
	if ((error = pthread_cond_signal(&items))) {
		pthread_mutex_unlock(&bufferLock); 
		return error; 
	}
	return pthread_mutex_unlock(&bufferLock);
}

// Get isDone flag value
int getDoneFlag(int* flag) {
	int error;
	if ((error = pthread_mutex_lock(&bufferLock)))
		return error;
	*flag = isDone;
	return pthread_mutex_unlock(&bufferLock);
}

// Set isDone and inform all waiting threads
int setDoneFlag(void) {
	int error1;
	int error2;
	int error3;
	if ((error1 = pthread_mutex_lock(&bufferLock)))
		return error1;
	isDone = 1;
	// Wake up everyone
	error1 = pthread_cond_broadcast(&items);             
	error2 = pthread_cond_broadcast(&emptySlots);
	error3 = pthread_mutex_unlock(&bufferLock);
	// Error check
	if (error1)
		return error1;
	else if (error2)
		return error2;
	else if (error3)
		return error3;
	else 
		return 0;
}


// Constructor for buffer
void initialiseBuffer(int size) {
	bufferSize = size;
	buffer = malloc(sizeof(buffer_t) * bufferSize);
}

// Destructor to prevent memory leaks
void freeBuffer() {
	free(buffer);
}