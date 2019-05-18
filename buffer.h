// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 27.04.2019
// Header file of the buffer abstract data type
#ifndef BUFFER_H
#define BUFFER_H

#include <limits.h>

// Data type to keep information about file copy
typedef struct {
	int sourceFileDescriptor;
	int targetFileDescriptor;
	char fileName[PATH_MAX];
} buffer_t;

// Constructor for buffer
void initialiseBuffer(int size);
// Destructor to prevent memory leaks
void freeBuffer();
// Removes an item from the buffer and put it into *item_ptr
int getItem(buffer_t* item_ptr);
// Inserts item into the buffer
int putItem(buffer_t item);
// Get isDone flag value
int getDoneFlag(int* flag);
// Set isDone and inform all waiting threads
int setDoneFlag(void);

#endif