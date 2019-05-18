// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 27.04.2019
// Implementation file of the consumer thread functions

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include "buffer.h"
#include "extensions.h"


// Producing a buffer_t struct object for a file (or a subdirectory) to copy
static void* consumer(void* cArgs) {                    
	int error;
	buffer_t item;
	struct stat fileStat;
    DIR *pDir;
    struct dirent *pDirEnt;
	char absolutePath[PATH_MAX * 2];

	while (1)  {
		if ((error = getItem(&item)))
			break;
		if ((error = copyFile(&item, ((consumerArgs*) cArgs)->targetDirectory))) 
	 		break;
		fprintf(stdout, "%ld thread consumed '%s' to copy from %d to %d.\n", pthread_self(), item.fileName, item.sourceFileDescriptor, item.targetFileDescriptor);
	}
	if (error != ECANCELED)
	setError(error);

	return NULL;
}

// Constructor for a thread
int initialiseConsumerThread(pthread_t* consumerThread_ptr, consumerArgs *arguments) {
	#ifdef DEBUG
	fprintf(stdout, "%ld consumer initialized:%s.\n", pthread_self(), arguments->targetDirectory);
	#endif
	int error;
	error = pthread_create(consumerThread_ptr, NULL, consumer, arguments);
	return (setError(error));
}

// Destructor to prevent memory leaks
int joinConsumerThread(pthread_t* consumerThread_ptr, consumerArgs *arguments) {
	#ifdef DEBUG
	fprintf(stdout, "%ld consumer joined:%s.\n", pthread_self(), arguments->targetDirectory);
	#endif
	int error;
	error = pthread_join(*consumerThread_ptr, NULL);
	free(arguments);
	return (setError(error));
}