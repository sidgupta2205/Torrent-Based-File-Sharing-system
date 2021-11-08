#include "allheaders.h"
using namespace std;
// Tracker
// - Start listening with a port

int serverSocket, newSocket;

struct filesInfo
{
    string portno;
    string name;
    string size;
};

struct User
{
    string portno;
    string user_id;
    string passw;
    int fd;
};

struct sockaddr_in serverAddr;
struct sockaddr_storage serverStorage;
char cmds[1024];
socklen_t addr_size;
pthread_t rthreads[100];
map<int,int>data;
map<string,User> users;



map<string,filesInfo>files;
void* user_thread(void*);
void gettokens(vector<string>&,string);

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
        vector<string>tokens;
        string tokenize = cmd;
        gettokens(tokens,tokenize);
        
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
            string fnam = string(message);
            
            if(files.find(fnam)==files.end())
            {
                filesInfo ff;
                ff.name = fnam;
                ff.portno = to_string(port_no);
                cmd = "getfilesize";
                send(fd,cmd.c_str(),sizeof(cmd),0);
                memset(message, 0, 1024);
                read(fd,message,1024);
                string res = string(message);
                if(res=="filename")
                {
                    send(fd,fnam.c_str(),fnam.size(),0);
                }
                memset(message, 0, 1024);
                read(fd,message,1024);
                ff.size = string(message);
                files[fnam] = ff;
                cout<<"file size recev id "<<ff.size<<endl;
            }
            else
            {
                files[fnam].portno = files[fnam].portno+" "+to_string(port_no);
            }

            cout<<"adding file "<<fnam<<" "<<files[fnam].portno<<endl;
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
                cout<<"file found at port "<<files[file_name].portno<<endl;
                string res = files[file_name].portno;
                string siz = files[file_name].size;
                send(fd,res.c_str(),sizeof(res),0);
                cout<<"file size sent to server "<<siz<<endl;
                send(fd,siz.c_str(),siz.size(),0);
                
            }
        }
        else if(tokens[0]=="create_user")
        {
            cout<<"command recv is "<<cmd<<endl;

            if(users.find(tokens[1])!=users.end())
            {
                string res = "Registration failed";
                send(fd,res.c_str(),res.size(),0);
                cout<<"User Already Exist"<<endl;
                continue;

            }

            User user;
            //check if user_id does not exist previously
            user.user_id = tokens[1];
            user.passw = tokens[2];
            user.portno = to_string(port_no);
            user.fd = fd;

            users[user.user_id] = user;

            string res = "Registration success";
            send(fd,res.c_str(),res.size(),0);
            cout<<"User Registration successfull"<<endl;

        }
        else if(tokens[0]=="login")
        {
            cout<<"command recvd is "<<cmd<<endl;
            if(users.find(tokens[1])==users.end())
            {
                string res="Login Failure";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                if(users[tokens[1]].passw == tokens[2])
                {
                    string res="Login Success";
                    cout<<res<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else
                {
                    string res="Login Failure";
                    cout<<res<<endl;
                    send(fd,res.c_str(),res.size(),0);

                }
            }
        }

        sleep(0.1);

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

