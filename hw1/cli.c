#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include <wait.h>

#define MAX_LINE  1024
#define DEBUG     1


// Parsing function
int issueCommand(char** tokens, FILE* masterOutput);

//  I/O functions
FILE* outCheck(char** tokens);

// Sets fref and shwoenv to PATH
int setPaths();

// setup external function args
char** getargs(char** tokens);

//  command functions
int pwd(char** tokens, FILE* masterOutput);
int cd(char** tokens, FILE* masterOutput);

int runExternal(char** tokens, FILE* masterOutput);

/*


 */
int main (int argc, char** argv) {

    char line[MAX_LINE];
    char** tokens;

    FILE* masterOutput = stdout;

    setPaths();

    // get a line from stdin and get the tokens
    printf("$> ");

    while (fgets(line, MAX_LINE, stdin)) { 

        // remove the '\n' from the end of the line
        line[strlen(line)-1] = '\0';

        tokens = gettokens(line); 

        masterOutput = outCheck(tokens);

        issueCommand(tokens, masterOutput);

        //return masterOutput to stdout if needed
        if (masterOutput != stdout) {
            fclose(masterOutput);
            masterOutput = stdout;
        }

        free(tokens);
        printf("$> ");
    }
    return 0;
}


/*
   Handles user input

 */

int issueCommand(char**tokens, FILE* masterOutput){

    int i;
    int opt = -1;

    char* opts[5];
    opts[0] = "exit";
    opts[1] = "cd";
    opts[2] = "pwd";


    if(tokens[0]){

        //locate index of user command in opts array for switch
        for (i = 0; i < 3; i++){
            if (strcmp(tokens[0], opts[i]) == 0){
                opt = i;
            }
        }

        switch(opt){
            case 0://exit
                exit(0);
                break;
            case 1://cd
                cd(tokens, masterOutput);
                break;
            case 2://pwd      
                pwd(tokens, masterOutput);
                break;
            default:
                runExternal(tokens, masterOutput);
                break;
        }
    }
    return 0;
}


/*
   Performs cd command


 */

int cd(char** tokens, FILE* masterOutput){

    char *dir = tokens[1];
    char succ[MAX_LINE];
    char fail[MAX_LINE];
    char *output;

    strcpy(succ, "cwd changed to ");
    strcpy(fail, "No such directory: ");

    if (!(tokens[1])) {
        fprintf(masterOutput, "Please enter a directory.\n");
        return 0;
    }


    if (!chdir(dir)){
        fprintf(masterOutput, "%s", succ);
        pwd(tokens, masterOutput);
        fprintf(masterOutput, "\n");
    }else{
        output = strcat(fail, dir);
        fprintf(masterOutput, "%s\n", output);
    }

    return 0;
}

/*
   Performs pwd command


 */

int pwd(char** tokens, FILE* masterOutput){

    char *cwd;
    char buff[MAX_LINE];
    size_t size = MAX_LINE;

    cwd = getcwd(buff, size);

    fprintf(masterOutput, "%s", cwd);

    return 0;

}

/*
   Runs eternal commands. 


 */
int runExternal(char **tokens, FILE* masterOutput){

    int status;
    pid_t pid;
    char **args;

    args = getargs(tokens);

    if((pid = fork()) == -1){ //error

    }else if (pid == 0){ //child
        //set childs I/O
        dup2(fileno(masterOutput), STDOUT_FILENO);

        execvp(tokens[0], args);

        fclose(masterOutput);

        exit(0);
    }else{ //parent

        wait(&status);
#if DEBUG
        printf("child status: %d\n", status);
#endif
    }

    return 0;
}



/*
   Creates args for external commands by
   removing I/O redirects from tokens
   if they exist.

 */
char **getargs(char** tokens){

    char **args;
    int i = 0;
    int j = 0;

    //count size of args
    for (i = 0; tokens[i]; i++){
        if((tokens[i][0] == '>') || (tokens[i][0] == '<')){ 
            if(tokens[i][1] != '\0'){//no space
                j++;
            }else{//space after
                j = j + 2;
            } 
        }
    }

    args = (char **)malloc((i-j) * sizeof(char **));
    j = 0;

    //fill args
    for (i = 0; tokens[i]; i++){
        if((tokens[i][0] == '>') || (tokens[i][0] == '<')){ 
            if(tokens[i][1] == '\0'){//space between file, skip it
                i++;
            }
        }else{
            args[j] = (char *)malloc(sizeof(char) * strlen(tokens[i]));
            args[j] = tokens[i];
            j++;
        } 
    }
    args[j] = 0;
    return args;
}



/*
   sets masterOutput to either stdout or a file
 */
FILE* outCheck(char **tokens){

    int i;
    FILE *newout;

    for (i = 0; tokens[i]; i++){

        if(tokens[i][0] == '>'){ //stdout

#if DEBUG
            printf("> found in token: %s\n", tokens[i]);
#endif

            if(tokens[i][1] != '\0'){//no space
                if((newout = fopen(&tokens[i][1],"a"))){
                    return newout;
                }else{
                    printf("cannot open file %s to write\n", &tokens[i][1]);
                }
            }else{//space after
                if((newout = fopen(tokens[i+1],"a"))){
                    return newout;
                }else{
                    printf("cannot open file %s to write\n", tokens[i+1]);
                }
            } 
        }
    }
    return stdout;
}
/*

   Adds programs in cwd to PATH env.
   called at the start of the program cwd is changed

 */

int setPaths(){

    char *oldPATH = getenv("PATH");

    char *cwd;
    char buff[MAX_LINE];
    size_t size = MAX_LINE;

    cwd = getcwd(buff, size);

    char* newPATH = (char *)malloc(MAX_LINE * sizeof(char));

    strcpy(newPATH, oldPATH);

    strcat(newPATH, ":");
    strcat(newPATH, cwd);

    setenv("PATH", newPATH, 1);

    return 0;
}
