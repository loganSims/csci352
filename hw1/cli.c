/*
  Logan Sims
  W01086217
  Date: 7/20/2015

  CSCI 352 
  Assignment #1
  CLI

  A basic command line interpreter.

  Has 3 internal commands:
	1. exit
		exits program
	2. cd [dir] 
		changes cwd to dir
	3. pwd
		displays cwd

  Can also run external commands by
  using fork/exec.

 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include <wait.h>

#define MAX_LINE  1024
#define DEBUG     0


// handles user commands
int issueCommand(char** tokens);

//  I/O functions
int outCheck(char** tokens);
char** inCheck(char** tokens);
char** getInput(char** tokens, FILE* infile, int start, int end);

// Sets cwd to PATH
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

  Performs the following:
    1. Adds cwd to path
    3. Gets user input (loop begins)
    2. Checks I/O options
    3. Executes user command
    4. Returns output to stdout if needed (loop ends)
 */
int main (int argc, char** argv) {

    char line[MAX_LINE];
    char** tokens;

    setPaths();

    printf("$> ");

    while (fgets(line, MAX_LINE, stdin)) { 

        line[strlen(line)-1] = '\0';

        tokens = gettokens(line); 

        outCheck(tokens);
        tokens = inCheck(tokens);

        issueCommand(tokens);

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

   Input: Tokens from user.
   Return: 0 for success. (this value is not used)

   Gets tokens already processed for
   I/O redirection.

   Based on user command calls functions.

   NOTE: if command not internal assumes 
   user is trying to run external command.


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
            case 0:
                exit(0);
                break;
            case 1:
                cd(tokens);
                break;
            case 2:      
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

   Input: User tokens
   Return: 0 (value not used)

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
 
   Input: User tokens
   Return: 0 (value not used)

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
   
   Input: User tokens
   Return: 0 (value not used)

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
   Input: User Tokens
   Return: User tokens with I/O redirect 
   commands removed.

   Creates args for external commands by
   removing I/O redirects from tokens
   if they exist.

   First counts size of args,
   Then builds them.
 */
char **getargs(char** tokens){

    char **args;
    int i = 0;
    int j = 0;

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
   Handles output redirection.
   
   Input: User tokens
   Return: 0 (value not used)

   Sets output to a file if output redirect
   is in a command.
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
   Handles input redirect
  
   Input: User tokens
   Return: new tokens created from getInput().
	   Refer to getInput() for details

   Reads tokens for input redirect.
   locates location in command for 
   getInput()

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
                   tokens = getInput(tokens, infile, i, (i+2));
                }
            } 
        }
    }
    return tokens;
}

/*
   Reads a file for input, appending it to
   the current token commands.

   Input: 
	1. tokens: User tokens.
	2. infile: The input file given by user
	3. start: The start index of the input redirect command
	4. end: The end index.
   Return: Newly created tokens with contents of input file
   appended into old tokens.


   Inputs 3 and 4 are used for the 
   append this function makes.
   When given an input file the CLI
   subs the contents of the file in the 
   location of the input redirect command.

   ex. 
      echo < input.txt > out.txt
      becomes...
      echo Hello World > out.txt 
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
        
    //realloc space for the rest of the tokens 
    //to be appended to newtokens
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

   Input: N/A
   Returns: 0 (value not used)

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
