#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utilities.h"

#define MAX_LEN (1000)

int main(int argc, char* argv[]){

	printf("in subscriber\n");
	char buffer[MAX_LEN];

	//these are the values of command_handler[] passed to publisher as args
	int readPipe = argv[2][0];
	int writePipe = argv[1][0];
	int numTopics = argv[3][0];


	//int numTopics = argv[3];

	pid_t pubid = getpid();	//publisher id

	sprintf(buffer, "sub %d connect", pubid );		//writing first message
	if(write(writePipe, buffer, MAX_LEN) == -1){
		perror("error writing\n");
	}

	read(readPipe, buffer, MAX_LEN);
		if(strcmp("accept", buffer) != 0){
			printf("Did not accept sub\n");
			exit(1);
		}
	printf("%s\n", buffer );

	sprintf(buffer, "sub ready for topic");
	write(writePipe, buffer, MAX_LEN);

	for(int i =0; i < numTopics; i++){
		read(readPipe, buffer, MAX_LEN);
   		if(strcmp(buffer, "Topic Store Test") !=0 ){
   			printf("topic store test failed\n");
   			printf("does not want: %s\n", buffer);
   		}
   	printf("Successful: %s\n", buffer);
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