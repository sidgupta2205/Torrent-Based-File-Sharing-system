// C program for the Server Side
#include<iostream>
// inet_addr
#include <arpa/inet.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

// Semaphore variables
sem_t x, y;
pthread_t tid;
pthread_t writerthreads[100];
pthread_t readerthreads[100];
int readercount = 0;

struct params
{
	char file_name[1024];
	int newfd;
};


// Reader Function
void* reader(void* param)
{
	int n;
	char buf[1024];
	int fd1;
	int fd2;

	


	// Lock the semaphore
	params *p = (params *)param;
    cout<<p->file_name<<endl;
    
	struct stat binfo;
        lstat(p->file_name,&binfo);
        unsigned long long size;
		size = binfo.st_size; 
        
    send(p->newfd, &size,
		sizeof(size), 0);

	
	
	//receive the file
    //check if its available
    //send the size of file
    //start sending the file to client
	// Unlock the semaphore
	
	long long curr=0; 

	if ((fd1=open(p->file_name,O_RDWR))<0)
	{
		cout<<"file does not exist"<<endl;
	}
	else
	{
		while((n=read(fd1,buf,1024))>0)
		{
			send(p->newfd, &buf,1024, 0);
			curr = curr+n;
			cout<<curr<<endl;
		}

		cout<<"file_sent"<<endl;
		
	}

	
	sleep(500);

	// Lock the semaphore
	

	
	pthread_exit(NULL);
}

// Writer Function
void* writer(void* param)
{
	printf("\nWriter is trying to enter");

	// Lock the semaphore
	sem_wait(&y);

	printf("\nWriter has entered");

	// Unlock the semaphore
	sem_post(&y);

	printf("\nWriter is leaving");
	pthread_exit(NULL);
}

// Driver Code
int main()
{
	// Initialize variables
	int serverSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;

	socklen_t addr_size;
	sem_init(&x, 0, 1);
	sem_init(&y, 0, 1);

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8989);
    
	// Bind the socket to the
	// address and port number.
	bind(serverSocket,
		(struct sockaddr*)&serverAddr,
		sizeof(serverAddr));

	// Listen on the socket,
	// with 40 max connection
	// requests queued
	if (listen(serverSocket, 50) == 0)
		printf("Listening\n");
	else
		printf("Error\n");

	// Array for thread
	pthread_t tid[60];

	int i = 0;

	while (1) {
		addr_size = sizeof(serverStorage);

		// Extract the first
		// connection in the queue
		newSocket = accept(serverSocket,
						(struct sockaddr*)&serverStorage,
						&addr_size);
		params p;

		read(newSocket,
			p.file_name, 1024);
        p.newfd = newSocket;
        
		
        // Creater readers thread
        if (pthread_create(&readerthreads[i++], NULL,
                        reader, &p)
            != 0)

            // Error in creating thread
            printf("Failed to create thread\n");
		
		

		if (i >= 50) {
			// Update i
			i = 0;

			while (i < 50) {
				// Suspend execution of
				// the calling thread
				// until the target
				// thread terminates
				pthread_join(writerthreads[i++],
							NULL);
				pthread_join(readerthreads[i++],
							NULL);
			}

			// Update i
			i = 0;
		}
	}

	return 0;
}
