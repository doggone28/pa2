#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//Down below are the stuff we need
#include <sys/stat.h> 
//Tells us the type of each argument
//On success, stat() returns 0.
#include <fcntl.h> //Adds the modes
#include <dirent.h> //Holds open dir etc
#include <unistd.h> //Holds open files etc

#define BUFFER 4096;


/*
    Will check if the file name ends witha .txt to be scanned
    Will return 0 if false and 1 if true
    We will use strncmp with pointer arithmatci to reach the last 4 chars and not use strcmp because of null terminator might not be added
*/

int txtFile(const char *name){
    int len = strlen(name); //strlen does not include the null terimator
    //We need to chech for the last 4 chars .txt so if the name is less than 4 chars we return 0
    if(len < 4) {
        return 0;
    }
    //Now we use strcmp to compare the last 4 chars of our string to .txt
    //So name pointer arithmatic to end of last char (not \0) then -4 to get to the begging of the .
    if (strncasecmp(name + (len-4), ".txt", 4) == 0){ // == 0 means its the same anything else is not 
        return 1;
    }else{
        return 0;
    }
}

/*
Here we will recursivley search through a directory
Somethings we need to know before starting this
    -On the first go we will pass the arg string and then open it with openDIR
    -We will then traverse the contents with the struct *dirent
    -To get the type of the Directory Entry we use the struct stat, which needs a path to open it
    -Now since this is recursion we need to construct a path for the struct stat to work as it needs a path to know where its looking at
        -This will also be needed when we scan the file using open because it needs the path to get there
        -So to construct the path we will keep recursively call the dirname each time while we add onto it using strcpy and strcat
    -Now that we have the correct path we can use it in stat and then when scanning files into open
    -Remember these rules stated by the assignment
        1. Names beginning with a period (.) are ignored
        2. Directories are recursively traversed
        3. Regular files with names ending in “.txt” are scanned
        4. Any other files are ignored
*/

void searchDirectory(const char *dirName){ 
    DIR* dir = opendir(dirName);
    if(dir == NULL){
        perror("opendir failed");
        return;
    }
    //We will now travese the directory recursively 
    struct dirent* entry; 
    struct stat pathStatus; 

    while((entry = readdir(dir)) != NULL){
        if(entry->d_name[0] == '.'){  //Ignoring names with (.) at index 0
            continue;
        }

        //We now need to construct the correct path to the file we are at --> Example foo/bar/baz
        char path[4096] = {0};
        strcpy(path, dirName); //--> On first go foo
        strcat(path, "/"); //--> foo/
        strcat(path, entry->d_name); // foo/bar is now our the path to the entry we are looking at
        //!This might lead to overflow so need to find a better way to make it better

        //Now that we have the correct path we can now get the type of the entry
        int type = stat(path, &pathStatus);
        if (type != 0) { // Something went wrong, stat returns 0 on success
            perror("stat");
            continue;
        }

        if (S_ISREG((pathStatus.st_mode))) { //If its a regular file
            int checkIfTxt = txtFile(path);
            if(checkIfTxt == 0){
                printf("Not a txt file just a reg one %s\n", path);
            }else{
                printf("This is a text file %s\n", path);
            }
        } else if ((S_ISDIR(pathStatus.st_mode))) { // If its a directory we recursively call our function again to enter again with the name of path
            printf("directory: %s\n", entry->d_name);
            searchDirectory(path);
        } else {
            printf("Skipping %s: It is not a file or a directory\n", entry->d_name);
        }
    }
    closedir(dir); 
}

/*
Our program will take in aruguements, we will use stat to determine whether they are of dir or regular files
Once we figure out the type 
    - If a directory we will traverse it recusivley and then send each txt file to be scanned
    - If it is a regular file ending in txt we will send it out to be scanned
*/
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Not Enough Arguements");
        return 1;
    }
    //Process the inputs
    for(int i =0; i<argc; i++){
        struct stat pathStatus; //This will tell us the type of file we are reading
        int type = stat(argv[i],&pathStatus);
        
        if(type!=0){  //If it doesnt return 0 then something went wrong, print out an error of this arg and then continue to the next arg
            perror(argv[i]);
            continue;
        }
        
        //So now the struct is assigned to the file/dir and using .st_mode it will give us the type if its a file or a directory
        if (S_ISREG((pathStatus.st_mode))) { // If its a regular file
            int checkIfTxt = txtFile(argv[i]);

            //Checking if my txt file identifier works
            if(checkIfTxt == 0){
                printf("Not a txt file just a reg one %s\n", argv[i]);
            }else{
                printf("This is a text file %s\n", argv[i]);
            }

        }else if (S_ISDIR(pathStatus.st_mode)) { // If its a directory we need to scan the contents inside recursivley
            searchDirectory(argv[i]);
        } else {
            printf("Skipping %s: It is not a file or a directory\n", argv[i]);
        }
    
    }

}