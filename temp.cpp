#include "allheaders.h"
using namespace std;
int main()
{
	string dir_name = "../hello";
	mkdir(dir_name.c_str(),S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	return 0;

}
