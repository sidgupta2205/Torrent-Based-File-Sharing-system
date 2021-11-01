// C program for the Client Side
#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// inet_addr
#include <arpa/inet.h>
#include <unistd.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>

// Function to send data to
// server socket.

using namespace std;



void* clienthread(void* args)
{


	std::string client_request = *reinterpret_cast<std::string*>(args);
	
	int network_socket;

	// Create a stream socket
	network_socket = socket(AF_INET,
							SOCK_STREAM, 0);

	// Initialise port number and address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8989);

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

	// Send data to the socket
	send(network_socket, client_request.c_str(),
		sizeof(client_request), 0);

	sleep(5);

	long long size;
	read(network_socket,
			&size, sizeof(size));

	cout<<"file size is"<<size<<endl;

	//create a new file

	int n;
	char buf[1024];
	int fd2;

	if ((fd2=open(client_request.c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
		cout<<"creat error"<<endl;

	while(size>0)
	{
		n=read(network_socket,buf,1024);
		// copy contents in the file
		if(write(fd2,buf,n)!=n)
		{
			//cout<<strerror(errno);
			cout<<"error"<<endl;
			break;
		}
		// reduce size 
		size = size-n;
		cout<<size<<endl;
	}

	cout<<"copy success"<<endl;

	// Close the connection
	close(network_socket);
	pthread_exit(NULL);

	return 0;
}

// Driver Code
int main()
{
	

	// Input
	string choice;
	choice = "teams.png";
	pthread_t tid;

	// Create connection
	// depending on the input
	// send the name of the file to download
		

		// Create thread
		pthread_create(&tid, NULL,
					clienthread,
					&choice);
		sleep(20);
		
	// Suspend execution of
	// calling thread
	pthread_join(tid, NULL);
}
