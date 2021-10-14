#include "allheaders.h"

using namespace std;

void copy(string,string);


void deleteDir(string dirName)
{
	DIR *di;
	struct dirent *diren;

	if(!(di = opendir(dirName.c_str()))){
		cout<<"Can't open the directory";
		return;
	}

	while((diren = readdir(di))){
		string dname =  string(diren->d_name);
		if( (dname == ".") || (dname == "..") ){
				continue;
		}

		deleteFile(dirName+"/"+dname);
	}
	closedir(di);
	return;
}

int deleteFile(string file)
{
	struct stat fileInfo;
	lstat(file.c_str(),fileInfo);
	if(S_ISDIR(fileInfo.st_mode))
	{
		deleteDir(file);
		rmdir(file.c_str());
	}
	else
	{
		return unlink(file.c_str());
	}

}

bool create_dir(string name,string dest)
{
	string full = dest+"/"+name;
	
	try
	{
		mkdir(full.c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
		return true;
	}
	catch(exception &e)
	{
		return false;
	}
	
	return false;

}

int renameFile(string file,string nr)
{
	return rename(file.c_str(),nr.c_str());
}

bool searchFile(string curr_dir,string name)
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
	            			//cout<<"adding this directory "<<stname<<endl;
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
			//cout<<strerror(errno);
			return 0;
		}
	if(n<0)
	{
		//cout<<strerror(errno);
		return 0;
	}
	close(fd1);
	close(fd2);

	return 1;

}

void copy_dir_helper(string dirName, string destination){

	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(dirName.c_str()))){
		cout<<"Can't open the directory";
		return;
	}

	while((diren = readdir(di))){
		string dname =  string(diren->d_name);
		if( (dname == ".") || (dname == "..") ){
				continue;
		}

		copy(dname,destination);
	}
	closedir(di);
	return;
}

void copy(string src,string dest)
{

	struct stat fileInfo;
	lstat(src.c_str(),&fileInfo);
	if(S_ISDIR(fileInfo.st_mode))
	{
		create_dir(src,dest);
		copy_dir_helper(src,dest+"/"+src);
	}
	else
	{
		copy_file(src,dest);
	}

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

