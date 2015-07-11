#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"
#include <wait.h>

#define MAX_LINE  1024
#define DEBUG     1



//  I/O functions
FILE* outCheck(char **tokens);

char **getargs(char** tokens);

//  command functions
int pwd(char **tokens, FILE* masterOutput);
int cd(char **tokens, FILE* masterOutput);



int runExternal(char **tokens, FILE* masterOutput);

/*

  Handles user input

*/
int main (int argc, char** argv) {
    
    char line[MAX_LINE];
    char **tokens;
    
    int i;
    int opt;

    //FILE* masterInput = stdin;
    FILE* masterOutput = stdout;
    
    char* opts[3];
    opts[0] = "exit";
    opts[1] = "cd";
    opts[2] = "pwd";

    // get a line from stdin and get the tokens
    printf("$> ");
    while (fgets(line, MAX_LINE, stdin)) {
 
        opt = -1;
 
        // remove the '\n' from the end of the line
        line[strlen(line)-1] = '\0';
        
        tokens = gettokens(line);

        masterOutput = outCheck(tokens);



        if(tokens[0]){

            //locate index of user command in opts array for switch
            for (i = 0; i < 4; i++){
                if (strcmp(tokens[0], opts[i]) == 0){
                  opt = i;
                }
            }

            switch(opt){
                case 0://exit
#if DEBUG            
printf("exit command found\n");
#endif
                exit(0);
                break;
                case 1://cd
#if DEBUG            
printf("cd command found\n");
#endif
                cd(tokens, masterOutput);
                break;
                case 2://pwd      
#if DEBUG            
printf("pwd command found\n");
#endif
                pwd(tokens, masterOutput);
                break;
                
                default:
#if DEBUG            
printf("external command found\n");
#endif
                runExternal(tokens, masterOutput);


                break;
            }
        }


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







int issueCommand(char**tokens){







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
 
    if((pid = fork()) == -1){ //error
    }else if (pid == 0){ //child
        //TODO set up env?


        //set childs I/O
        dup2(fileno(masterOutput), STDOUT_FILENO);


        //create childs args
        args = getargs(tokens);

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



