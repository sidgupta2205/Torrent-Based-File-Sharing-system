#include "allheaders.h"

using namespace std;


struct Download
{
    Download(string filename,int port, int chunkns,int i,string si)
    {
        port_no = port;
        file_name = file_name;
        chunks = chunkns;
        completed = 0;
        index = i;
        size = strtol(si.c_str(),NULL,10);
    }
    int port_no;
    int fd;
    int chunks;
    int completed;
    int index;
    int size;
    string file_name;
};

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
map<string,string> fsizes;
//storing downloading files struct
vector<Download> Downloads;
vector<Download> Completed;

void* receiving_thread(void*);
void* receiving_tracker(void*);
void* user_thread(void*);
void* user_thread_connected(void*);
string getfilesize(string);
void getports(vector<int>&ports,string result);
void gettokens(vector<string>&,string);
void printall(vector<string>,int);
void copy_string(string &a,string b);
int create_file(string);
string getfilename(string);
string dest_file;
string src_file;
bool checkfile(string);

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
        getline(cin,cmd);

        
        vector<string> tokens;
        //send(tracker_socket,cmd.c_str(),cmd.size(),0);
        string tokenize;
        copy_string(tokenize,cmd);
        cout<<"sending "<<cmd<<endl;
        gettokens(tokens,tokenize);
        cout<<"sending "<<cmd<<endl;
        
        if(cmd=="getport")
        {
            send(tracker_socket,cmd.c_str(),sizeof(cmd),0);    
            string client_name;
            cin>>client_name;
            send(tracker_socket,client_name.c_str(),sizeof(client_name),0);        
        }
        else if(cmd=="seed_file")
        {
            send(tracker_socket,cmd.c_str(),sizeof(cmd),0);
            string file_name;
            cin>>file_name;
            send(tracker_socket,file_name.c_str(),sizeof(file_name),0);
            seeded[file_name] = true;
        }
        else if(cmd=="download_file")
        {
            send(tracker_socket,cmd.c_str(),sizeof(cmd),0);
            string file_name;
            cin>>file_name;
            send(tracker_socket,file_name.c_str(),sizeof(file_name),0);
        }

        else if(tokens[0]=="create_user")
        {
            if(tokens.size()<3)
            {
                cout<<"incorrect command"<<endl;
            }
            else
            {
                cout<<cmd<<endl;
                send(tracker_socket,cmd.c_str(),cmd.size(),0);
                
            }
            
        }

        else if(tokens[0]=="logout")
        {
            cout<<"logout initiated"<<endl;
            send(tracker_socket,cmd.c_str(),cmd.size(),0);

        }

        else if(tokens[0]=="login")
        {
            if(tokens.size()<3)
            {
                cout<<"incorrect commd"<<endl;
            }
            else
            {
                cout<<"login initiated"<<endl;
                send(tracker_socket,cmd.c_str(),cmd.size(),0);   
            }
        }

        else if(tokens[0]=="create_group")
        {
            if(tokens.size()<2)
            {
                cout<<"incorrect commd"<<endl;
            }
            else
            {
                cout<<"create group initiated"<<endl;
                send(tracker_socket,cmd.c_str(),cmd.size(),0);
            }

        }

        else if(tokens[0]=="list_groups")
        {
            send(tracker_socket,cmd.c_str(),cmd.size(),0);
        }

        else if(tokens[0]=="join_group")
        {
            if(tokens.size()<2)
            {
                cout<<"Invalid cmd"<<endl;
                continue;
            }
            send(tracker_socket,cmd.c_str(),cmd.size(),0);
        }

        else if(tokens[0]=="requests")
        {
            if(tokens.size()<3)
            {
                cout<<"Invalid cmd"<<endl;
                continue;
            }
            send(tracker_socket,cmd.c_str(),cmd.size(),0);
        }

        //accept_request group_id user_id
        else if(tokens[0]=="accept_request")
        {

            if(tokens.size()<3)
            {
                cout<<"Invalid cmd"<<endl;
                continue;
            }

            send(tracker_socket,cmd.c_str(),cmd.size(),0);

        }
        
        //upload_file path group_id
        else if(tokens[0]=="upload_file")
        {
            if(tokens.size()<3)
            {
                cout<<"Invalid command"<<endl;
                continue;
            }

            // check if file exists
            if(!checkfile(tokens[1]))
            {
                continue;
            }
            // check file size
            string file_size = getfilesize(tokens[1]);
            
            // extract file name
            string file_name = getfilename(tokens[1]);
            fsizes[file_name] = file_size;
            // append size to cmd
            string res = "upload_file "+file_name+" "+tokens[2];
            cout<<res<<endl;
            send(tracker_socket,cmd.c_str(),cmd.size(),0);
            // send to tracker

        }
        else if(tokens[0]=="list_files")
        {

            if(tokens.size()<2)
            {
                cout<<"Invalid cmd"<<endl;
                continue;
            }

            send(tracker_socket,cmd.c_str(),cmd.size(),0);

        }

        //download_file <group_id> <file_name> <destination_path>
        else if(tokens[0]=="download_files")
        {

            if(tokens.size()<4)
            {
                cout<<"Invalid cmd"<<endl;
                continue;
            }

            src_file = tokens[2];
            dest_file = tokens[3];
            send(tracker_socket,cmd.c_str(),cmd.size(),0);

            // taking into consideration destination path is always valid
            
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
    string file_name;
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
                
                pthread_create(&rthreads[ithr++],NULL,user_thread,&ports[0]);
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
        else if(cmd=="Registration success")
        {
            cout<<"User sucessfully registered"<<endl;
        }
        else if(cmd=="Registration failed")
        {
            cout<<"User cannot be registered"<<endl;
        }
        else if(cmd=="Login Success")
        {
            cout<<"User successfully logeed in "<<endl;
        }
        else if(cmd=="Login Failure")
        {
            cout<<"User Login failed"<<endl;
        }
        else if(cmd=="Group creation success")
        {
            cout<<"Group successfully created"<<endl;
        }
        else if(cmd=="Group creation failed")
        {
            cout<<"Group cannot be created"<<endl;
        }
        else if(cmd=="Not Logged In")
        {
            cout<<"User is not logged in"<<endl;
        }
        else if(cmd=="User Request Registered with group")
        {
            cout<<"User request is successfully Registered with group"<<endl;
        }
        else if(cmd=="User Already Present")
        {
            cout<<"User is already regitered with the group"<<endl;
        }
        else if(cmd=="User request Already Present")
        {
            cout<<"User request already regitered with the group"<<endl;
        }
        else if(cmd=="Unable to join group")
        {
            cout<<"Unable to join group at this momemt"<<endl;
        }
        else if(cmd=="Permission denied to user")
        {
            cout<<"User does not have permission for this group"<<endl;
        }
        else if(cmd=="User has not requested")
        {
            cout<<"Given user has not requested to join the group"<<endl;
        }
        else if(cmd=="User added in group")
        {
            cout<<"Given user has is successfully added to the group"<<endl;
        }
        else if(cmd=="Invalid request")
        {
            cout<<"User is not authorized for this group"<<endl;
        }
        else if(cmd=="File uploaded success")
        {
            cout<<"File is successfully uploaded in the group"<<endl;
        }
        else if(cmd=="File not Found")
        {
            cout<<"File not found on the server end"<<endl;
        }
        else if(cmd=="user logged out success")
        {
            cout<<"User is logged out successfully"<<endl;
        }

        else
        {

            vector<string> tokens;
            gettokens(tokens,cmd);
            
            if(tokens[0]=="group_details")
            {
                cout<<"Group details"<<endl;
                printall(tokens,1);
            }
            else if(tokens[0]=="request_list")
            {
                cout<<"request list details"<<endl;
                printall(tokens,1);
            }
            else if(tokens[0]=="getfilesize2")
            {
                cout<<tokens[1]<< " "<< fsizes[tokens[1]]<<endl;
                send(tracker_socket,fsizes[tokens[1]].c_str(),fsizes[tokens[1]].size(),0);
            }
            else if(tokens[0]=="list_file")
            {
                cout<<"files of group"<<endl;
                printall(tokens,1);
            }
            else if(tokens[0]=="file_info")
            {
                string size = tokens[1];
                string port_nos = tokens[2];
                cout<<size<<" "<<port_nos<<endl;
                vector<int> ports;
                getports(ports,port_nos);
                string anss = to_string(ports[0]);
                cout<<"asking for file with port "<<anss<<endl;
                // fill download dsa
                Download d(src_file,ports[0],1,Downloads.size(),size);
                //create dest file and add fd
                d.fd = create_file(dest_file);
                if(d.fd<0)
                {
                    cout<<"Unable to Download file"<<endl;
                }
                else
                {
                    Downloads.push_back(d);
                    // divide in chunks
                    //currently 1
                // distribute in threads
                                // entry in downloads
                pthread_create(&rthreads[ithr++],NULL,user_thread,&d);
                }
                
            }
            
        }
        sleep(0.1);
        
    }

    
}


//thread for a new user 
void* user_thread(void *p)
{
    Download *ptr = (Download *) p;

    // connect with user
    // ask for chunk of file
    // write in fd
    // check if download is completed
    // if no increment completed with lock
    // if yes move from downloads to src
    // add port no in downloading

    int port = ptr->port_no;
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
    string cmd="downloads_file";
    //string cmd="download_file";

    

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

    else if(cmd=="downloads_file")
    {
        string file_name = ptr->file_name;
        string res = "downloads_file "+file_name+" "+to_string(ptr->chunks);
        char message[1024];
        memset(message,0,1024);
        read(user_socket,message,1024);
        string sized = string(message);
        if(sized=="sending")
        {   
            //change as per chunksize
            int chunksize = ptr->size;
            int n;
            char buf[1024];
            int fd2 = ptr->fd;
            while(chunksize>0)
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
                    chunksize = chunksize-n;
                    cout<<chunksize<<endl;
            }

                cout<<"copy success"<<endl;
            //start downloading
        }
        else
        {
            cout<<"Downloading cannot be Completed"<<endl;
            //remove from Download dsa
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

    // check if that file chunk is present or not
    // if present send that chunk
    // 
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
        vector<string>token;
        gettokens(token,cmds);
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
            else if(token[0]=="")
            {
                cout<<"file not seeded by this user"<<endl;
                string res="-1";
                send(fd,res.c_str(),sizeof(res),0);
            }
        }

        else if(token[0]=="downloads_file")
        {
            string file_name = token[1];
            int chunks = strtol(token[2].c_str(),NULL,10);
            if(checkfile(file_name))
            {
                string res="sending";
                cout<<"Preparing to send file"<<endl;
                send(fd,res.c_str(),sizeof(res),0);

                int fd1=open(file_name.c_str(),O_RDWR);

                char buf[1024];
                int n;
                int curr=0;
                while((n=read(fd1,buf,1024))>0)
                {
                    send(fd, &buf,1024, 0);
                    curr = curr+n;
                    cout<<curr<<endl;
                }
            }
            else
            {
                string res="sending failed";
                cout<<"aborting to send file as file not present"<<endl;
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

void gettokens(vector<string>&tokens,string details)
{

    istringstream ss(details);
  
    string word; // for storing each word
  
    // Traverse through all words
    // while loop till we get 
    // strings to store in string word
    while (ss >> word) 
    {
        // print the read word
        //cout << word << "\n";
        tokens.push_back(word);
    }

}

void copy_string(string &a, string b)
{
    for (char i:b)
    {
        a.push_back(i);
    }
}

void printall(vector<string> tokens,int index)
{
    for(int i=index;i<tokens.size();i++)
    {
        cout<<i<<" "<<tokens[i]<<endl; 
    }
}

bool checkfile(string filePath)
{
    int fd2;

    if ((fd2=open(filePath.c_str(),O_RDWR))<0)
    {
        cout<<"Unable to find file"<<endl;
        return false;
    }
    else
    {
        cout<<"File available"<<endl;
    }

    close(fd2);
    return true;


}

string getfilename(string path)
{
    size_t botDirPos = path.find_last_of("/");

    string file = path.substr(botDirPos+1, path.length());
    return file;
}

int create_file(string path)
{

    int fd2;

    if ((fd2=open(path.c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
    {
        cout<<"creat error"<<endl;
        return -1;
    }

    return fd2;    

}