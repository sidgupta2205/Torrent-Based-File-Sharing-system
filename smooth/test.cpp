#include<bits/stdc++.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<grp.h>
#include<pwd.h>
#include<termios.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include <sys/stat.h>
#include <ctime>
#define clr() printf("\033[H\033[J")
#define clearLine() printf("\033[K")
#define MAX 5											// ------------> Change this to resize display window for ls command
using namespace std;

/******************************************* GLOBAL VARIABLES *******************************************/

struct termios orig_termios; 							// ------------> For initialisation of Normal mode
string rootPath;
size_t cwdSize=1024;
char cwd[1024];
struct winsize w;  										// ------------> Gives terminal window size in terms of rows and columns

int cursorLoc = 18;
int top=0;
int bottom=top+MAX;
int cursor=1;
int hzCentre;
int col;

int commandLine;
int alertLine;
int statusLine;


vector<dirent*> fileNames;
vector<string> commandTokens;
stack<string> backS;
stack<string> forwardS;



/******************************************** HELPER FUNCTIONS ***********************************************/

void moveCursor(int x,int y) {
	cout<<"\033["<<x<<";"<<y<<"H";
	fflush(stdout);
}

void disableNormalMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void printAlertLine(string s){
	moveCursor(alertLine,0);
	cout<<"\033[1;31m"<<"ALERT : "<<s<<"\033[0m";
	moveCursor(cursor, 0);
}

void enableNormalMode() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON);
	if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) != 0) {
		printAlertLine("Unable to switch to Normal Mode                        ");
	}
}

void init(char * cwd){
    enableNormalMode();
	rootPath = string(cwd);
	ioctl(STDOUT_FILENO,TIOCGWINSZ,&w);
	commandLine=w.ws_row-8;
	col = w.ws_col;
	hzCentre = col/2;
    top = 0;
    bottom = top + MAX;
    cursor = 0;
	statusLine = commandLine + 2;
	alertLine=MAX+4;
}

void printStatusLine(string s){
	moveCursor(statusLine,0);
	cout<<"\033[1;32m"<<"STATUS : "<<s<<"\033[0m";
	moveCursor(commandLine, 18);
}

int noOfFiles(){
    return fileNames.size();
}

void resetPointers(){
	top = 0;
	bottom = min(top+MAX, noOfFiles());
	cursor = 0;
}

void printCWD(){
	moveCursor(alertLine+2,0);
	cout<<"\033[1;35m"<<"Root : "<<rootPath<<"\033[0m"<<endl;
	cout<<"\033[1;35m"<<"Current Working Directory : " <<cwd<<"\033[0m"<<endl;
	moveCursor(cursor, 0);
}

void printNormalMode(){
	moveCursor(commandLine-2,0);
	for(int i=0; i<hzCentre-12; i++) cout<<"-";
	moveCursor(commandLine-2,hzCentre-9);
	cout<<"\033[4;36m"<<"Mode : Normal Mode "<<"\033[0m";
	moveCursor(commandLine-2,hzCentre+12);
	for(int i=hzCentre+12; i<=col; i++) cout<<"-";
	moveCursor(commandLine,0);
	for(int i=commandLine; i<statusLine; i++) printf("\033[K");
	moveCursor(cursor,0);
}

void printCommandMode(){
	moveCursor(commandLine-2,0);
	for(int i=0; i<hzCentre-12; i++) cout<<"-";
	moveCursor(commandLine-2,hzCentre-9);
	cout<<"\033[4;36m"<<"Mode : Command Mode"<<"\033[0m";
	moveCursor(commandLine-2,hzCentre+12);
	for(int i=hzCentre+12; i<=col; i++) cout<<"-";
	moveCursor(commandLine,0);
	cout<<"\033[1;36m"<<"Enter-Command ~$ "<<"\033[0m";
	moveCursor(commandLine,18);
	for(int i=commandLine; i<statusLine; i++) printf("\033[K");
	moveCursor(commandLine,18);
}

/******************************************* DISPLAY ALL FILES AND FOLDERS ********************************************/

void displayFiles(){

	clr();
	struct stat fileInfo;

	for(auto itr = top; itr < min(bottom, noOfFiles()); itr++){
		lstat(fileNames[itr]->d_name,&fileInfo);
		//struct tm *t=gmtime(&(fileInfo.st_mtime));
		//string months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

        cout<<"$"<<itr+1<<" : \t\t";
        (S_ISDIR(fileInfo.st_mode)) ? cout<<"d" : S_ISSOCK(fileInfo.st_mode) ? cout<<"s": cout<<"-";

        (S_IRUSR & fileInfo.st_mode) ? cout<<"r" : cout<<"-"; (S_IWUSR & fileInfo.st_mode) ? cout<<"w" : cout<<"-"; (S_IXUSR & fileInfo.st_mode) ? cout<<"x" : cout<<"-";

        (S_IRGRP & fileInfo.st_mode) ? cout<<"r" : cout<<"-"; (S_IWGRP & fileInfo.st_mode) ? cout<<"w" : cout<<"-"; (S_IXGRP & fileInfo.st_mode) ? cout<<"x" : cout<<"-";

        (S_IROTH & fileInfo.st_mode) ? cout<<"r" : cout<<"-"; (S_IWOTH & fileInfo.st_mode) ? cout<<"w" : cout<<"-"; (S_IXOTH & fileInfo.st_mode) ? cout<<"x" : cout<<"-";
			
		int SIZE_OF_FILE = fileInfo.st_size;
		if(SIZE_OF_FILE/1048576 > 1) cout<<"\t\t"<<SIZE_OF_FILE/1048576<<" MB";
		else if(SIZE_OF_FILE/1024 > 1) cout<<"\t\t"<<SIZE_OF_FILE/1024<<" KB";
		else cout<<"\t\t"<<SIZE_OF_FILE<<" B";

		/*auto uid = geteuid();
    	auto pw = getpwuid(uid);
    	if (pw) printf("\t\t%s",pw->pw_name);*/
		/*cout<<"\t\t\t"<<months[(t->tm_mon)]<<" "<<t->tm_mday<<" "<<t->tm_hour<<":"<<t->tm_min;*/	

		if((S_ISDIR(fileInfo.st_mode))){
			cout<<"\t\t"<<"\033[1;32m"<<fileNames[itr]->d_name<<"\033[0m";
		}
		else{
			cout<<"\t\t"<<"\033[1;36m"<<fileNames[itr]->d_name<<"\033[0m";
		}

		cout<<"\n";
	}
	printCWD();
	printNormalMode();
	return;
}

/**************************************** PROCESS CURRENT DIRECTORY *****************************************/

void processCurrentDIR(char const* dir){
	DIR* di;
	struct dirent* direntStructure;

	if(!(di=opendir(dir))){
		printAlertLine("Directory is empty                                 ");
		return;
	}
	chdir(dir);
	getcwd(cwd,cwdSize);
	fileNames.clear();

	while((direntStructure=readdir(di))){
		fileNames.push_back(direntStructure);
	}

	closedir(di);
	resetPointers();
    //bottom = min(MAX,noOfFiles());
	displayFiles();
	return;
}

/********************************* SCROLLING AND VERTICAL OVERFLOW FIX *************************************/

void scrollUp(){
	if(cursor>1){
		cursor--;
		moveCursor(cursor,0);
		return;
	}
	if(top==0){
		printAlertLine("You hit the top                               ");
		return;
	} 
	top--;
	bottom--;
	displayFiles();
	moveCursor(cursor,0);
	return;
}

void scrollUpK(){
    top = max(top-MAX, 0);
    bottom = top+MAX;
    displayFiles();
    moveCursor(cursor,0);
    return;
}

void scrollDownL(){
    bottom = min(bottom+MAX, noOfFiles());
    top = bottom - MAX;
    displayFiles();
    moveCursor(cursor,0);
    return;
}

void scrollDown(){
	if(cursor<noOfFiles() && cursor<MAX){
		cursor++;
		moveCursor(cursor,0);
		return;
	}
	if(bottom==noOfFiles()){
		printAlertLine("You hit the bottom                                ");
		return;
	} 
	top++;
	bottom++;
	displayFiles();
	moveCursor(cursor,0);
	return;
}

/************************************* GO TO PARENT DIRECTORY ****************************************/

void levelUp(){
	if(cwd==rootPath){
		printAlertLine("You're already present in the home directory");
		return;
	}
	backS.push(string(cwd));
	processCurrentDIR("../");  // this line moves our control to parent directory
	return;
}

/**************************************** GO TO HOME DIRECTORY **************************************/

void home(){
	if(cwd==rootPath){
		printAlertLine("You're already present in the home directory");
		return;
	} 
	backS.push(string(cwd));
	processCurrentDIR(rootPath.c_str());
	return;
}

/************************************** MOVE BACK AND FORWARD ***************************************/

void moveBack(){
	if(!backS.size())return;
	
	string prevDirectory = backS.top();
	backS.pop();
	forwardS.push(string(cwd));
	processCurrentDIR(prevDirectory.c_str());
	return;
}

void moveForward(){
	if(!forwardS.size())return;
	
	string nextDirectory = forwardS.top();
	forwardS.pop();
	backS.push(string(cwd));
	processCurrentDIR(nextDirectory.c_str());
	return;
}

/************************************** ENTER INTO THE FOLDER *****************************************/

void enter(){
	struct stat fileInfo;
	char *fileName = fileNames[cursor+top-1]->d_name;
	lstat(fileName,&fileInfo);

	if(S_ISDIR(fileInfo.st_mode)){
		if(strcmp(fileName,"..")==0 ){
			levelUp();
			return;
		}  
		if(strcmp(fileName,".")==0) return;
		backS.push(string(cwd));
		processCurrentDIR((string(cwd)+'/'+string(fileName)).c_str());
	}
	else{
		pid_t pid=fork();
		if(pid==0){
			printAlertLine("File opened in default editor                   ");
			execl("/usr/bin/xdg-open","xdg-open",fileName,NULL);
			exit(1);
		}
	}
	return;
}

/***************************************** COPY FILE/DIRECTORY ****************************************************/

void copy_helper(string fname, string path){
	char b[1024];
	int fin,fout, nread;
	fin = open(fname.c_str(),O_RDONLY);
	fout = open((path).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	while((nread = read(fin,b,sizeof(b)))>0){
		write(fout,b,nread);
	}
}

void copy(int i){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	string fname = commandTokens[i];
	string path = destination+'/'+fname;
	copy_helper(fname, path);
	
}

void copy_dir_helper(string dirName, string destination){
	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(dirName.c_str()))){
		printStatusLine("Can't open the directory                            ");
		return;
	}

	chdir(dirName.c_str()); 
	while((diren = readdir(di))){
		lstat(diren->d_name,&fileInfo);
		string dname =  string(diren->d_name);
		if(S_ISDIR(fileInfo.st_mode)){
			
			if( (dname == ".") || (dname == "..") ){
				continue;
			}
			/*if(s(".",diren->d_name) || strcmp("..",diren->d_name)==0){
				continue;
			}*/
			mkdir((destination + '/' + dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
			copy_dir_helper(dname , destination + '/' + dname);
		}
		else{
			copy_helper(dname,destination + '/' + dname);
		}
	}
	chdir("..");
	closedir(di);
	return;
}

void copy_dir(int i){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	string dname = commandTokens[i];
	mkdir((destination+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
	copy_dir_helper(cwd + '/' + dname, destination + "/" + dname);
		//copy_dir_helper(dname, destination);
	
}

void copyWrapper(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];
	struct stat fileInfo;
	for(int i=1; i<len-1; i++){
		string loc = cwd + '/' + commandTokens[i];
		lstat(loc.c_str(), &fileInfo);
		if(S_ISDIR(fileInfo.st_mode)){
			copy_dir(i);
		}
		else{
			copy(i);
		}
	}
}

/*********************************** DELETE FILE/DIRECTORY ******************************************/

int delete_file(){
	string destination = commandTokens[1];
	int status = unlink(destination.c_str());
	return status;
}

void delete_dir_helper(string destination){
	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(destination.c_str()))){
		printStatusLine("Can't open the directory                             ");
		return;
	}
	chdir(destination.c_str()); 
	while((diren = readdir(di))){
		lstat(diren->d_name,&fileInfo);
		if(S_ISDIR(fileInfo.st_mode)){
			if(strcmp(".",diren->d_name)==0 || strcmp("..",diren->d_name)==0){
				continue;
			}
			delete_dir_helper(diren->d_name);
			rmdir(diren->d_name);
		}
		else{
			unlink(diren->d_name);
		}
	}
	chdir("..");
	closedir(di);
}

int delete_dir(){
	string destination = commandTokens[1];
	//printStatusLine(destination);
	if(destination==cwd){
		printStatusLine("You are present inside the directory which you want to delete!");
		return 0;
	}
	delete_dir_helper(destination);
	rmdir(destination.c_str());
	return 1;
}

/******************************************* MAIN FILE/DIRECTORY *******************************************/

void move(int i){
	copyWrapper();
	int len = commandTokens.size();
	
	string fname = commandTokens[i];
	string deleting_path = string(cwd) + "/" + fname;
	int status = unlink(deleting_path.c_str());
	
}

void move_dir(int i){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];
	
	string dname = commandTokens[i];
	mkdir((destination+'/'+dname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
	copy_dir_helper(cwd + '/' + dname ,destination + '/' + dname);
	delete_dir_helper(dname);
	rmdir(dname.c_str());	
	
}

void moveWrapper(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];
	struct stat fileInfo;

	for(int i=1; i<len-1; i++){
		string loc = cwd + '/' + commandTokens[i];
		lstat(loc.c_str(), &fileInfo);
		if(S_ISDIR(fileInfo.st_mode)){
			move_dir(i);
		}
		else{
			move(i);
		}
	}

}

/***************************************** CREATE FILE/DIRECTORY *******************************************/

void create_file(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	for(int i=1; i<len-1; i++){
		string fname = commandTokens[i];
		open((destination + '/' + fname).c_str(),O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
	}
}

void create_dir(){
	int len = commandTokens.size();
	string destination = commandTokens[len-1];

	for(int i=1; i<len-1; i++){
		string fname = commandTokens[i];
		mkdir((destination + '/' + fname).c_str(),S_IRUSR|S_IWUSR|S_IXUSR);
	}
}

/************************************************ RENAME ****************************************************/

void rename(){
	string oldName = commandTokens[1];
	string newName = commandTokens[2];
	rename(oldName.c_str(),newName.c_str());
}

/************************************************ GOTO ******************************************************/

void goTo(){
	string destination = commandTokens[1];
	if(destination == "/") home();
	else processCurrentDIR(destination.c_str());
}

/************************************************ SEARCH ****************************************************/

bool search_helper(string dirName, string tobeSearch){
	DIR *di;
	struct dirent *diren;
	struct stat fileInfo;

	if(!(di = opendir(dirName.c_str()))){
		printStatusLine("Can't open the directory                             ");
		return false;
	}

	chdir(dirName.c_str()); 
	while((diren = readdir(di))){
		lstat(diren->d_name,&fileInfo);
		string dname =  string(diren->d_name);
		if(tobeSearch == dname){
			//printAlertLine("A WILD TRUE APPEARS");
			processCurrentDIR(dirName.c_str());
			return true;
		}
		if(S_ISDIR(fileInfo.st_mode)){
			if( (dname == ".") || (dname == "..") ){
				continue;
			}
			bool t =  search_helper(dirName + '/' + dname, tobeSearch);
			if(t) return true;
		}
	}
	chdir("..");
	closedir(di);
	return false;
}

bool search(){
	string tbs = commandTokens[1];
	return search_helper(cwd, tbs);
}

/************************************************ COMMAND MODE ****************************************************/

bool modifyPath(){
	int len = commandTokens.size();
	string path = commandTokens[len-1];

	if(path[0]=='~')
	path.replace(0,1, rootPath);

	else if(path[0]=='.')
	path = cwd;

	else if(path[0]=='/')
	path = cwd + path;

	else return false;

	commandTokens[len-1] = path;
	return true;
}

void performActions(string query){
	
	if(query == "copy"){
		copyWrapper();
		printStatusLine("Files copied successfully!                           ");
	}

	else if(query == "move"){
		moveWrapper();
		processCurrentDIR(cwd);
		printStatusLine("Files moved successfully!                            ");
	}

	else if(query == "delete_file"){
		int sts = delete_file();
		if(!sts){
			processCurrentDIR(cwd);
			printStatusLine("File deleted successfully..                           ");
		} 		
		else printStatusLine("Deletion Failed!                                     ");
	}

	else if(query == "delete_dir"){
		int sts = delete_dir();
		if(sts!=0){
			processCurrentDIR(cwd);
			printStatusLine("Directory deleted successfully!                          ");
		}
	}

	else if(query == "create_file"){
		create_file();
		processCurrentDIR(cwd);
		printStatusLine("Files created successfully!                                ");
	}

	else if(query == "create_dir"){
		create_dir();
		processCurrentDIR(cwd);
		printStatusLine("Directory created successfully!                            ");
	}

	else if(query == "rename"){
		rename();
		processCurrentDIR(cwd);
		printStatusLine("File renamed successfully!                           ");
	}

	else if(query == "goto"){
		backS.push(string(cwd));
		goTo();
	}

	else if(query == "search"){
		bool f = search();
		if(f) printStatusLine("File/Folder found!                               ");
		else printStatusLine("File/Folder not found!                            ");
	} 

	else{
		printStatusLine("Invalid Command!                                        ");
	}

	return;
}

string processInput(){
	char ch; string command; 
	cursorLoc = 18; moveCursor(commandLine, 18);
	while(true){
		ch = cin.get();
		if(ch==10){
			cursorLoc = 18;
			moveCursor(commandLine, cursorLoc);
			return command;
		}
		if(ch==27){
			printAlertLine("Welcome back to Normal Mode                                      ");
			return "esc";
		}
		else{
	
			if(ch==127){
				if(cursorLoc==18) continue;
				command.pop_back();
				--cursorLoc;
				printCommandMode();
				cout<<command;
				moveCursor(commandLine, 18+command.size());
			}
			else{
				command.push_back(ch);
				cout<<ch;
				moveCursor(commandLine, ++cursorLoc);
			}
		}
	}
	return "cont";
}

void switchToCommandMode(){

	while(true){
		printCommandMode();
		commandTokens.clear();
		string command = processInput();
		if(command=="esc"){
			printNormalMode();
			break;
		} 
		else{
			clearLine();
		}

		stringstream sscommand(command); 
    	string token; 
		while(getline(sscommand, token, ' ')) commandTokens.push_back(token); 

		string query = commandTokens[0];
		int isValidPath = modifyPath();	

		if(!isValidPath && query!="search" && query!="rename"){
			printAlertLine("Invalid Command !                                                 ");
			moveCursor(statusLine, 0);
			clearLine();
			printCommandMode();
			continue;
		}
		
		performActions(query);
	}
	return;
}

/************************************************ MAIN FUNCTION ****************************************************/

int main(){

    clr();
	getcwd(cwd,cwdSize);
	init(cwd);
	printCWD();
	processCurrentDIR(cwd);

	char ch=' ';
	while(ch!='q'){
	
			ch=cin.get();	

			switch(ch){
				case ':':switchToCommandMode();break;

				case 10:enter();break;

				case 127:levelUp();break;

				case 65:scrollUp();break;

				case 68:moveBack();break;

				case 107:scrollUpK();break;

				case 104:home();break;

				case 66:scrollDown();break;

				case 67:moveForward();break;

                case 108:scrollDownL();break;
				
				default:break;
			
		}
	}

	clr();
	cout<<endl<<endl<<endl<<endl<<endl<< " ---------------------------   | THANKS FOR USING MY FILE EXPLORER |  ----------------------------"<<endl<<endl<<endl<<endl<<endl;
	atexit(disableNormalMode);

    return 0;
}

