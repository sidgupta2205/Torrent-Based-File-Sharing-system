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
//thread count
int ithr=0;
map<string,bool> seeded;
// Server


void* receiving_thread(void*);
void* receiving_tracker(void*);
void* user_thread(void*);
void* user_thread_connected(void*);
string getfilesize(string);
void getports(vector<int>&ports,string result);
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
	pthread_create(&rthreads[ithr],NULL,receiving_thread,&hello);
    ithr++;    




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
    string m = to_string(port);
    send(tracker_socket,m.c_str(),sizeof(m),0);
    pthread_create(&rthreads[ithr],NULL,receiving_tracker,&hello);
    ithr++;




//  Main thread will take commands and send to tracker using fd created earlier
    while(1)
    {
        string cmd;
        cin>>cmd;

        cout<<"sending"<<cmd<<endl;
        send(tracker_socket,cmd.c_str(),sizeof(cmd),0);
        if(cmd=="getport")
        {
            
            string client_name;
            cin>>client_name;
            send(tracker_socket,client_name.c_str(),sizeof(client_name),0);        
        }
        else if(cmd=="seed_file")
        {
            string file_name;
            cin>>file_name;
            send(tracker_socket,file_name.c_str(),sizeof(file_name),0);
            seeded[file_name] = true;
        }
        else if(cmd=="download_file")
        {
            string file_name;
            cin>>file_name;
            send(tracker_socket,file_name.c_str(),sizeof(file_name),0);
        }

        sleep(0.1);
        
    }

    return 0;

}



// receiving thread will take request on port and create a thread
void* receiving_thread(void *p)
{
    
	while (1) {
		cout<<"receiving thread is active"<<endl;
		addr_size = sizeof(serverStorage);
        
		// Extract the first
		// connection in the queue
		newSocket = accept(serverSocket,
						(struct sockaddr*)&serverStorage,
						&addr_size);

        cout<<"new user is connected"<<endl;

        pthread_create(&rthreads[ithr],NULL,user_thread_connected,&newSocket);
        ithr++;

        sleep(0.1);
        
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
        
        string cmd;
        cmd.clear();
        cmd = string(buffer);

        if(cmd=="recv_port")
        {
            cout<<"recv_port is received"<<endl;
            read(tracker_socket,message,1024);
            cout<<"port received is "<<message<<endl;
        }

        else if(cmd=="file_port")
        {
            cout<<"file port received "<<endl;
            read(tracker_socket,message,1024);
            string result = string(message);
            if(result=="-1")
            {
                cout<<"file not found"<<endl;
            }
            else
            {
                cout<<"file found at "<<result<<endl;
                cout<<"creating a few thread for connecting with peer"<<endl;
                memset(message,0,1024);
                cout<<"reading file size"<<endl;
                read(tracker_socket,message,1024);
                string size = string(message);
                cout<<"file size is "<<size<<endl;
                //int anss = strtol(result.c_str(),NULL,10);
                vector<int> ports;
                getports(ports,result);
                string anss = to_string(ports[0]);
                pthread_create(&rthreads[ithr++],NULL,user_thread,&anss);
            }

        }

        else if (cmd=="getfilesize")
        {
            string res ="filename";
            send(tracker_socket,res.c_str(),res.size(),0);
            memset(message,0,1024);
            read(tracker_socket,message,1024);
            string fname = string(message);
            string siz = getfilesize(fname);
            send(tracker_socket,siz.c_str(),siz.size(),0);

        }

        sleep(0.1);
        
    }

    
}


//thread for a new user 
void* user_thread(void *p)
{

    int port = *((int *)p);
    cout<<"connecting at port "<<port<<endl;
    int user_socket = socket(AF_INET,SOCK_STREAM, 0);
    // Initialise port number and address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    // Initiate a socket connection
    int connection_status = connect(user_socket,
                                    (struct sockaddr*)&server_address,
                                    sizeof(server_address));

    // Check for connection error
    if (connection_status < 0) {
        puts("Error connecting to user\n");
        pthread_exit(NULL);
    }

    cout<<"connected success with user"<<endl;

    sleep(0.1);
    // send download file cmd
    string cmd="download_file";
    

            
            
    if(cmd=="download_file")
    {
        cout<<"sending download cmd to user"<<endl;
            char message[1024];
        string cmd = "download_file";
        send(user_socket,cmd.c_str(),sizeof(cmd),0); 
        // recv response
        read(user_socket,message,1024);

        string m = string(message);
        int fd1;
        if(m=="send_file")
        {
            cout<<"enter file name"<<endl;
            string file_name="teams.png";
            send(user_socket,file_name.c_str(),sizeof(file_name),0);
            memset(message,0,1024);
            read(user_socket,message,1024);
            string sized = string(message);
            int size = strtol(message,NULL,10);
            if(sized=="-1")
            {
                cout<<"file not found on user side"<<endl;
            }
            else
            {
                cout<<"file found at user with size "<<sized<<endl;
                string res = "ready";
                send(user_socket,res.c_str(),res.size(),0);

                int n;
                char buf[1024];
                int fd2;

                if ((fd2=open(file_name.c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
                
                {
                    cout<<"creat error"<<endl;
                    pthread_exit(NULL);
                    
                }
                

                
                while(size>0)
                {
                    n=read(user_socket,buf,1024);
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

            }
        
            // send file name
            // if size > 0 then start a new thread and download
            // else print file not present and return

            cmd="doNothing";
            sleep(0.1);

        }

        
    
    }
    pthread_exit(NULL);
}

string getfilesize(string filename)
{
    struct stat binfo;
    lstat(filename.c_str(),&binfo);
    unsigned long long size;
	size = binfo.st_size;
    string res = to_string(size); 
    return res;
}

void* user_thread_connected(void *p)
{

    int fd = *((int *)p);
    char buffer[1024];
    char message[1024];
    
    cout<<"user thread with fd "<<fd<<" is activated"<<endl;

    char cmd[1024];
    while(1)
    {
        memset(cmd,0,1024);
        read(fd,cmd,1024);

        string cmds = string(cmd);

        if(cmds=="download_file")
        {
            cout<<"download_file received";
            string res = "send_file";
            send(fd,res.c_str(),sizeof(res),0);
            memset(message,0,1024);
            read(fd,message,1024);
            string file_name = string(message);
            if(seeded[file_name])
            {
                //create a thread and start sending
                cout<<"file seeded by this user"<<endl;
                struct stat binfo;
                lstat(file_name.c_str(),&binfo);
                unsigned long long size;
		        size = binfo.st_size;
                string res = to_string(size); 
                send(fd,res.c_str(),sizeof(res),0);

                memset(message,0,1024);
                read(fd,message,1024);
                res = string(message);
                int fd1;
                 if(res=="ready")
                 {
                        cout<<"sending file intiatiated"<<endl;
                        if ((fd1=open(file_name.c_str(),O_RDWR))<0)
                        {
                            cout<<"file does not exist"<<endl;
                            pthread_exit(NULL);
                            
                        }
                        else
                        {
                            char buf[1024];
                            int n;
                            int curr=0;
                            while((n=read(fd1,buf,1024))>0)
                            {
                                send(fd, &buf,1024, 0);
                                curr = curr+n;
                                cout<<curr<<endl;
                            }

                            cout<<"file_sent"<<endl;
                        
                        }
                 }
                
                
                           
            }
            else
            {
                cout<<"file not seeded by this user"<<endl;
                string res="-1";
                send(fd,res.c_str(),sizeof(res),0);
            }
        }

        sleep(0.1);
    }

    
    pthread_exit(NULL);

}


void getports(vector<int>&ports,string result)
{
    istringstream ss(result);
  
    string word; // for storing each word
  
    // Traverse through all words
    // while loop till we get 
    // strings to store in string word
    while (ss >> word) 
    {
        // print the read word
        cout << word << "\n";
        ports.push_back(stoi(word));
    }

}