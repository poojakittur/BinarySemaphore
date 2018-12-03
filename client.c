//Name : Pooja Kittur
//Course : COSC 6360 Operating Systems 
//ASsignment Name : Second Assignment
//This is the Client program - Hardcoded to run on port 5003

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<stdlib.h>
#include<string.h>

// you can add all the includes you need but
// cannot modify the main program

void error(char *msg)
{
 perror(msg);
 exit(0);
}

int sem_create();
int sem_P(int sem_id);
int sem_V(int sem_id);
int sem_destroy(int sem_id);
int createSocket();
void closeSocket(int);

int main() {
    int mutex1, mutex2; //semaphores
    int pid; // process ID
    if ((mutex1 = sem_create()) < 0) {
         printf("Cannot create semaphore mutex1.\n");
         _exit(1);
    }  //if
    if ((mutex2 = sem_create()) < 0) {
         _exit(2);
         printf("Cannot create semaphore mutex2.\n");
    }  // if
    // basic checks
    sem_P(mutex2);
    printf("Completed a P() operation on mutex2.\n");
    sem_V(mutex2);
    sem_V(mutex2);
    printf("Completed two V() operations on mutex2.\n");
    
    if ((pid = fork()) == 0) {
        // child process
        printf("Child process requests mutex1.\n");
        sem_P(mutex1);
        printf("Child process got mutex1.\n");
        sleep(10);
        printf("Child process is ready to release mutex1.\n");
        sem_V(mutex1);
        printf("Child process released mutex1.\n");
        _exit(0);
    } // if
    sleep(3);
    printf("Parent process requests mutex1.\n");
    sem_P(mutex1);
    printf("Parent process got mutex1.\n");
    sleep(8);
    printf("Parent process is ready to release mutex1.\n");
    sem_V(mutex1);
    printf("Parent process released mutex1.\n");

    // Ending
    sem_destroy(mutex1);
    if (sem_P(mutex1) >= 0) {
        printf("OOPS! Semaphore mutex1 was not properly destroyed!\n");
    } // if
    sem_destroy(mutex2);
    if (sem_P(mutex2) >= 0) {
        printf("OOPS! Semaphore mutex2 was not properly destroyed!\n");
    } //if
    if (sem_P(mutex1) >= 0) {
        printf("OOPS! Server accepted a call to a non-existent semaphore!\n");
    }
} // main

int sem_create() {
   	int sem_id;
	char buffer[256];
	int sockfd;
	int n;

	sockfd = createSocket();

	//Create a buffer with operation code(C)
	sprintf(buffer,"%c %d", 'C', 0);

	//Write message to server
	n = write(sockfd,buffer,strlen(buffer));
 	
	if (n < 0)
 		error("ERROR writing to socket");
 	
	//Read the response from server
 	n = read(sockfd, &sem_id, sizeof(int));

	if (n < 0)
 		error("ERROR reading from socket");
	
	//close the socket
	closeSocket(sockfd);
	return sem_id;
}

int sem_P(int sem_id) {
    	int n;
	char buffer[256];
	int sockfd;
	int result;

	sockfd = createSocket();

	//Create a buffer with operation code(P) and semid
	sprintf(buffer,"%c %d", 'P', sem_id);
	n = write(sockfd,buffer,strlen(buffer));
 	
	if (n < 0)
 		error("ERROR writing to socket");
 	bzero(buffer,256);
 	n = read(sockfd, &result, sizeof(int));
 	
	if (n < 0)
 		error("ERROR reading from socket");
	
	closeSocket(sockfd);
	return result;
}

int sem_V(int sem_id) {
   	int n;
	char buffer[256];
	int sockfd;
	int result;

	sockfd = createSocket();

	//Create a buffer with operation code(V) and semid
	sprintf(buffer,"%c %d", 'V', sem_id);
	n = write(sockfd,buffer,strlen(buffer));
 	
	if (n < 0)
 		error("ERROR writing to socket");
 	bzero(buffer,256);
 	n = read(sockfd, &result, sizeof(int));
 	
	if (n < 0)
 		error("ERROR reading from socket");

	closeSocket(sockfd);
	return result;
}

int sem_destroy(int sem_id) {
	int n;
	char buffer[256];
	int sockfd;
	int result;

	sockfd = createSocket();

	//Create a buffer with operation code(P) and semid
	sprintf(buffer,"%c %d", 'D', sem_id);
	n = write(sockfd,buffer,strlen(buffer));
 	
	if (n < 0)
 		error("ERROR writing to socket");
 	bzero(buffer,256);
 	
	n = read(sockfd,&result,sizeof(int));
 	
	if (n < 0)
 		error("ERROR reading from socket");

	closeSocket(sockfd);
	return result;
}

int createSocket(){
	int sockfd, portno, n;
 	struct sockaddr_in serv_addr;
	struct hostent *server;
	int errcode, sem_id;
	char buffer[256];
	char hostname[1024];

 	portno = atoi("5003");

	//Socket creation
	//Code reference(Line no : 194 - 214) - http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/socket.html
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		 error("ERROR opening socket");
	
	//server = gethostbyname(argv[1]);
	//gethostname(hostname, 1023);
	server = gethostbyname("localhost");
	if (server == NULL) {
		 fprintf(stderr,"ERROR, no such host\n");
		 exit(0);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
 	bcopy((char *)server->h_addr,
		(char *)&serv_addr.sin_addr.s_addr,
 		server->h_length);
 	serv_addr.sin_port = htons(portno);

	//Establish the connection with server
	if (connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
	 	error("ERROR connecting");

	return sockfd;
}


void closeSocket(int socketId){
	close(socketId);
}

