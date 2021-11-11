#include "allheaders.h"
using namespace std;
// Tracker
// - Start listening with a port

//current group id
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

struct Group
{
    string groupid;
    string owner;
    vector<string> group_users;
    map<string,filesInfo> files;
    vector<string> user_requests;
};

struct sockaddr_in serverAddr;
struct sockaddr_storage serverStorage;
char cmds[1024];
socklen_t addr_size;
pthread_t rthreads[100];
map<int,int>data;
map<string,User> users;
map<string,Group> groups;



map<string,filesInfo>files;
void* user_thread(void*);
void gettokens(vector<string>&,string);
string getall(vector<string>);
string getallgroups();
string getallfiles(string);

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
    string user_id = "";
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
        else if(tokens.size()>0 and tokens[0]=="login")
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
                    user_id = tokens[1];
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

        else if(tokens[0]=="logout")
        {
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                string res = "user logged out success";
                user_id = "";
                cout<<"user logged out success"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
        }

        else if(tokens.size()>0 and tokens[0]=="create_group")
        {
            if(user_id=="")
            {
                cout<<"User not logged in"<<endl;
                string res = "Group creation failed";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);

            }

            else if(groups.find(tokens[1])!=groups.end())
            {
                string res = "Group creation failed";
                cout<<"Group user id already taken"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                Group g;
                g.groupid = tokens[1];
                g.owner = user_id;
                g.group_users.push_back(user_id);
                groups[tokens[1]] = g;
                string res = "Group creation success";
                send(fd,res.c_str(),res.size(),0);
                cout<<res<<"by "<<user_id<<endl;
            }
        }

        else if(tokens.size()>0 and tokens[0]=="list_groups")
        {
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                string res = getallgroups();
                res = "group_details " + res;
                cout<<"sending group details "<<res;
                send(fd,res.c_str(),res.size(),0);
            }
            
        }

        else if(tokens[0]=="join_group")
        {
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups.find(tokens[1])==groups.end())
            {
                string res="Unable to join group";
                cout<<"Group not found"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                auto it = find(groups[tokens[1]].group_users.begin(),groups[tokens[1]].group_users.end(),user_id);
                auto it2 = find(groups[tokens[1]].user_requests.begin(),groups[tokens[1]].user_requests.end(),user_id);
                if(it!=groups[tokens[1]].group_users.end())
                {
                    string res = "User Already Present";
                    cout<<res<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else if(it2!=groups[tokens[1]].user_requests.end())
                {
                    string res = "User request Already Present";
                    cout<<res<<endl;
                    send(fd,res.c_str(),res.size(),0);

                }
                else
                {

                    string res = "User Request Registered with group";
                    cout<<res<<endl;
                    groups[tokens[1]].user_requests.push_back(user_id);
                    send(fd,res.c_str(),res.size(),0);

                }
            }
            
        }

        else if(tokens[0]=="requests")
        {
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups.find(tokens[2])==groups.end())
            {
                string res="Unable to join group";
                cout<<"Group not found"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups[tokens[2]].owner!=user_id)
            {
                string res="Permission denied to user";
                cout<<"Requested user not group owner"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else 
            {
                string res = getall(groups[tokens[2]].user_requests);
                cout<<"Sending user request "<<endl;
                res = "request_list "+res;
                send(fd,res.c_str(),res.size(),0);
            }
            
        }
        else if(tokens[0]=="accept_request")
        {
            
            //token 1 group id
            //token 2 user id

            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups.find(tokens[1])==groups.end())
            {
                string res="Unable to join group";
                cout<<"Group not found"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups[tokens[1]].owner!=user_id)
            {
                string res="Permission denied to user";
                cout<<"Requested user not group owner"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                auto it2 = find(groups[tokens[1]].user_requests.begin(),groups[tokens[1]].user_requests.end(),tokens[2]);
                if(it2==groups[tokens[1]].user_requests.end())
                {
                    string res="User has not requested";
                    cout<<res<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else
                {
                    groups[tokens[1]].user_requests.erase(remove(groups[tokens[1]].user_requests.begin(), groups[tokens[1]].user_requests.end(), tokens[2]), groups[tokens[1]].user_requests.end());
                    groups[tokens[1]].group_users.push_back(tokens[2]);
                    string res="User added in group";
                    cout<<"User has been successfully added to group"<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
            }

        }

        //upload_file path group_id
        else if(tokens[0]=="upload_file")
        {
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups.find(tokens[2])==groups.end())
            {
                string res="Unable to join group";
                cout<<"Group not found"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                auto it2 = find(groups[tokens[2]].group_users.begin(),groups[tokens[2]].group_users.end(),user_id);
                if(it2==groups[tokens[2]].user_requests.end())
                {
                    string res="Invalid request";
                    cout<<"User does not exist in group"<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else
                {
                    if(groups[tokens[2]].files.find(tokens[1])==groups[tokens[2]].files.end())
                    {
                        //create a filesInfo
                        filesInfo f;
                        f.name = tokens[1];
                        f.portno = to_string(port_no);
                        string gcmd = "getfilesize2 "+f.name;
                        send(fd,gcmd.c_str(),sizeof(gcmd),0);
                        memset(message, 0, 1024);
                        read(fd,message,1024);
                        string res = string(message);
                        f.size = res;
                        cout<<f.name<<" with "<<f.size<<" is available to download "<<endl;
                        groups[tokens[2]].files[f.name] = f;
                    }
                    else
                    {
                        string sp = " ";
                        groups[tokens[2]].files[tokens[1]].portno = groups[tokens[2]].files[tokens[1]].portno+sp+to_string(port_no);
                        cout<<groups[tokens[2]].files[tokens[1]].portno;
                        cout<<"port no added for file"<<endl;
                    }

                    string res = "File uploaded success";
                    send(fd,res.c_str(),res.size(),0);

                }
            }

        }
        else if(tokens[0]=="list_files")
        {
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups.find(tokens[1])==groups.end())
            {
                string res="Unable to join group";
                cout<<"Group not found"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                auto it2 = find(groups[tokens[1]].group_users.begin(),groups[tokens[1]].group_users.end(),user_id);
                if(it2==groups[tokens[1]].user_requests.end())
                {
                    string res="Invalid request";
                    cout<<"User does not exist in group"<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else
                {
                    string res = getallfiles(tokens[1]);
                    res = "list_file "+res;
                    send(fd,res.c_str(),res.size(),0);
                }
            }
        }
        //download_file <group_id> <file_name> <destination_path>
        else if(tokens[0]=="download_files")
        {
            
            if(user_id=="")
            {
                string res="Not Logged In";
                cout<<res<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else if(groups.find(tokens[1])==groups.end())
            {
                string res="Unable to join group";
                cout<<"Group not found"<<endl;
                send(fd,res.c_str(),res.size(),0);
            }
            else
            {
                auto it2 = find(groups[tokens[1]].group_users.begin(),groups[tokens[1]].group_users.end(),user_id);
                if(it2==groups[tokens[1]].user_requests.end())
                {
                    string res="Invalid request";
                    cout<<"User does not exist in group"<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else if(groups[tokens[1]].files.find(tokens[2])==groups[tokens[1]].files.end())
                {
                    string res="File not Found";
                    cout<<"Requested File is not present in the group"<<endl;
                    send(fd,res.c_str(),res.size(),0);
                }
                else
                {
                    string res = "file_info "+groups[tokens[1]].files[tokens[2]].size+" "+groups[tokens[1]].files[tokens[2]].portno;
                    send(fd,res.c_str(),res.size(),0);
                    cout<<res<<endl;
                }
            }

            // login
            // group
            // member
            // file present
            // sent size and port no's    
        }


        sleep(0.1);

    }

    pthread_exit(NULL);

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

string getallgroups()
{
    string res="";
    for ( auto it = groups.begin(); it != groups.end(); ++it  )
    {
        res = res+it->first+" ";   
    }

    if(res!="")
        res.pop_back();
    
    return res;
}

string getall(vector<string> users)
{
    string res="";

    for (string usr:users)
    {
        res = res+usr+" ";
    }
    if(res!="")
        res.pop_back();

    return res; 

}

string getallfiles(string group_id)
{
    
    string res="";
    for ( auto it = groups[group_id].files.begin(); it != groups[group_id].files.end(); ++it  )
    {
        res = res+it->first+" ";   
    }

    if(res!="")
        res.pop_back();
    
    return res;
}