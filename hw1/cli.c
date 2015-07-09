#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

#define MAX_LINE  1024
#define DEBUG     1


FILE* outCheck(char **tokens);
int myprint(char** tokens, char* output, int newline);

int pwd(char** tokens);
int cd(char** tokens);

/*

  Handles parsing user input

*/
int main (int argc, char** argv) {
    
    char line[MAX_LINE];
    char **tokens;
    
    int i;
    int opt;

    
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
                cd(tokens);
                break;
                case 2://pwd      
#if DEBUG            
printf("pwd command found\n");
#endif
                pwd(tokens);
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
  Performs cd command



  TODO BUG: multiple calls to myprint causes newline with
  each open to file.
*/

int cd(char** tokens){

   char *dir = tokens[1];
   char succ[MAX_LINE];
   char fail[MAX_LINE];
   char *output;

   strcpy(succ, "cwd changed to ");
   strcpy(fail, "No such directory: ");

   if (!chdir(dir)){
      myprint(tokens, succ, 0);
      pwd(tokens);
      myprint(tokens, "", 1);
   }else{
      output = strcat(fail, dir);
      myprint(tokens, output, 1);
   }

   return 0;
}

/*
  Performs pwd command


*/

int pwd(char** tokens){

    char *cwd;
    char buff[MAX_LINE];
    size_t size = MAX_LINE;

    cwd = getcwd(buff, size);

    myprint(tokens, cwd, 0);
    return 0;

}










/*
   prints based on users stdout choice

*/

int myprint(char** tokens, char* output, int newline){

  FILE *out;

  if((out = outCheck(tokens))){
    if(newline){
      fprintf(out, "%s", output);
    }else{
      fprintf(out, "%s\n", output);
    }
    fclose(out);
  }else{
    if(newline){
      printf("%s\n", output);
    }else{ 
      printf("%s", output);
    }
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
  return NULL;

}



