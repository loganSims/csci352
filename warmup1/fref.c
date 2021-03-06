/*
Logan Sims
CSCI 352
6/27/2015

Warm-up: fref

Using the match function in regexp.c from "Beautiful Code",
this program searches through one of more files and displays 
any lines that contain a specified regular expression.
*/

#include "regexp.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define thing(X) #X

#define BUF_SIZE 1024
#define DEBUG 1
 
/*
fref.c using the main function to open and read files.
It works like a wrapper for the match function 
to find regular expressions in files.
*/
int main(int argc, char *argv[]){


  //Checks for at least 3 arguments  
  if (argc < 3){
    printf("Usage: fref REGEX FILE...\n");
    exit(1);
  }
   
  /*
  Variable set up
  i : a index for argv
  lineNum : counter for the line numbers being read
  regex : the regex given by user
  */
  int i;
  int lineNum;
  char *regex = argv[1]; 
  char line[BUF_SIZE];
  FILE *fileptr;

  /*
  The 3rd argument and on will be filenames,
  This for loop reads through those filenames and
  attempts to open them.
  */
  for(i = 2; argv[i]; i++){

    fileptr = fopen(argv[i], "r");

    if (fileptr == NULL){
      printf("File %s not found\n", argv[i]);
    }else{

      lineNum = 1;

      /*
      once file has been opened read 
      line by line looking for regex
      */
      while(fgets(line, BUF_SIZE, fileptr)){
        if(match(regex, line)){
          printf("%s:%d %s", argv[i], lineNum, line);
        }
        lineNum++;
      }
      fclose(fileptr);
    }
  }
  return 0;
}
