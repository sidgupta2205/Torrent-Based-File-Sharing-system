#include "allheaders.h"

using namespace std;

int deleteFile(string file)
{
	return unlink(file.c_str());
}

int renameFile(string file,string nr)
{
	return rename(file.c_str(),nr.c_str());
}

int searchFile(string curr_dir,string name)
{
	// add current directory in queue
	queue<string> qt;
	qt.push(curr_dir);
	
	while(!qt.empty())
	{
		string tmp = qt.front();
		qt.pop();
		DIR *dir;
	    struct dirent *diread;
	    struct stat binfo;
	    //cout << "\033[H\033[2J\033[3J" ;
	    if ((dir = opendir(tmp.c_str())) != nullptr) {

	        while ((diread = readdir(dir)) != nullptr) {
	            if(string(diread->d_name)==name)
	            	return true;
	            else
	            {
	            	string stname = string(diread->d_name);
	            	lstat(diread->d_name,&binfo);
	            	if(S_ISDIR(binfo.st_mode))
	            		if(!(stname==".." or stname=="."))
	            		{
	            			cout<<"adding this directory "<<stname<<endl;
	            			qt.push(stname);
	            		}
	            }
	        }
	        closedir (dir);
	    } else {
	        //perror ("opendir");
	        //cout<<"error occured "<<tmp<<endl;
	        continue;
	    }
	}
   
   return false;

	// search for all files in directory
	// add directory in queue
	// if file is found return true
	// if queue is empty
	// return false

}

int copy_file(string src,string dest)
{

	int n;
	char buf[buffer_size];
	int fd1;
	int fd2;

	

	if ((fd1=open(src.c_str(),O_RDWR))<0)
		cout<<"creat error"<<endl;
	if ((fd2=open((dest+"/"+src).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
		cout<<"creat error"<<endl;




	while((n=read(fd1,buf,buffer_size))>0)
		if(write(fd2,buf,n)!=n)
		{
			cout<<strerror(errno);
			return 0;
		}
	if(n<0)
	{
		cout<<strerror(errno);
		return 0;
	}
	close(fd1);
	close(fd2);

	return 1;

}

// int main()
// {

// 	char tmp[PATH_MAX];
//     getcwd(tmp, PATH_MAX);
	
// 	if(copy_file("file.txt","filec.txt"))
// 		cout<<"copy_success"<<endl;

// 	if(deleteFile("filec.txt")==0)
// 		cout<<"file successful delete"<<endl;

// 	if(renameFile("file.txt")==0)
// 		cout<<"file rename successful"<<endl;

// 	if(searchFile(string(tmp),"file9.c"))
// 		cout<<"found file"<<endl;
// 	else
// 		cout<<"not found"<<endl;

// 	exit(0);

// 	return 0;
// }

