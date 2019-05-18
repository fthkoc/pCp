// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 29.04.2019
// Implementation file for the all extension methods

#include "extensions.h"

static int globalError = 0; 
static pthread_mutex_t errorLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t  operationLock = PTHREAD_MUTEX_INITIALIZER;
static int totalBytesCopied;
static int copiedFileCount;


// Get error flag value
int getError(int* error_ptr) {
   int tempError;   
   if ((tempError = pthread_mutex_lock(&errorLock)))
      return tempError; 
   *error_ptr = globalError;
   return pthread_mutex_unlock(&errorLock);
}

// Set error flag value
int setError(int error) {
   int tempError;
   if (!error)
      return error;
   if ((tempError = pthread_mutex_lock(&errorLock)))
      return tempError;   
   if (!globalError)
      globalError = error; 
   tempError = pthread_mutex_unlock(&errorLock);
   return tempError ? tempError : error;
}

// Copies file(s) by the given information in the buffer_t to the target directory
int copyFile(buffer_t* item, char* targetDirectory) {
	int error, byteRead;
	char buffer[1024];
	struct stat fileStat;
    char filePath[PATH_MAX];

	if ((error = pthread_mutex_lock(&operationLock)))
		return setError(error);
	// Copy files here
	while (byteRead = read(item->sourceFileDescriptor, &buffer, sizeof(buffer)) != 0) {
		write(item->targetFileDescriptor, &buffer, sizeof(buffer));
		totalBytesCopied += byteRead;
	}
	copiedFileCount++;
	#ifdef DEBUG
	fprintf(stdout, "%ld copied:%s, from %d to %d.\n", pthread_self(), item->fileName, item->sourceFileDescriptor, item->targetFileDescriptor);
	#endif
	close(item->sourceFileDescriptor);
	close(item->targetFileDescriptor);
	error = pthread_mutex_unlock(&operationLock);
	return setError(error); 
}

// Print information about total operation
void printResult() {
	fprintf(stdout, "Total bytes copied: %d.\n", totalBytesCopied);
	fprintf(stdout, "File copied: %d.\n", copiedFileCount);
}
