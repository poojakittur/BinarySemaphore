//Name : Pooja Kittur 
//Course :COSC 6360 Operating Systems
//Assignment Name : Second Assignment
//This is the Server program - Hardcoded to run on port 5003

#include<unistd.h> 
#include<stdio.h> 
#include<sys/socket.h> 
#include<stdlib.h> 
#include<netinet/in.h> 
#include<string.h> 

void sem_create(int);
void sem_P(int, int);
void sem_V(int, int);
void sem_destroy(int, int);
void printQueue(int);

static int semTableId = 0;
int arraySize = 64;

struct semaphore{
	int sem_value;
	struct Queue *queue;
	int queueLength;

} *semaphores;

struct Queue{
	int socketId;
	struct Queue *next;
};

int main(int argc, char const *argv[]){
	int sockfd, newsockfd, portno, clilen, n;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[256], opcode;
	int sem_id;
	int i;
	
	//Dynamically create the semaphore data structure and assign the value of sem_value -1
	semaphores = malloc(arraySize *sizeof(semaphores));
	for(i=0; i< arraySize; i++){
		semaphores[i].sem_value = -1;
	}
	
	//Create a new socket of stream type
	//Code reference(Line no : 44 - 76) : http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html 
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Error while creating the socket");
		return (-1);
	}

	/*if (argc < 2) {

 		fprintf(stderr,"ERROR, no port provided\n");
 		exit(1);
 	}*/ //code to uncomment if the port number is specified in system arguments

	
	//Sets all the values of buffer to zero
	bzero((char *) &serv_addr, sizeof(serv_addr));
	//portno = atoi(argv[1]);
	portno = atoi("5003");
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	//Binding socket to the address
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		 perror("ERROR on binding");

	//Listen on the socket for connection	
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	
	while(1){
		
		// accept() call causes the process to block until a client connects to server.
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

 		if (newsockfd < 0)
			perror("ERROR on accept");

		bzero(buffer,256);
		
		//Reads the incoming request
 		n = read(newsockfd,buffer,255);
		sscanf(buffer,"%c  %d", &opcode, &sem_id);

		switch(opcode){
			case 'C':sem_create(newsockfd);
				break;
			case 'P':sem_P(sem_id, newsockfd);
				break;
			case 'V':sem_V(sem_id, newsockfd);
				break;
			case 'D':sem_destroy(sem_id, newsockfd);
				break;
		}
			
	}
}	

void sem_create(int socktFd){
	int buffer;
	int n;
	int i;
	int sem_id;
	int initialArraySize;

	printf("Create function () \n");
	
	//Reallocate the memory if the semaphore table reaches the initial size 64
	if(semTableId >= arraySize)
	{
		initialArraySize = arraySize;
		arraySize = arraySize + arraySize;
		semaphores = realloc(semaphores, arraySize *sizeof(semaphores));
		for(i = initialArraySize; i < arraySize; i++)
			semaphores[i].sem_value = -1;

	}
	sem_id = semTableId;
	semaphores[semTableId].sem_value = 1;

	n = write(socktFd, &sem_id, sizeof(int));
	if (n < 0) 
		perror("ERROR writing to socket");
	semTableId++;
	
	//close(socktFd);
}

void sem_P(int sem_id, int socktFd){
	int buffer;
	int n;
	
	printf("P function () -> Sem id received is : %d\n", sem_id);
	
	n = semaphores[sem_id].sem_value;
	if(n == -1){
		buffer = -1;
		n = write(socktFd, &buffer, sizeof(int));

	}else if(n == 1){    
		semaphores[sem_id].sem_value = 0;
		buffer = 0;
		n = write(socktFd, &buffer, sizeof(int));

	}else{
		if(semaphores[sem_id].queueLength == 0){
			printf("Resource is already being used \n");
			//Create a node in queue and add new socketid to queue
			struct Queue *newNode = (struct Queue*)malloc(sizeof(struct Queue));
			newNode -> socketId = socktFd;
			semaphores[sem_id].queue = (struct Queue*)malloc(sizeof(struct Queue));
			semaphores[sem_id].queue = newNode;

			printQueue(sem_id);
			
		}else{
			printf("Resource is already being used\n");
			//Create a node in queue and add new socketid to queue
			struct Queue *newNode = (struct Queue*)malloc(sizeof(struct Queue));
			newNode -> socketId = socktFd;
			struct Queue *position = semaphores[sem_id].queue;

			while(position -> next != NULL){
				position = position -> next;
			}
			position -> next = newNode;
			printQueue(sem_id);
		}
		semaphores[sem_id].queueLength++;

	}
}

void sem_V(int sem_id, int socktFd){
	int buffer;
	int n;
	int waitingSocktId;

	printf("V function () -> Sem id received is : %d\n", sem_id);

	if(semaphores[sem_id].sem_value == -1){  //if semaphore doesnt exist
		buffer = -1;
	}else if(semaphores[sem_id].sem_value == 1){
		semaphores[sem_id].sem_value = 1;
		buffer = 0;
	}else{					//when value of semaphore is zero
		if(semaphores[sem_id].queueLength > 0){
			waitingSocktId = semaphores[sem_id].queue -> socketId;
			semaphores[sem_id].queueLength--;

			if(semaphores[sem_id].queueLength > 0)
				semaphores[sem_id].queue = semaphores[sem_id].queue -> next;

			buffer = 0;
			printf("Serving sockt id : %d\n",waitingSocktId); 
			n = write(waitingSocktId, &buffer, sizeof(int));
		}else
			buffer = 0;
		
	}
	n = write(socktFd, &buffer, sizeof(int));
	if (n < 0) 
		perror("ERROR writing to socket");

}

void sem_destroy(int sem_id, int socktFd){

	int buffer;
	int n;

	printf("Destroy function () -> Sem id received is : %d\n", sem_id);
	if(semaphores[sem_id].sem_value == -1){
		buffer = -1;
	}else{
		semaphores[sem_id].sem_value = -1;
		buffer = 0;
	}
	n = write(socktFd, &buffer, sizeof(int));
	if (n < 0) 
		perror("ERROR writing to socket");

}

void printQueue(int sem_id){
	printf("Resources already in Queue are : \t");
	struct Queue *position = semaphores[sem_id].queue;
	while(position != NULL){
		printf("%d\t", position -> socketId);
		position = position -> next;
	}
	printf("\n");
	return;	
}



