#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <pthread.h>
#include <assert.h>

#include "utilities.h"

#define MAX_LEN 1024


/*****************
This function creates a child and sets its attributes(pid,
pipes, proxyID, etc.). It then pipes a new process for this newly created child,
executes the correct file(pub/sub) and returns the child to the server to pushed onto 
the list of connection records.
********************/
ConnectionRecord spawn_Child(char* filename, int type, EntryList *topicList, int numTopics){

	ConnectionRecord newChild;
	newChild.connectionType = type;
	newChild.toChild = -1;	//going to instantiate new pipes to assign to these
	newChild.fromChild = -1;
	newChild.numTopics = numTopics;
	newChild.topicLists = topicList;

	pid_t pid;
	int toChildPipe[2];		//create pipes to assign to newChild
	int fromChildPipe[2];

	char* exec_handler[] = {filename, NULL, NULL, NULL};
	exec_handler[1] = malloc(2*sizeof(int));
	exec_handler[2] = malloc(2*sizeof(int));

	if(pipe(toChildPipe)<0 || pipe(fromChildPipe) < 0 ){
		printf("error creating child pipes\n");
		exit(1);
	}

	if((pid = fork()) < 0){
		perror("Error forking\n");
		exit(1);
	}
	else if( pid > 0){		//parent
		newChild.pid = pid;

		newChild.toChild = toChildPipe[1];
		newChild.fromChild = fromChildPipe[0];

		newChild.open = true; 	//keeps track of whether or not FD's have been closed
	}
	else{	//child

		close(fromChildPipe[0]);
		close(toChildPipe[1]);
		//set parameters for execution(newly created open pipes)
		exec_handler[1][0] = fromChildPipe[1];
		exec_handler[1][1] = '\0';
		exec_handler[2][0] = toChildPipe[0];
		exec_handler[2][1] = '\0';
		//execute()
		if(execvp(exec_handler[0], exec_handler) < 0){
			perror("Error executing file\n");
		}

		//we don't want child to return, parent continues to fork
		exit(1);
	}

	free(exec_handler[1]);
	free(exec_handler[2]);
	return newChild;
}



int main(int argc, char *argv[]){
	
	int i,n,m,topics;
	
	//placeholder for connections
	int connectionIndex = 0;


	printf("Enter number of publishers: ");
	scanf("%d", &n);
	printf("Enter number of subscribers: ");
	scanf("%d", &m);
	printf("Enter number of topics: ");
	scanf("%d", &topics);

	//array of connection records
	ConnectionRecord *connections = malloc((n+m) * sizeof(ConnectionRecord));

	pthread_t *threads = malloc((n+m)*sizeof(pthread_t));


	//initialize fifo for each topic
	EntryList *topicList = malloc((topics) * sizeof(EntryList));
	for(i = 0; i < topics; i++){
		topicList[i] = createEntryList(i);
	}

	//spawn a child for each pub/sub, execute appropriate file, add to connection list
	for(i=0; i < n; i++){
		printf("connection index: %d\n", connectionIndex);
		connections[connectionIndex] = spawn_Child("./publisher", 0, topicList, topics);
		connectionIndex++;
	}
	for(i=0; i < m; i++){
		printf("connection index: %d\n", connectionIndex);
		connections[connectionIndex] = spawn_Child("./subscriber", 1, topicList, topics);
		connectionIndex++;
	}

	//loop through list of connections, find all conns with open FD's
	for(i=0; i < connectionIndex; i++){
		
		if(connections[i].open == true){
			if(connections[i].connectionType == 0){
				int threadSuccess = pthread_create(&threads[i], NULL, pubServer, &(connections[i]));
				assert(threadSuccess == 0);
			}

			else if(connections[i].connectionType == 1){
				int threadSuccess = pthread_create(&threads[i], NULL, subServer, &(connections[i]));
				assert(threadSuccess == 0);
			}
			else{

				printf("Connection type not configured for child");
				exit(1);
			}
			connections[i].open = false;
		}
	}

	//give subscribers time to get their messages
	sleep(1); 

	//loop through connections to join all threads
	for(i=0; i < connectionIndex; i++){
		int joinResult = pthread_join(threads[i], NULL);
		assert(joinResult == 0);
	}	

	free(threads);
	free(connections);
	
	return 0;
}