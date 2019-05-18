// CSE344 - System Programming HW6, 141044013, Fatih KOÇ, 27.04.2019
// Implementation file of the producer thread functions

#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include "buffer.h"
#include "extensions.h"

// Producing a buffer_t struct object for each file to copy 
static void* producer(void* pArgs) {
	buffer_t item;
	int error;
	int isDone = 0;
	struct stat fileStat;
    DIR *pDir;
    struct dirent *pDirEnt;
    char filePath[PATH_MAX * 2];
	struct producerArgs* pArgsForSubdirectory;
    char subDirectoryPath[PATH_MAX * 2];

	if (lstat(((producerArgs*) pArgs)->sourceDirectory, &fileStat) < 0) {
		fprintf(stderr, "lstat() error at producer for [%s]! %s\n", ((producerArgs*) pArgs)->sourceDirectory, strerror(errno));
		setError((error = -1));
		return NULL;
	}
	if (S_ISDIR(fileStat.st_mode)) {
		// Open the directory
		if ((pDir = opendir(((producerArgs*) pArgs)->sourceDirectory)) == NULL) {
			fprintf(stderr, "Cannot open directory '%s'! %s\n", ((producerArgs*) pArgs)->sourceDirectory, strerror(errno));
			setError((error = -1));
			return NULL;;
		}
		// Create target directory if it is not existed
		if (mkdir(((producerArgs*) pArgs)->targetDirectory, 0777) && errno != EEXIST){
			fprintf(stderr, "mkdir error for [%s]! %s\n", ((producerArgs*) pArgs)->targetDirectory, strerror(errno));
			setError((error = -1));
			return NULL;
		}
		
		while (!isDone) {
			// Iterate through all files in it
			if ((pDirEnt = readdir(pDir)) == NULL)
				break;
			// [.] and [..] directories avoided to stay in the given path
			if ((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0))
	            continue;
	        // File type check
			sprintf(filePath, "%s/%s", ((producerArgs*) pArgs)->sourceDirectory, pDirEnt->d_name);
			if (lstat(filePath, &fileStat) < 0) {
				fprintf(stderr, "lstat() error at producer for [%s]! %s\n", filePath, strerror(errno));
				setError((error = -1));
				break;
			}
			// If it is a subdirectory
			if (S_ISDIR(fileStat.st_mode)) {
				// Create new argument array for subdirectory
				pArgsForSubdirectory = malloc(sizeof(producerArgs));
				sprintf(subDirectoryPath, "%s/%s", ((producerArgs*) pArgs)->sourceDirectory, pDirEnt->d_name);
				strcpy(pArgsForSubdirectory->sourceDirectory, subDirectoryPath);
				sprintf(subDirectoryPath, "%s/%s", ((producerArgs*) pArgs)->targetDirectory, pDirEnt->d_name);
				strcpy(pArgsForSubdirectory->targetDirectory, subDirectoryPath);
				// Recursive call
				producer(pArgsForSubdirectory);
				free(pArgsForSubdirectory);
			}
			else {
		        // filePath is updated for every file in the directory
				sprintf(filePath, "%s/%s", ((producerArgs*) pArgs)->sourceDirectory, pDirEnt->d_name);
		        // Open the source file in the source directory in read mode
		        if ((item.sourceFileDescriptor = open(filePath, O_RDONLY, S_IRUSR)) == -1) {
					fprintf(stderr, "Cannot open file '%s' to read! %s\n", filePath, strerror(errno));
		        	setError((error = -1));
					break;
	        	}
		        // filePath is updated for every file in the directory
				sprintf(filePath, "%s/%s", ((producerArgs*) pArgs)->targetDirectory, pDirEnt->d_name);
		        // Open the target file in the target directory in write mode
		        if ((item.targetFileDescriptor = open(filePath, O_WRONLY | O_TRUNC | O_CREAT, S_IWUSR)) == -1) {
					fprintf(stderr, "Cannot open file '%s' to write! %s\n", filePath, strerror(errno));
		        	setError((error = -1));
		        	break;
		        }
		        strcpy(item.fileName, pDirEnt->d_name);
		        fprintf(stdout, "%ld thread produced '%s' to copy from %d to %d.\n", pthread_self(), item.fileName, item.sourceFileDescriptor, item.targetFileDescriptor);
		        // Add file information to the buffer
				if ((error = putItem(item))) 
				 	break;
			}
			if ((error = getDoneFlag(&isDone))) 
		 			break;
		}
		closedir (pDir);
		
	}
	if (error != ECANCELED)
		setError(error);

	return NULL;
}

// Constructor for a thread
int initialiseProducerThread(pthread_t* producerThread_ptr, producerArgs* arguments) {
	#ifdef DEBUG
	fprintf(stdout, "%ld producer initialized:%s->%s.\n", pthread_self(), arguments->sourceDirectory, arguments->targetDirectory);
	#endif
	int error;
	error = pthread_create(producerThread_ptr, NULL, producer, arguments);
	return (setError(error)); 
}

// Destructor to prevent memory leaks
int joinProducerThread(pthread_t* producerThread_ptr, producerArgs* arguments) {
	#ifdef DEBUG
	fprintf(stdout, "%ld producer joined:%s->%s.\n", pthread_self(), arguments->sourceDirectory, arguments->targetDirectory);
	#endif
	int error;
	error = pthread_join(*producerThread_ptr, NULL);
	free(arguments);
	return (setError(error)); 
}