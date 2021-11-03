#include "allheaders.h"

using namespace std;

int serverSocket, newSocket;
//tracker_socket contains fd for connection with tracker
int tracker_socket;
struct sockaddr_in serverAddr;
struct sockaddr_storage serverStorage;
char cmds[1024];
socklen_t addr_size;
pthread_t rthreads[100];

// Server


void* receiving_thread(void*);
void* receiving_tracker(void*);
void* user_thread(void*);


//- Bind to a address
int main(int argc, char *argv[])
{   

    int port = strtol(argv[1],NULL,10);
    int trackerport = strtol(argv[2],NULL,10);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

    bind(serverSocket,
		(struct sockaddr*)&serverAddr,
		sizeof(serverAddr));
    
    if (listen(serverSocket, 50) == 0)
		printf("Listening\n");
	else
		printf("Error\n");

    //do not know what data structure to send receive thread
	string hello = "";
    // Another thread will listen for connection
	pthread_create(&rthreads[0],NULL,receiving_thread,&hello);
    




// - Fetch tracker address and create a connect request and if pass then create a thread

    tracker_socket = socket(AF_INET,SOCK_STREAM, 0);

    // Initialise port number and address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(trackerport);

    // Initiate a socket connection
    int connection_status = connect(tracker_socket,
                                    (struct sockaddr*)&server_address,
                                    sizeof(server_address));

    // Check for connection error
    if (connection_status < 0) {
        puts("Error connecting to tracker\n");
        return 0;
    }

    cout<<"Connection estabilished with tracker n"<<endl;

    //this thread will receive all request from tracker
    pthread_create(&rthreads[0],NULL,receiving_tracker,&hello);





//  Main thread will take commands and send to tracker using fd created earlier
    while(1)
    {
        string cmd;
        cin>>cmd;

        if(cmd=="getport")
        {
            cout<<"sending"<<cmd<<endl;
            string client_name;
            cin>>client_name;
            send(tracker_socket,cmd.c_str(),sizeof(cmd),0);
            send(tracker_socket,client_name.c_str(),sizeof(client_name),0);        
        }

        sleep(0.1);
        
    }

    return 0;

}



// receiving thread will take request on port and create a thread
void* receiving_thread(void *p)
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

        pthread_create(&rthreads[i],NULL,user_thread,&newSocket);
        i++;
    }
    
}




// receiving tracker will receive cmds from tracker
void* receiving_tracker(void *p)
{
    char buffer[1024];
    char message[1024];
    while(1)
    {
        memset(buffer, 0, 1024);
        memset(message,0,1024);
        read(tracker_socket,buffer,1024);

        string cmd = string(buffer);

        if(cmd=="recv_port")
        {
            cout<<"recv_port is received"<<endl;
            read(tracker_socket,message,1024);
            cout<<"port received is "<<message<<endl;
        }

        sleep(0.1);
        
    }

    
}


//thread for a new user 
void* user_thread(void *p)
{


    int fd = *((int*)p);
    char buffer[1024];
    char message[1024];
    while(1)
    {

        memset(buffer, 0, 1024);
        read(fd,buffer,1024);
        string cmd = string(buffer);
        if(cmd=="download")
        {
            //download thread
        }

    }

}
