#include <filesystem> //commands for directory search
#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include <dirent.h>
#include <errno.h>

void childSearch(std::string path,std::string file) {
    printf("I am child %ld \n", (long)getpid());
    if(getDir(path,file)){
        std::cout << "Found file at this path: " + path + "/" + file << std::endl;
    }
    else{
        std::cout << "File not found\n"'
    }

}

void processEngine(std::string initialPath, std::vector<std::string> files) {
    pid_t ppid = getpid();
    pid_t childpid;
    std::cout << "initial path: " + initialPath << std::endl;

    for (int i = 0; i < test.size(); i++) {
        bool isChild = false;
        if (getpid() == ppid) {
            childpid = fork();
            if (getpid() == ppid)
                continue;
        }
        switch (childpid) {
        case -1:
            perror("Failed to fork");
            return;
            break;
        case 0:
            childSearch(initialPath,files[0]);
            isChild = true;
            exit(0);
            break;
        }
        if (isChild)
            break;
    }


    if (getpid() == ppid) {
        printf("I am parent %ld\n", (long)getppid());
        for(int i = 0; i < test.size();i++){
            wait(NULL);
            std::cout << "Got " << i+1 << " done" << std::endl;
        }

    }

    //sleep(4);

    printf("all done!\n");
}

int getCWD() {
    long maxpath;
    char* mycwdp;

    if ((maxpath = pathconf(".", _PC_PATH_MAX)) == -1)
    {
        perror("Failed to determine the pathname length");
        return 1;
    }
    if ((mycwdp = (char*)malloc(maxpath)) == NULL)
    {
        perror("Failed to allocate space for pathname");
        return 1;
    }
    if (getcwd(mycwdp, maxpath) == NULL)
    {
        perror("Failed to get current working directory");
        return 1;
    }
    printf("Current working directory: %s\n", mycwdp);
    return 0;
}

int getDirContents(std::vector<std::string>& directory, char* currDir) {
    struct dirent* direntp;
    DIR* dirp;
    printf("%s",currDir);
    
    if (currDir == NULL)
    {
        fprintf(stderr, "No directory name");
        return 1;
    }

    dirp = opendir(currDir);
    if ((dirp == NULL))
    {
        perror("Failed to open directory");
        return 1;
    }
    
    //printf("here\n");

    while ((direntp = readdir(dirp)) != NULL) {
        //printf("%s\n", direntp->d_name);
        directory.push_back(direntp->d_name);
    }
    while ((closedir(dirp) == -1) && (errno == EINTR));
    return 0;
}

int getDir(std::string path,std::string file) {
    struct dirent* direntp;
    DIR* dirp;
    //printf("%s",currDir);
    
    if (path == NULL)
    {
        fprintf(stderr, "No directory name");
        return 1;
    }

    dirp = opendir(path);
    if ((dirp == NULL))
    {
        perror("Failed to open directory");
        return 1;
    }
    
    //printf("here\n");

    while ((direntp = readdir(dirp)) != NULL) {
        //printf("%s\n", direntp->d_name);
        if(direntp->d_name == file);
        return 1;
    }
    while ((closedir(dirp) == -1) && (errno == EINTR));
    return 0;
}

int main(int argc, char** argv)
{
    getCWD();
    std::vector<std::string> files = { "main" };
    processEngine(argv[1],files);

    std::vector<std::string> directory;

    getDirContents(directory, argv[1]);
    for (auto i : directory) {
        std::cout << i << std::endl;
    }

    return 0;
    
}