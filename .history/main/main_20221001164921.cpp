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


namespace fs = std::filesystem; //abkuerzung fuer filesystem namespace

bool iequals(const std::string& a, const std::string& b) //Hilfsfunktion zum Vergleichen von Strings ungeachtet der Groß- oder Kleinschreibung
{
    unsigned int sz = a.size();
    if (b.size() != sz)
        return false;
    for (unsigned int i = 0; i < sz; ++i)
        if (tolower(a[i]) != tolower(b[i]))
            return false;
    return true;
}

/*
Funktion die den Inhalt des Verzeichnisses im directory vector speichert
Die Variable "path" ist der Pfad in dem die Funktion nach der Datei suchen soll, "file" ist der Name der Datei.
*/

int getDir(std::string path,std::string& file,std::vector<fs::directory_entry>& directory) { 
    for (auto &entry : fs::directory_iterator(path)){
        if(!CaseInsensitive && entry.path().filename() == file){
            /* 
            sollten der Name der gefundenen Datei gleich dem Namen der gesuchten Datei sein, dann returned die Funktion 1
            Dieser Rückgabewert wird in der childSearch Funktion abgefangen und beendet das Suchen.
            */
            return 1;
        }
        if(CaseInsensitive && iequals(entry.path().filename(),file)){
            // analog zu oben nur case-insensitive
            file = entry.path().filename();
            return 1;
        }
        if(entry.is_directory()){
            // jegliche Verzeichnisse die gefunden werden, werden in den directory vector gespeichert, um in ihnen rekursiv suchen zu können
            directory.push_back(entry);
        }
    }
    return 0;
}



void childSearch(std::string path,std::string& file,bool& found) {
    if(found){
        return; 
        // Falls der found boolean true ist wird die Suche in allen rekursiven Aufrufen dieses Prozesses beendet. 
        // (Um unnötiges Suchen zu vermeiden und die Rechenzeit zu verkürzen)
    }
    std::vector<fs::directory_entry> directory;
    if(getDir(path,file,directory)){ // Falls returnwert == 1 wurde die Datei gefunden
        pid_t pid = getpid();
        std::cout << pid << ": " << file << ": " << path + '/' + file << "\n";
        found = true;
    }
    else{
        if(directory.size() != 0 && Recursive){ // Falls 0 wird die Suche in den Unterverzeichnissen fortgesetzt
            for(auto& entry: directory){
                childSearch(entry.path(),file, found);
            }
        }
    }

}

void processEngine(std::string initialPath, std::vector<std::string> files) {
    pid_t ppid = getpid(); // Parent PID wird gespeichert
    pid_t childpid;

    for (int i = 0; i < files.size(); i++) {
        if (getpid() == ppid) { // Falls die PID == PPID dann soll geforkt werden, alle Prozesse werden ausschließlich vom Parent geforkt
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
            childSearch(initialPath,files[i],found); // Falls PID == 0 => wir befinden uns im Child Prozess d.h. wir können die Suche nach den Dateien beginnen.
            if(!found){
                //std::cout << "file " + files[i] + " not found\n";
            }
            exit(0); // Terminiert den Child Prozess
            break;
        default:
            break;
            // parent
        }
    }

    if (getpid() == ppid) {
        for(int i = 0; i < files.size();i++){
            wait(NULL); // Parent Prozess wartet bis alle Child Prozesse terminiert wurden
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
    int flags = 0; // speichert die Anzahl der gesetzten Flags

    try{
        parseCLI(argc,argv,flags);
    }catch(int i){
        switch(i){
            case 1:
            std::cout << "Invalid Flag\n"; // Throws Error falls eine der Flags ungültig ist
            return 1;
        }
        
    }

    std::string searchpath; 
    int argIndex = 0;
    /* 
    Da getopt das argv array, nach dem parsen, so sortiert, dass die Flags immer nach dem Programmnamen (immer argv[0]) aber vor dem Suchpfad und Dateinamen stehen
    muss zwischen der Anzahl der Flags im switch-case unterschieden werden, weil die Indexierung des Suchpfades und der Dateinamen im argv array davon abhängt.
    */
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
        files.push_back(argv[argIndex]); // alle Dateinamen nach welchen gesucht werden soll werden im "files" Vektor abgespeichert
    }

    processEngine(searchpath,files); //processEngine startet den Abspaltungsvorgang
    

    return 0;
}