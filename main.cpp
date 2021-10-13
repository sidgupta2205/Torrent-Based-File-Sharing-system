#include "allheaders.h"
#include "cmdFunctions.h"
using namespace std;

string getcurrstring();
struct termios original;
struct winsize w;
int rowsize;
int colsize;
int posc=0;
int cmdr;
unsigned int originval;
vector<char *> files;
vector<string> curr; // Holds curr directory path 
vector<string>cmds;
// changing terminal mode to noncanonical

void moveCursor(int r,int col)
{
    posc = r;
    cout<<"\033["<<r<<";"<<col<<"H";
    fflush(stdout);
}

void setNonCanonicalMode()
{
	tcgetattr(STDIN_FILENO,&original);
    cout<<original.c_lflag<<endl;
    struct termios newo = original;
	newo.c_lflag &= ~(ECHO | ICANON);
    //newo.c_oflag &= ~(OPOST);
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &newo) != 0) {
		cout<<"operation failed for switching to non canonical Mode"<<endl;
	}

    cout<<original.c_lflag<<endl;

}

void setCanonicalMode()
{
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original) != 0) {
        cout<<"cannot move back to canonical mode"<<endl;
    }
}

vector<string> spaceSeperated(string str)
{
    vector<string>st;
    string roll="";

    for (char i:str)
    {
        if(i==' ')
        {
            if(roll!="")
            {
                st.push_back(roll);
                roll = "";
            }
        }
        else
        {
            roll.push_back(i);
        }
    }

    if(roll!="")
        st.push_back(roll);

    return st;
}

void processCmds(string roll)
{
    vector<string>cmds;
    cmds = spaceSeperated(roll);
    string cmd = cmds[0];
    if(cmd=="copy")
    {

        string dest = cmds[cmds.size()-1];
        for(int i=1;i<cmds.size()-1;i++)
        {
            copy_file(cmds[i],dest);
        }
        
    }

    else if(cmd=="move")
    {
        string dest = cmds[cmds.size()-1];
        for(int i=1;i<cmds.size()-1;i++)
        {
            if(copy_file(cmds[i],dest));
                deleteFile(cmds[i]);
        }

    }

    else if(cmd=="rename")
    {
        renameFile(cmds[1],cmds[2]);
    }

    cmds.clear();
}




void movetocmdmode()
{
    moveCursor(cmdr,1);
    int curcol=1;
    //setCanonicalMode();
    char ch='r';
    string roll="";
    while(ch!=27)
    {
        ch = cin.get();
        if(ch==10)
        {
            processCmds(roll);
            cout << clrline;
            roll="";
            moveCursor(cmdr,0);
        }
        else if(ch==127)
        {
            if(curcol>1)
            {
                curcol--;
                moveCursor(cmdr,0);
                roll.pop_back();
                cout<<clrline;
                cout<<roll;
            }

        }
        else
        {
            roll.push_back(ch);
            curcol++;
            cout<<ch;
        }
        
    }
    //change to canonical mode
    //read Input    
}


void setWindowParams()
{
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
    rowsize=w.ws_row;
    colsize=w.ws_col;
    cmdr = rowsize-5;
}
//intializing welcome screen
void initialize()
{
	setNonCanonicalMode();
    setWindowParams();
}

void printfileofdirs(string tmp)
{
    files.clear();
    DIR *dir;
    struct dirent *diread;
    vector<int> file_size;
    int len;
    //cout << "\033[H\033[2J\033[3J" ;
    if ((dir = opendir(tmp.c_str())) != nullptr) {
        while ((diread = readdir(dir)) != nullptr) {
            files.push_back(diread->d_name);
        }
        closedir (dir);
    } else {
        perror ("opendir");
        return ;
    }
   
    len = files.size();
    for(int i=0;i<len;i++)
    {
	    struct stat binfo;
	    lstat(files[i],&binfo);
	    cout<<files[i]<<"  "<<binfo.st_size<<endl; 
    }

    moveCursor(1,0);

}

void scrollDown()
{
    cout<<"\033["<<files.size()<<";"<<cmdr<<"H";
    if(posc<=files.size())
    {
        moveCursor(posc+1,0);
    }

}

void scrollUp()
{
    if(posc>0)
    {

        moveCursor(posc-1,0);
    }

}

void create_string(char tmp[])
{
    string temp = string(tmp);
    string roll="";
    int i=0;
    while(i<temp.size())
    {
        if(temp[i]=='/')
        {
            if(roll!="")
                curr.push_back(roll);
            roll="";
        }
        else
        {
            roll.push_back(temp[i]);
        }
        i++;

    }
}

string getcurrstring()
{
    if(curr.size()==0)
        return "/";

    string curr_str = "";
    for(int i=0;i<curr.size();i++)
    {
        curr_str = curr_str+"/"+curr[i];
    }

    return curr_str;

}

void movetoparent()
{
    if(curr.size()) curr.pop_back();
    string f = getcurrstring();
    printfileofdirs(f);
}

void movetodir(string name)
{
    curr.push_back(name);
    string f = getcurrstring();
    printfileofdirs(f);
}

void enter()
{
    cout<<clr;
    //cout<<files[posc-1]<<endl;
    if(string(files[posc-1])==".")
    {
        //cout<<"this is the same place"<<endl;
        printfileofdirs(getcurrstring());
        return;
    }
    if(string(files[posc-1])=="..") 
        movetoparent(); 
    else
        movetodir(string(files[posc-1]));
    moveCursor(1,0);
}

int main() {
    initialize();
    char tmp[PATH_MAX];
    getcwd(tmp, PATH_MAX);
    create_string(tmp);
    //cout<<string(tmp)<<endl;
    cout << clr;
    printfileofdirs(getcurrstring());
    //moveCursor(1,0);
    char c=' ';
    
    while(c!='q')
    {
        c=cin.get();
        switch(c)
        {
            case ':':movetocmdmode();break;

            case 10:enter();break;

            //case 127:levelUp();break;

            case 65:scrollUp();break;

            //case 68:moveBack();break;

            //case 107:scrollUpK();break;

            //case 104:home();break;

            case 66:scrollDown();break;

            //case 67:moveForward();break;

            //case 108:scrollDownL();break;

            case 'c':cout<<copy_file("file.txt","filec.txt")<<endl;break;
            
            default:break;
        }
    }

    //cout<<clr;

    
    atexit(setCanonicalMode);
    return 0; 
}