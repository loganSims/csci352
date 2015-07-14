#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include <wait.h>

#define MAX_LINE  1024
#define DEBUG     0

// Parsing function
int issueCommand(char** tokens);

//  I/O functions
int outCheck(char** tokens);
char** inCheck(char** tokens);
char** getInput(char** tokens, FILE* infile, int start, int end);

// Sets fref and shwoenv to PATH
int setPaths();

// setup external function args
char** getargs(char** tokens);

//  command functions
int pwd(char** tokens);
int cd(char** tokens);

int runExternal(char** tokens);

/*
  main function. 
  Has main loop for user input.
 */
int main (int argc, char** argv) {

    char line[MAX_LINE];
    char** tokens;

    setPaths();

    // get a line from stdin and get the tokens
    printf("$> ");

    while (fgets(line, MAX_LINE, stdin)) { 

        // remove the '\n' from the end of the line
        line[strlen(line)-1] = '\0';

        //get tokens
        tokens = gettokens(line); 

        //check output options
        outCheck(tokens);

        tokens = inCheck(tokens);

        //execute user command
        issueCommand(tokens);

        //return output to stdout
        if(freopen("/dev/tty","w", stdout) == NULL){
            printf("failed to change stdout back\n");
        }

        free(tokens);
        printf("$> ");
    }
    return 0;
}
/*
   Handles user input.
 */
int issueCommand(char**tokens){

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
                cd(tokens);
                break;
            case 2://pwd      
                pwd(tokens);
                break;
            default:
                runExternal(tokens);
                break;
        }
    }
    return 0;
}
/*
   Performs cd command.

   Uses chdir() function.
   Uses pwd to display cwd on success.
 */
int cd(char** tokens){

    char *dir = tokens[1];
    char succ[MAX_LINE];
    char fail[MAX_LINE];
    char *output;

    strcpy(succ, "cwd changed to ");
    strcpy(fail, "No such directory: ");

    //check for input
    if (!(tokens[1])) {
        printf("Please enter a directory.\n");
        return 0;
    }

    //attempt to change directory
    if (!chdir(dir)){
        printf("%s", succ);
        pwd(tokens);
        printf("\n");
    }else{
        output = strcat(fail, dir);
        printf("%s\n", output);
    }

    return 0;
}
/*
   Performs pwd command.

   Used getcwd command to display
   current working directory.
 */
int pwd(char** tokens){

    char *cwd;
    char buff[MAX_LINE];
    size_t size = MAX_LINE;

    cwd = getcwd(buff, size);

    printf("%s", cwd);

    return 0;

}
/*
   Runs eternal commands. 

   Uses fork and execvp to run child process

   Builds arguments for child by using 
   getargs() defined below.  
 */
int runExternal(char **tokens){

    int status;
    pid_t pid;
    char **args;

    args = getargs(tokens);

    if((pid = fork()) == -1){ //error

        printf("failed to fork.");

    }else if (pid == 0){ //child

        //TODO? set childs I/O

        execvp(tokens[0], args);

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
   sets output stdout or a file
 */
int outCheck(char **tokens){

    int i;

    for (i = 0; tokens[i]; i++){

        if(tokens[i][0] == '>'){ //stdout

#if DEBUG
            printf("> found in token: %s\n", tokens[i]);
#endif
            if(tokens[i][1] != '\0'){//no space
                if((freopen(&tokens[i][1],"w", stdout)) == NULL){
                    printf("cannot open file %s to write\n", &tokens[i][1]);
                }
            }else{//space after
                if((freopen(tokens[i+1],"w", stdout)) == NULL){
                    printf("cannot open file %s to write\n", tokens[i+1]);
                }
            } 
        }
    }
    return 0;
}

/*
   sets reads input file if present and appends to tokens.
 */
char** inCheck(char** tokens){

    int i;
    FILE * infile;

    for (i = 0; tokens[i]; i++){

        if(tokens[i][0] == '<'){ //stdin

#if DEBUG
            printf("< found in token: %s\n", tokens[i]);
#endif
            if(tokens[i][1] != '\0'){//no space
                if((infile = fopen(&tokens[i][1],"r")) == NULL){
                   printf("Cannot open file for reading\n");
                }else{
                   tokens = getInput(tokens, infile, i, i+1);
                }
            }else{//space after
                if((infile = fopen(tokens[i+1],"r")) == NULL){ 
                   printf("Cannot open file for reading\n");
                }else{
                   tokens = getInput(tokens, infile, i, (i+1));
                }
            } 
        }
    }
    return tokens;
}

/*
  Reads a file for input, appending it to
  the current token commands.



 */
char** getInput(char** tokens, FILE* infile, int start, int end){

    char** t;
    char line[MAX_LINE];
    int i = 0;
    int size = start;

    char** newtokens = (char**)malloc(start * sizeof(char*));
    
    memcpy(newtokens, tokens, (start * sizeof(char*)));

    while (fgets(line, MAX_LINE, infile)) { 

        line[strlen(line)-1] = '\0';

        t = gettokens(line); 

        //count size of t, add that to current size of newtokens
        while(t[i]){size++; i++;};

        //realloc space for t to be appended to newtokens
        newtokens = (char**)realloc(newtokens, size * sizeof(char*));

        //append t to newtokens
        memcpy(newtokens + start, t, (i * sizeof(char*)));

        //increment start to the new end of newtokens
        start = start + i;

        free(t);

    }

    //count size of tokens
    i = 0;
    while(tokens[i]){i++; size++;};
        
    //realloc space for the rest of the tokens to be appended to newtokens
    newtokens = (char**)realloc(newtokens, size * sizeof(char*));

    //append remaining tokens to newtokens
    memcpy(newtokens+start, tokens + end, ((i - end) * sizeof(char *)));

    free(tokens);

    //NULL term newtokens
    newtokens[start + (i-end)] = NULL;

    return newtokens;

}



/*
   Appends cwd to PATH env var.
   Called at the start of the program before 
   cwd is changed. Allows fref and showenv to
   be executed independent of cwd.
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
