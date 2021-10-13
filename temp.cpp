#include "allheaders.h"
using namespace std;
int main()
{
	char buf1[] = "abcdefghij";
	char buf2[] = "gupta";
	int fd;
	
	if ((fd=open("file.txt",O_RDWR))<0)
		cout<<"creat error"<<endl;
	if(write(fd,buf1,10) ==-1)
		cout<<strerror(errno);
	return 0;

}
