/*
   Logan Sims
   CSCI 352
   Assignment 2
   7/24/2015

   reads transactions.txt and edits a b-tree

 */

#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define DEBUG 0


int main (int argc, char** argv) {


  if (argc < 2){
    printf("usage: ./editbtree [TRANSACTIONS FILE]\n");
    return 0;
  }

  char *filename = argv[1];
  char line[BUF_SIZE];

  FILE *fd;


  fd = fopen(filename, "r");

  if (fd == NULL){
    printf("File %s not found\n", filename);
  }else{
    while(fgets(line, BUF_SIZE, fd)){

    }
  }


  fclose(fd);
  
  return 0;
}

