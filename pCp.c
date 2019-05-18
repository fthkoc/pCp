// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 27.04.2019
// Main program of a directory copying utility
// In some parts I influenced from UNIX System Programming book, Chapter 16

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include "buffer.h"
#include "extensions.h"


int initialiseProducerThread(pthread_t* producerThread_ptr, producerArgs* arguments);
int joinProducerThread(pthread_t* producerThread_ptr, producerArgs* arguments);

int initialiseConsumerThread(pthread_t* consumerThread_ptr, consumerArgs* arguments);
int joinConsumerThread(pthread_t* consumerThread_ptr, consumerArgs* arguments);

unsigned long getTimeNow();


int main(int argc, char* argv[]) {
	struct stat argFileStat;
	pthread_t* producerThread;
	struct producerArgs* pArgs;
	pthread_t* consumerThreads;
	struct consumerArgs** cArgs;
	int totalConsumerThreadCount;
	int error;

	#ifdef DEBUG // File arguments detailed information
	for (int i = 3; i < 5; ++i) {
		lstat(argv[i], &argFileStat);
	    fprintf(stdout, "Information for file '%s'\n", argv[i]);
	    fprintf(stdout, "File Size: \t\t%ld bytes\n", argFileStat.st_size);
	    fprintf(stdout, "Number of Links: \t%ld\n", argFileStat.st_nlink);
	    fprintf(stdout, "File inode: \t\t%ld\n", argFileStat.st_ino);
	    fprintf(stdout, "File Permissions: \t");
	    fprintf(stdout, (S_ISDIR(argFileStat.st_mode)) ? "d" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IRUSR) ? "r" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IWUSR) ? "w" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IXUSR) ? "x" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IRGRP) ? "r" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IWGRP) ? "w" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IXGRP) ? "x" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IROTH) ? "r" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IWOTH) ? "w" : "-");
	    fprintf(stdout, (argFileStat.st_mode & S_IXOTH) ? "x" : "-");
	    fprintf(stdout, "\n\n");
	    fprintf(stdout, "The file %s a directory\n", (S_ISDIR(argFileStat.st_mode)) ? "is" : "is not");
	    fprintf(stdout, "---------------------------\n");
	}
	#endif
	// Argument validation
	switch (argc) {
		case 5:
			if (((lstat(argv[3], &argFileStat) < 0) || !(S_ISDIR(argFileStat.st_mode))) && ((lstat(argv[4], &argFileStat) < 0) || !(S_ISDIR(argFileStat.st_mode)))) {
				fprintf(stderr, "Wrong argument usage! Argument 4 and Argument 5 must be a valid directory path.\n");
				fprintf(stderr, "Usage -> ./pCp numberOfConsumers bufferSize sourceDirectoryPath destinationDirectoryPath\n");
				return -1;
			}
			break;
		default:
			fprintf(stderr, "Invalid number of arguments!\nUsage -> ./pCp numberOfConsumers bufferSize sourceDirectoryPath destinationDirectoryPath\n");
			fprintf(stderr, "Example Usage -> ./pCp 10 5 sourceDirectoryPath destinationDirectoryPath\n");
			return -1;
	}
	// Get starting time
    unsigned long startTime = getTimeNow();
	// Get number of the consumer threads from arguments
	totalConsumerThreadCount = atoi(argv[1]);
    // Initialise buffer with the given size
	initialiseBuffer(atoi(argv[2]));
	// Producer thread creation
	producerThread = (pthread_t *) malloc(sizeof(pthread_t));
	if (producerThread == NULL) {
		fprintf(stderr, "Failed to allocate space for producer. %s\n", strerror(error));
		return 1;
	}
	// Initialize producer arguments
	pArgs = malloc(sizeof(producerArgs));
	strcpy(pArgs->sourceDirectory, argv[3]);
	strcpy(pArgs->targetDirectory, argv[4]);
	// initialize producer threads
	if ((error = initialiseProducerThread(producerThread, pArgs))) {
		fprintf(stderr, "Failed to create producer:%s\n", strerror(error));
     	return 1;
  	}
	// Consumer thread creation
  	consumerThreads = (pthread_t *) calloc(totalConsumerThreadCount, sizeof(pthread_t));
	if (consumerThreads == NULL) {
		fprintf(stderr, "Failed to allocate space for consumers. %s\n", strerror(error));
	  	return 1;
	}
	cArgs = malloc(sizeof(consumerArgs*) * totalConsumerThreadCount);
	// initialize consumer threads
	for (int i = 0; i < totalConsumerThreadCount; i++) {
		// Initialize consumer arguments
		cArgs[i] = malloc(sizeof(consumerArgs));
		strcpy(cArgs[i]->targetDirectory, argv[4]);
	  	if ((error = initialiseConsumerThread(&consumerThreads[i], cArgs[i]))) {
	    	fprintf(stderr, "Failed to create consumer %d:%s\n", i, strerror(error));
	    	return 1;
	  	}
	}

	if ((error = joinProducerThread(producerThread, pArgs))) {
    	fprintf(stderr, "Failed producer join:%s\n", strerror(error));
    	return 1;
	}

	if ((error = setDoneFlag())) {
		fprintf(stderr, "Failed to set done indicator:%s\n", strerror(error));
		return 1;
	}

	for (int i = 0; i < totalConsumerThreadCount; i++) {
		if ((error = joinConsumerThread(&consumerThreads[i], cArgs[i]))) {
        	fprintf(stderr, "Failed consumer %d join:%s\n", i, strerror(error));
        	return 1;
		}
	}
	// Free all the allocated memory
	free(cArgs);
    free(producerThread);
    free(consumerThreads);
    freeBuffer();
    // Calculate working time
    unsigned long finishTime = getTimeNow();
	unsigned long difference = finishTime - startTime;
	fprintf(stdout, "Total time passed: %ld ms.\n", difference);
	// Print summary about operation
	printResult();

	return 0;
}

// Get current time
unsigned long getTimeNow() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    unsigned long result = tv.tv_usec;
    result /= 1000;
    result += (tv.tv_sec * 1000);
    return result;
}
