#include "allheaders.h"
using namespace std;
int main()
{
	struct stat bin;
	DIR *dir;
	struct dirent* diread;
	string tmp = ".";
	if ((dir = opendir(tmp.c_str())) != nullptr) {
	        while ((diread = readdir(dir)) != nullptr) {
	            lstat(diread->d_name,&bin);
	            if(S_ISDIR(bin.st_mode))
	            	cout<<diread->d_name<<" directory"<<endl;
	           	else
	           		cout<<diread->d_name<<" not "<<endl;
	        }
	        closedir (dir);
	    } else {
	        perror ("opendir");
	        return 0 ;
	    }
	return 0;
}