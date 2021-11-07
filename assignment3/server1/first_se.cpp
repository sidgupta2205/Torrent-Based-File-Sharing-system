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
#include<map>
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
int port2;
struct params
{
	char file_name[1024];
	int newfd;
};

struct clients
{
	int id;
	int fd;
	int port;
	string ip;
};

struct client_message
{
	int sender;
	int receivor;
	string message;
};

map<int,clients> data;

int cc=0;
	// Initialize variables
int serverSocket, newSocket;
struct sockaddr_in serverAddr;
struct sockaddr_storage serverStorage;
char cmds[1024];
socklen_t addr_size;



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


	}

	
	sleep(1);

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

void* send_message(void *p)
{
	char buffer[1024];
	// read client
	// read message
	// send message
	// send message to receivor
	client_message* m = (client_message*)p;
	string recv_message="www";
	read(m->sender,buffer,1024);
	m->receivor = strtol(buffer,NULL,10);
	read(m->sender,buffer,1024);
	cout<<data[m->receivor].fd<<endl;	
	send(data[m->receivor].fd,recv_message.c_str(),sizeof(recv_message),0);
	send(data[m->receivor].fd,buffer,1024,0);
	
	//send(data[m->receivor].fd,recv_message.c_str(),sizeof(recv_message),0);
	pthread_exit(NULL);
}

void* connect(void* p)
{

	char * buf = (char *)p;
	string t = string(buf);

	cout<<t<<endl;
	sleep(3);
	// clients *cl = (clients*)p;
	// data[cl->id] = *cl; 
	// cout<<"connected with "<<cl->id<<endl;
	pthread_exit(NULL);

}



void* receving_thread(void* p)
{
	int i=0;
	while (1) {
		cout<<"receiving thread is active"<<endl;
		addr_size = sizeof(serverStorage);

		// Extract the first
		// connection in the queue
		newSocket = accept(serverSocket,
						(struct sockaddr*)&serverStorage,
						&addr_size);
		params p;
		memset(cmds, 0, 1024);
		//read cmds 1. connect 2. transfer message to client 3. get all clients registered 4. download
		cout<<"receiving thread has accepted a connection"<<endl;
		read(newSocket,cmds,1024);
		cout<<cmds<<endl;
		if(strcmp(cmds,"download")==0)
		{

				read(newSocket,
				p.file_name, 1024);
			p.newfd = newSocket;
			

			
			// Creater readers thread
			if (pthread_create(&readerthreads[i++], NULL,
							reader, &p)
				!= 0)

				// Error in creating thread
				printf("Failed to create thread\n");
			

		}

		else if(strcmp(cmds,"connect")==0)
		{
			char buffer[1024];
			read(newSocket,buffer,1024);

			if (pthread_create(&readerthreads[i++], NULL,
							connect, &buffer)
				!= 0)

				// Error in creating thread
				printf("Failed to create thread\n");

			//save fd in a map with id as key and struct as object
				
		}

		else if(strcmp(cmds,"sendmessage")==0)
		{
			//read client
			char buffer[1024];
			read(newSocket,buffer,1024);
			cout<<buffer<<endl;
			sleep(3);
			//read message 
		}

		else if(strcmp(cmds,"recv_message"))
		{

			cout<<"recv_message is detected"<<endl;
			char buffer[1024];
			read(newSocket,buffer,1024);
			cout<<"port received is "<<buffer<<endl;

		}

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

	cout<<"receiving thread is dead"<<endl;
	pthread_exit(NULL);
}

// Driver Code
int main(int argc, char *argv[])
{
    int port = strtol(argv[1],NULL,10);
	port2 = strtol(argv[2],NULL,10);
	int trackerport = strtol(argv[3],NULL,10);
	// Bind the socket to the
	// address and port number.

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);	

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

	string hello = "";
	pthread_create(&writerthreads[0],NULL,receving_thread,&hello);

	string cmds;
	while(1)
	{
			cmds.clear();
			cin>>cmds;

		if(cmds=="connect")
		{
			int network_socket;

			// Create a stream socket
			network_socket = socket(AF_INET,
									SOCK_STREAM, 0);

			// Initialise port number and address
			struct sockaddr_in server_address;
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = INADDR_ANY;
			server_address.sin_port = htons(port2);

			// Initiate a socket connection
			int connection_status = connect(network_socket,
											(struct sockaddr*)&server_address,
											sizeof(server_address));

			// Check for connection error
			if (connection_status < 0) {
				puts("Error\n");
				return 0;
			}

			printf("Connection estabilished\n");
			string message = "hello from client 1";
			// Send data to the socket
			send(network_socket, cmds.c_str(),
				sizeof(cmds), 0);
			send(network_socket,message.c_str(),sizeof(message),0);
		}

		else if(cmds=="tracker_connect")
		{
			int network_socket;

			network_socket = socket(AF_INET,
									SOCK_STREAM, 0);

			// Initialise port number and address
			struct sockaddr_in server_address;
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = INADDR_ANY;
			server_address.sin_port = htons(trackerport);

			// Initiate a socket connection
			int connection_status = connect(network_socket,
											(struct sockaddr*)&server_address,
											sizeof(server_address));

			// Check for connection error
			if (connection_status < 0) {
				puts("Error\n");
				return 0;
			}

			printf("Connection estabilished\n");
			string message = to_string(port);
			// Send data to the socket
			send(network_socket, cmds.c_str(),
				sizeof(cmds), 0);
			send(network_socket,message.c_str(),sizeof(message),0);
		}

		else if(cmds=="getport")
		{

			int network_socket;

			network_socket = socket(AF_INET,
									SOCK_STREAM, 0);

			// Initialise port number and address
			struct sockaddr_in server_address;
			server_address.sin_family = AF_INET;
			server_address.sin_addr.s_addr = INADDR_ANY;
			server_address.sin_port = htons(trackerport);

			// Initiate a socket connection
			int connection_status = connect(network_socket,
											(struct sockaddr*)&server_address,
											sizeof(server_address));

			// Check for connection error
			if (connection_status < 0) {
				puts("Error\n");
				return 0;
			}

			string cm = "getport";
			string cli = "1";

			send(network_socket, cm.c_str(),
				cm.size(), 0);
			cout<<"sending cmd is "<<cm<<endl;
			sleep(2);
			send(network_socket,cli.c_str(),cli.size(),0);
			cout<<"sending client"<<endl;
			
		}

		
			sleep(3);


		}
	

	// Array for thread

	int i = 0;

	return 0;
}

