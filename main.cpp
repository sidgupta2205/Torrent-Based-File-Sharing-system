#include "allheaders.h"
#include "cmdFunctions.h"
using namespace std;


string getcurrstring();
string createAbs(string);
void display();
void moveCursor(int,int);
void moveWithout(int,int);
struct termios original;
struct winsize w;
int rowsize;
int colsize;
int len;
int posc=1;
int cursor;
int cmdr;
int top = 0;
int bottom=0;
int currpos;
int mode=1;
unsigned int originval;
vector<char *> files;
vector<string>toprint;
vector<string> curr; // Holds curr directory path 
vector<string>cmds;
stack<string>back;
stack<string>front;
// changing terminal mode to noncanonical



void printmodeLine()
{
    int temp = cursor;
    if(mode==1)
    {
        
        moveWithout(cmdr-1,0);
        cout<<clrline;
        cout<<"------NORMAL MODE ACTIVATED-------";

    }
    else
    {
        moveWithout(cmdr-1,0);
        cout<<clrline;
        cout<<"------COMMAND MODE ACTIVATED-------";

    }

    moveWithout(temp,0);

    return;
}

string createAbs(string pathd)
{
    if(pathd[0]=='/')
        return pathd;
    else
        return getcurrstring()+"/"+pathd;
}

void moveCursor(int r,int col)
{
    posc = r;
    cursor = r;
    cout<<"\033["<<r<<";"<<col<<"H";
    fflush(stdout);
}

void moveWithout(int r,int col)
{
    cursor = r;
    cout<<"\033["<<r<<";"<<col<<"H";
    fflush(stdout);   
}

void displayResult(string str)
{
    moveCursor(cmdr-2,0);
    cout<<str;
    moveCursor(cmdr,0);
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



void setWindowParams()
{
    ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
    rowsize=w.ws_row;
    colsize=w.ws_col;
    cmdr = rowsize-5;
    rowsize =cmdr-3;
}
//intializing welcome screen
void initialize()
{
	setNonCanonicalMode();
    setWindowParams();
    mode=1;
}

string getpermissions(struct stat fileInfo)
{
    string per_str="----------";
    if(S_ISDIR(fileInfo.st_mode))
        per_str[0]='d';
    else if(S_ISSOCK(fileInfo.st_mode))
        per_str[0]='s';


    if(S_IRUSR & fileInfo.st_mode)
        per_str[1]='r';
    if (S_IWUSR & fileInfo.st_mode)
        per_str[2]='w';
    if(S_IXUSR & fileInfo.st_mode)
        per_str[3]='x';

    if(S_IRGRP & fileInfo.st_mode)
        per_str[4]='r';
    if(S_IWGRP & fileInfo.st_mode)
        per_str[5]='w';
    if(S_IXGRP & fileInfo.st_mode)
        per_str[6]='x';

    if(S_IROTH & fileInfo.st_mode)
        per_str[7]='r';
    if(S_IWOTH & fileInfo.st_mode)
        per_str[8]='w';
    if(S_IXOTH & fileInfo.st_mode)
        per_str[9]='x';

    return per_str;

}

string getreadablesize(ll size)
{
    if(size>=GB_SIZE)
    {
        size = size/1073741824;
        return to_string(size)+"GB";
    }
    else if(size>=MB_SIZE)
    {
        size = size/MB_SIZE;
        return to_string(size)+"MB";
    }
    else if(size>=KB_SIZE)
    {
        size = size/KB_SIZE;
        return to_string(size)+"KB";
    }

    return to_string(size)+"B";

}

void printfileofdirs(string tmp)
{
    struct stat bin;
    lstat(tmp.c_str(),&bin);
    if(!(S_ISDIR(bin.st_mode)))
        return;
    files.clear();
    toprint.clear();
    DIR *dir;
    struct dirent *diread;
    vector<int> file_size;
    //cout << "\033[H\033[2J\033[3J" ;
    if ((dir = opendir(tmp.c_str())) != nullptr) {
        while ((diread = readdir(dir)) != nullptr) {
            files.push_back(diread->d_name);
            toprint.push_back(string(diread->d_name));
        }
        closedir (dir);
    } else {
        perror ("opendir");
        return ;
    }
   
    len = files.size();
    top = 0;
    bottom = min(len,rowsize);
    display();
    moveCursor(1,0);

}

void display()
{
    cout<<clr;
    moveWithout(1,0);
    for(int i=top;i<bottom;i++)
    {
        struct stat binfo;
        char date[100];
        lstat(files[i],&binfo);
        time_t t= binfo.st_mtime;
        string sized = getreadablesize(binfo.st_size);
        string per_str = getpermissions(binfo);
        strftime(date, 20, "%d-%m-%y %I:%M%p.", localtime(&(t)));
        fflush(stdout);
        cout<<toprint[i]<<"\t\t"<<sized<<"\t\t"<<per_str<<"\t\t"<<date<<"\n"; 
    }

    printmodeLine();

}

void scrollDown()
{
    //cout<<"\033["<<files.size()<<";"<<cmdr<<"H";
    if(posc==files.size())
        return;
    if(posc<bottom)
    {
        ++posc;
        moveWithout(cursor+1,0);
        return;
    }
    top++;
    posc++;
    bottom = min(bottom+1,len);
    display();
    moveWithout(rowsize,0);

}

void scrollUp()
{
    if(posc==1)
        return;
    if(posc>top)
    {
        --posc;
        moveWithout(cursor-1,0);
        return;
    }
    top--;
    bottom--;
    display();
    moveWithout(1,0);    

}

void create_string(string temp)
{
    string roll="";
    int i=0;
    curr.clear();
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
    if(roll!="")
        curr.push_back(roll);
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
    //navigation[++currpos]=f;
    back.push(f);
    printfileofdirs(f);
}

void movetodir(string name)
{
    back.push(getcurrstring());
    curr.push_back(name);
    string f = getcurrstring();
    printfileofdirs(f);
}

void moveTofront()
{
    if(!front.empty())
    {
        back.push(front.top());
        front.pop();
        create_string(back.top());
        cout<<clr;
        printfileofdirs(back.top());
    }

}

void moveToBack()
{
    if(!back.empty())
    {
        front.push(back.top());
        back.pop();
        create_string(front.top());
        cout<<clr;
        printfileofdirs(front.top());
    }
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
            copy(cmds[i],dest);
        }
        
    }

    else if(cmd=="move")
    {
        string dest = cmds[cmds.size()-1];
        for(int i=1;i<cmds.size()-1;i++)
        {
            if(copy(createAbs(cmds[i]),createAbs(dest)))
                deleteFile(createAbs(cmds[i]));
        }

    }

    else if(cmd=="delete")
    {
        for(int i=1;i<cmds.size();i++)
        {
            if(cmds[i]==getcurrstring())
                return;
            deleteFile(cmds[i]);
        }
        
    }

    else if(cmd=="rename")
    {
        renameFile(cmds[1],cmds[2]);
    }
    else if(cmd=="goto")
    {
        
        if(cmds[1]=="..")
        {
            cout<<clr;
            moveCursor(1,0);
            movetoparent();
        }
        else if(cmds[1]==".")
        {

        }
        else
        {   
            cout<<clr;
            moveCursor(1,0);
            movetodir(cmds[1]);
        }

        moveCursor(cmdr,0);

    }
    else if(cmd=="search")
    {
        bool res = searchFile(getcurrstring(),cmds[1]);
        moveCursor(cmdr-2,0);
        
        if(res)
        {
            cout<<"present";    
        }
        else
        {
            cout<<"Not present";
        }

        moveCursor(cmdr,0);

    }
    else if(cmd=="create_dir")
    {
        string dest = cmds[cmds.size()-1];
        for(int i=1;i<cmds.size()-1;i++)
        {
            if(create_dir(cmds[i],getcurrstring()+'/'+dest))
                displayResult("Directory Created");
            else
                displayResult("Creation failed");
        }   
    }
    else if(cmd=="create_file")
    {
        string dest = cmds[cmds.size()-1];
        for(int i=1;i<cmds.size()-1;i++)
        {
            if(create_file(cmds[i],getcurrstring()+'/'+dest))
                displayResult("File Created");
            else
                displayResult("File Creation failed");
        }

    }

    cmds.clear();
}

void movetocmdmode()
{
    mode=0;
    printmodeLine();
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

    moveCursor(1,0);
    mode=1;
    printfileofdirs(getcurrstring());
    printmodeLine();
    return;
    //change to canonical mode
    //read Input    
}

void printtContent()
{
    moveCursor(cmdr,0);
    
    cout<<endl;
    for(string i : curr)
    {
        cout<< i<<" ";
    }
    cout<<getcurrstring()<<endl;

}

void home()
{
    create_string(HOME);
    movetodir(getcurrstring());
}

void do_resize(int dummy)
{
    setWindowParams();
    printfileofdirs(getcurrstring());
    printmodeLine();
}

int main() {
    signal(SIGWINCH, do_resize);
    initialize();
    char tmp[PATH_MAX];
    getcwd(tmp, PATH_MAX);
    create_string(string(tmp));
    //cout<<string(tmp)<<endl;
    cout << clr;
    printfileofdirs(getcurrstring());
    printmodeLine();
    //moveCursor(1,0);
    char c=' ';
    
    while(c!='q')
    {
        c=cin.get();
        switch(c)
        {
            case ':':movetocmdmode();break;

            case 'p':printtContent();break;

            case 10:enter();break;

            case 127:cout<<clr;movetoparent();break;

            case 65:scrollUp();break;

            case 68:moveToBack();break;

            //case 107:scrollUpK();break;

            case 104:home();break;

            case 66:scrollDown();break;

            case 67:moveTofront();break;

            //case 108:scrollDownL();break;

            
            
            default:break;
        }
    }

    //cout<<clr;

    
    atexit(setCanonicalMode);
    return 0; 
}