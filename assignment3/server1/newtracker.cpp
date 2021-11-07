#include "allheaders.h"
using namespace std;


// Tracker
// - Start listening with a port

int serverSocket, newSocket;

struct sockaddr_in serverAddr;
struct sockaddr_storage serverStorage;
char cmds[1024];
socklen_t addr_size;
pthread_t rthreads[100];
map<int,int>data;
map<string,int>files;
void* user_thread(void*);

int main(int argc,char *argv[])
{
    int port = strtol(argv[1],NULL,10);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);

    bind(serverSocket,(struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if (listen(serverSocket, 50) == 0)
		printf("Listening\n");
	else
		printf("Error\n");


// - For every connect request create a thread for that user
    int i=0;
	while (1) {
		cout<<"receiving thread is active"<<endl;
		addr_size = sizeof(serverStorage);

		// Extract the first
		// connection in the queue
		newSocket = accept(serverSocket,
						(struct sockaddr*)&serverStorage,
						&addr_size);
        

        data[i] = newSocket;
        cout<<"data saved with "<<i<<" "<<data[i]<<endl;
        pthread_create(&rthreads[i],NULL,user_thread,&newSocket);
        sleep(0.1);
        i++;
    }

    return 0;
}


// - Send and receive on that thread
void* user_thread(void *p)
{

    int fd = *((int*)p);
    char buffer[1024];
    char message[1024];
    read(fd,message,1024);
    int port_no = strtol(message,NULL,10);
    cout<<"new user port no is "<<port_no<<endl;
    cout<<"new user thread is created with fd "<<fd<<endl;
    while(1)
    {

        memset(buffer, 0, 1024);
        read(fd,buffer,1024);
        string cmd = string(buffer);
        if(cmd=="getport")
        {
            cout<<"recived request for port at"<<fd<<message;
            read(fd,message,1024);
            string cmdans = "recv_port";
            string port_client = to_string(data[strtol(message,NULL,10)]);
            send(fd,cmdans.c_str(),cmdans.size(),0);
            sleep(0.1);
            send(fd,port_client.c_str(),port_client.size(),0);
        }
        else if(cmd=="seed_file")
        {
            cout<<"adding file received"<<endl;
            read(fd,message,1024);
            files[string(message)] = port_no;
            cout<<"adding file "<<string(message)<<" "<<port_no<<endl;
        }
        else if(cmd=="download_file")
        {
            cout<<"downloading file received "<<endl;
            string cmdres = "file_port";
            send(fd,cmdres.c_str(),sizeof(cmdres),0);
            read(fd,message,1024);
            string file_name = string(message);
            
            if(files.find(file_name)==files.end())
            {
                string res = "-1";
                cout<<"file not found "<<endl;
                
                send(fd,res.c_str(),sizeof(res),0);
            }
            else
            {
                cout<<"file found at port "<<files[file_name]<<endl;
                string res = to_string(files[file_name]);
                send(fd,res.c_str(),sizeof(res),0);
            }
        }

        sleep(0.1);

    }

}

