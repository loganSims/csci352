/*
Logan Sims
CSCI 352
6/27/2015

Warm-up: fref

Using the match function in regexp.c from "Beautiful Code",
this program searches through one of more files and displays 
any lines that contain a specified regular expression.
*/

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

  int x = 2;
  int y = 3;
  printf("%s\n", thing(x+y));
  return 0;
}
