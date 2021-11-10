#include "allheaders.h"
using namespace std;

int main()
{
    string path = "file.pdf";
    size_t botDirPos = path.find_last_of("/");

    string file = path.substr(botDirPos+1, path.length());
    cout<<file<<endl;
}
