#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

#define MAX_LINE  1024
#define DEBUG     1


FILE* outCheck(char **tokens);

int myprint(char** tokens, char* output);


/*

  Handles parsing user input

*/
int main (int argc, char** argv) {
    
    char line[MAX_LINE];
    char **tokens;
    
    int i;
    int opt;

    char *cwd;
    char buff[MAX_LINE];
    size_t size = MAX_LINE;
    
    char* opts[4];
    opts[0] = "exit";
    opts[1] = "cd";
    opts[2] = "pwd";
    opts[3] = "showenv";

    // get a line from stdin and get the tokens
    printf("$> ");
    while (fgets(line, MAX_LINE, stdin)) {
 
        opt = -1;
 
        // remove the '\n' from the end of the line
        line[strlen(line)-1] = '\0';
        
        // get and display the tokens in line
        tokens = gettokens(line);

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




                break;
                case 2://pwd      
#if DEBUG            
printf("pwd command found\n");
#endif
                cwd = getcwd(buff, size);
                myprint(tokens, cwd);
                break;
                case 3://showenv
#if DEBUG            
printf("showenv command found\n");
#endif



                break;
                default:
#if DEBUG            
printf("external command found\n");
#endif



                break;
            }
        }
        free(tokens);
        printf("$> ");
    }
    return 0;
}

/*
   prints based on users stdout choice

*/

int myprint(char** tokens, char* output){

  FILE *out;

  if((out = outCheck(tokens))){
    fprintf(out, "%s", output);
    fclose(out);
  }else{
    printf("%s", output);
  }

  return 0;
}


/*

  returns file desc of open file, 
  returns null if user did not change stdout.

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
        if((newout = fopen(&tokens[i][1],"w"))){
          return newout;
        }else{
          printf("cannot open file %s to write\n", &tokens[i][1]);
        }
      }else{//space after
        if((newout = fopen(tokens[i+1],"w"))){
          return newout;
        }else{
          printf("cannot open file %s to write\n", tokens[i+1]);
        }
      } 
    }
  }
  return NULL;

}











