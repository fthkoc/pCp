// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 29.04.2019
// Header file for the all extension methods
#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include "buffer.h"


// A struct to represent arguments for a producer thread
typedef struct producerArgs {
	char targetDirectory[PATH_MAX];
	char sourceDirectory[PATH_MAX];
} producerArgs;

// A struct to represent arguments for a consumer thread
typedef struct consumerArgs {
	char targetDirectory[PATH_MAX];
} consumerArgs;

// Get error flag value
int getError(int* error_ptr);
// Set error flag value
int setError(int error);
// Copies file(s) by the given information in the buffer_t to the target directory
int copyFile(buffer_t* item, char* targetDirectory);
// Print information about total operation
void printResult();

#endif