/*
Logan Sims
CSCI 352
7/11/2015

showenv

Displays the values of one or more environment variables.
The names of the variables can be regexp specified by the 
regexp.c program.

*/

#include "regexp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define BUF_SIZE 1024
#define DEBUG 1
 
/*
   The mane function gathers all the current env
   variables and checks for matches with the users
   input by using the match function from regexp.c
*/
int main(int argc, char *argv[], char** envp){

  //Checks for at least 3 arguments  
  if (argc < 2){
    printf("Usage: showenv VARNAME...\n");
    exit(1);
  }
   
  /*
  Variable set up
  i : a index for argv
  lineNum : counter for the line numbers being read
  regex : the regex given by user
  */
  int i;
  int j;
  char *evarregex; 

  /*
  The 2rd argument and on will be the var or regex for vars
  to be printed,
  This for loop reads through those names/regexp and
  attempts to match them with the vars in envp.
  */


  for(i = 1; argv[i]; i++){
  
    evarregex = argv[i];

    for (j = 0; envp[j]; j++){

      if(match(evarregex, envp[j])){
        printf("%s\n", envp[j]);
      }
    }
    
  }
  return 0;
}
