#include "allheaders.h"

using namespace std;

bool copy(string,string);
int deleteFile(string);
string extract_name(string);

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

		if(deleteFile(dirName+"/"+dname));
	}
	closedir(di);
	return;
}

int deleteFile(string file)
{
	struct stat fileInfo;
	lstat(file.c_str(),&fileInfo);
	if(S_ISDIR(fileInfo.st_mode))
	{
		deleteDir(file);
		rmdir(file.c_str());
		return 1;
	}
	else
	{
		if(unlink(file.c_str())==0)
			return 1;
		else
			return 0;
	}

	return 0;

}


bool create_file(string src,string dest)
{
	int fd2;
	string str = dest+'/'+src;
	if ((fd2=open((str).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
		return false;
	close(fd2);
	return true;

}

bool create_dir(string src,string dest)
{
	string name = extract_name(src);
	string full = dest+'/'+name;
	
	try
	{
		if(mkdir(full.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)==0)
			return true;
		cout<<strerror(errno);
		return false;
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
	            	lstat((tmp+"/"+string(diread->d_name)).c_str(),&binfo);
	            	if(S_ISDIR(binfo.st_mode))
	            		if(!(stname==".." or stname=="."))
	            		{
	            			//cout<<"adding this directory "<<stname<<endl;
	            			qt.push(tmp+"/"+string(diread->d_name));
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

string extract_name(string src)
{
	string roll="";
    int i=0;
    while(i<src.size())
    {
        if(src[i]=='/')
        {
            roll="";
        }
        else
        {
            roll.push_back(src[i]);
        }
        i++;

    }
    return roll;
}

int copy_file(string src,string dest)
{

	int n;
	char buf[buffer_size];
	int fd1;
	int fd2;

	string name=extract_name(src);

	if ((fd1=open(src.c_str(),O_RDWR))<0)
		cout<<"creat error"<<endl;
	if ((fd2=open((dest+"/"+name).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR))<0)
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

bool copy_dir_helper(string dirName, string destination){

	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(dirName.c_str()))){
		cout<<"Can't open the directory";
		return true;
	}

	while((diren = readdir(di))){
		string dname =  string(diren->d_name);
		if( (dname == ".") || (dname == "..") ){
				continue;
		}

		if(copy(dirName+'/'+dname,destination))
			continue;
		else
			return false;
	}
	closedir(di);
	return true;
}

bool copy(string src,string dest)
{

	struct stat fileInfo;
	lstat(src.c_str(),&fileInfo);
	if(S_ISDIR(fileInfo.st_mode))
	{
		create_dir(src,dest);
		return copy_dir_helper(src,dest+"/"+extract_name(src));

	}
	else
	{
		return copy_file(src,dest);
		
	}

	return false;


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

