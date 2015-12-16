#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "utilities.h"

#define MAX_LEN (1000)

int j;

int main(int argc, char* argv[]){

	printf("in publisher\n");
	char buffer[MAX_LEN];

	//these are the values of command_handler[] passed to publisher as args
	int readPipe = argv[2][0];
	int writePipe = argv[1][0];
	int numTopics = argv[3][0];

	pid_t pubid = getpid();	//publisher id

	sprintf(buffer, "pub %d connect", pubid );		//writing first message
	if(write(writePipe, buffer, MAX_LEN) == -1){
		perror("error writing\n");
	}

	read(readPipe, buffer, MAX_LEN);
	if(strcmp("accept", buffer) != 0){
		printf("Did not accept pub\n");
		printf("accept does not equal%s\n", buffer );
		exit(1);
	}
	printf("%s\n", buffer );

	//loop for topics
	for(j = 0; j < numTopics; ++j){
		sprintf(buffer, "Topic Store Test");
		if(write(writePipe, buffer, MAX_LEN) == -1){
			perror("error writing topics");
		}
		read(readPipe, buffer, MAX_LEN);
		if(strcmp("accept topic", buffer) != 0){
			printf("Did not accept\n");
			exit(1);
		}
		printf("%s\n", buffer);
	}

	//end child process
	sprintf(buffer, "end");
	write(writePipe, buffer, MAX_LEN);
	read(readPipe, buffer, MAX_LEN); 
	if(strcmp(buffer, "end") != 0){
		printf("error ending\n");
	}
	printf("%s\n", buffer);

	//terminate
	sprintf(buffer, "terminate");
	write(writePipe, buffer, MAX_LEN);
	
	read(readPipe, buffer, MAX_LEN);
	printf("%s\n", buffer );

	close(readPipe);
	close(writePipe);	
	return 0;
}