#include <filesystem>
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <errno.h>

bool Recursive = false;
bool CaseInsensitive = false;


namespace fs = std::filesystem;

bool iequals(const std::string& a, const std::string& b)
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

int getDir(std::string path,std::string& file,std::vector<fs::directory_entry>& directory) {
    for (auto &entry : fs::directory_iterator(path)){
        if(!CaseInsensitive && entry.path().filename() == file){
            return 1;
        }
        if(CaseInsensitive && iequals(entry.path().filename(),file)){
            file = entry.path().filename();
            return 1;
        }
        if(entry.is_directory()){
            directory.push_back(entry);
        }
    }
    return 0;
}



void childSearch(std::string path,std::string& file,bool& found) {
    if(found){
        return;
    }
    std::vector<fs::directory_entry> directory;
    if(getDir(path,file,directory)){
        pid_t pid = getpid();
        std::cout << pid << ": " << file << ": " << path + '/' + file << "\n";
        found = true;
    }
    else{
        if(directory.size() != 0 && Recursive){
            for(auto& entry: directory){
                childSearch(entry.path(),file, found);
            }
        }
    }

}

void processEngine(std::string initialPath, std::vector<std::string> files) {
    pid_t ppid = getpid();
    pid_t childpid;

    for (int i = 0; i < files.size(); i++) {
        if (getpid() == ppid) {
            childpid = fork();
            if (getpid() == ppid)
                continue;
        }
        bool found = false;
        switch (childpid) {
        case -1:
            perror("Failed to fork");
            return;
            break;
        case 0:
            // children
            childSearch(initialPath,files[i],found);
            if(!found){
                //std::cout << "file " + files[i] + " not found\n";
            }
            exit(0);
            break;
        default:
            break;
            // parent
        }
    }

    if (getpid() == ppid) {
        for(int i = 0; i < files.size();i++){
            wait(NULL);
        }

    }

}

void parseCLI(int argc, char** argv,int& flags){
    int opt;
      
    while((opt = getopt(argc, argv, "Ri")) != -1) 
    { 
        switch(opt) 
        { 
            case 'R': 
                //printf("Recursive set %c\n", opt);
                Recursive = true;
                flags++;
                break; 
            case 'i': 
                //printf("Case-insensitive set %c\n", opt); 
                CaseInsensitive = true;
                flags++;
                break;  
            case '?': 
                throw 1;
                break; 
        } 
    } 
}



int main(int argc, char** argv)
{
    int flags = 0;

    try{
        parseCLI(argc,argv,flags);
    }catch(int i){
        switch(i){
            case 1:
            std::cout << "Invalid Flag\n";
            return 1;
        }
        
    }

    std::string searchpath;
    int argIndex = 0;
    switch(flags){
        case 0:
            searchpath = argv[1];
            argIndex = 2;
            break;
        case 1:
            searchpath = argv[2];
            argIndex = 3;
            break;
        case 2:
            searchpath = argv[3];
            argIndex = 4;
            break;
        default:
            fprintf(stderr,"Flag count error");
    }

    std::vector<std::string> files;

    for(; argIndex < argc; argIndex++){
        files.push_back(argv[argIndex]);
    }

    processEngine(searchpath,files);
    

    return 0;
}