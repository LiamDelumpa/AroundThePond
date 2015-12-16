#include "utilities.h"

#define MAX_LEN (1000)

/***********************************************************
Functions to handle topic store
***************************************************************/
bool isLastEntry(size_t index){
	bool isLast = false;

	if((index + 1  % MAX_ENTRIES) == 0){
		isLast = true;
	}
	return isLast;
}
bool isEmpty(EntryList *entries){
	return entries->head == entries->tail;
}
bool isFull(EntryList *entries){

	bool full = false;
	//checks case where head = MaxEntries-1 and the tail is still 0. buffer full
	if(entries->tail == 0 && isLastEntry(entries->head)){
		full = true;
	}
	//checks case where head is exactly one spot behind tail. buffer full
	else if(entries->tail == entries->head + 1){
		full = true;
	}
	return full;
}

EntryList createEntryList(int topicID){

	EntryList entries;
	entries.topicID = topicID;
	entries.totalSubscribers = 0;
	entries.head = 0;
	entries.tail = 0;

	//initialize mutex
	int result = pthread_mutex_init(&(entries.mutex), NULL);
	assert(result == 0);

	return entries;
}

bool enqueue(EntryList *entries, char *data){

	bool success = false;
	int lockResult = pthread_mutex_lock(&(entries->mutex));
	assert(lockResult == 0);

	if(!isFull(entries)){

		TopicEntry *newEntry = malloc(sizeof *newEntry); 
		newEntry = &(entries->circularBuff[entries->head]);
		newEntry->timeStamp = time(NULL);
		

		size_t size = sizeof(newEntry->data);
		strncpy(newEntry->data, data, size);
		printf("Added %s to topic store\n", newEntry->data);

		entries->head++;
		success = true;

	}
	pthread_mutex_unlock(&(entries->mutex));

	return success;
}


bool dequeue(EntryList *entries){

	bool success = false;
	int lockResult = pthread_mutex_lock(&(entries->mutex));
	assert(lockResult == 0);


	if(!isEmpty(entries)){
		printf("current head: %zu\n", entries->head);
		printf("current tail: %zu\n", entries->tail);
		//printf("retrieved: %s\n", entries->circularBuff[entries->tail].data);

		TopicEntry *curr_entry = malloc(sizeof *curr_entry); 
		curr_entry = &(entries->circularBuff[entries->tail]);
		

		printf("retrieved entry: %s\n", curr_entry->data);

		curr_entry->readCount++;
		//entries->tail++;//not yet
		success = true;
	}
	
	pthread_mutex_unlock(&(entries->mutex));

	return success;
}

/*************************
Functions to respond to children
**************************/
void *pubServer(void *connection){

	printf("spawning pub thread\n");
	ConnectionRecord *temp = (ConnectionRecord *) connection;

	char* buffer =  malloc(MAX_LEN);
 	bool done = false;
 	/******************
 	initial read to buffer.
 	if buffer is terminate, send back terminate
 	if buffer is end, send back end
 	else->accept
 	*******************/
 	while(!done){
		read(temp->fromChild, buffer, MAX_LEN);
		printf("pub server received: %s\n", buffer);

		if(strcmp(buffer, "end") == 0){
			write(temp->toChild, "end", MAX_LEN);
		}
		else if(strcmp(buffer, "terminate") == 0){
			write(temp->toChild, "terminate", MAX_LEN);
			done = true;
		}

		else if(strcmp(buffer, "Topic Store Test") == 0){
			bool success = enqueue(&(temp->topicLists[0]), buffer);
			if(success){
				write(temp->toChild, "accept topic", MAX_LEN);
			}
		}
		else{
			printf("pub server accepts\n");
			write(temp->toChild, "accept", MAX_LEN);
		}
	}

	close(temp->toChild);
	free(buffer);
	return NULL;
}

void *subServer(void *connection){

	printf("spawning sub thread\n");
	ConnectionRecord *temp = (ConnectionRecord *) connection;

	char* buffer =  malloc(MAX_LEN);
 	bool done = false;
 	/******************
 	initial read to buffer.
 	if buffer is terminate, send back terminate
 	if buffer is end, send back end
 	else->accept
 	*******************/
 	while(!done){
		read(temp->fromChild, buffer, MAX_LEN);
		printf("sub server received: %s\n", buffer);

		if(strcmp(buffer, "end") == 0){
			write(temp->toChild, "end", MAX_LEN);
		}
		else if(strcmp(buffer, "terminate") == 0){
			write(temp->toChild, "terminate", MAX_LEN);
			done = true;
		}
		else if(strcmp(buffer, "sub ready for topic") == 0){
			for(int i=0; i < temp->numTopics; i++){
				bool success = 	dequeue(&(temp->topicLists[i]));
				while(!success){
					printf("EntryList empty: try again\n");
					sleep(1);
					success = dequeue(&(temp->topicLists[0]));
				}
				write(temp->toChild, temp->topicLists[0].circularBuff[temp->topicLists[0].tail].data , MAX_LEN);
			}
		}
		else{
			printf("sub server accepts\n");
			write(temp->toChild, "accept", MAX_LEN);
		}
	}

	close(temp->toChild);
	free(buffer);
	return NULL;
}
/*****************************88******************8*********
*********************************************************8*/



