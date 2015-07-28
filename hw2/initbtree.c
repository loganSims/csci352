/*
   Logan Sims
   CSCI 352
   Assignment 2
   7/24/2015

   reads inventory.txt and init b-tree

 */

#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define DEBUG 0

int setNumber(struct Data *item, char *line, int *pos);
int setDesc(struct Data *item, char *line, int *pos);
int setPrice(struct Data *item, char *line, int *pos);
int setCategory(struct Data *item, char *line, int *pos);
int setStock(struct Data *item, char *line, int *pos);
int setHist(struct Data *item, char *line, int *pos);

int saveItem(struct Data *item);

int main (int argc, char** argv) {

  char *filename = "inventory.txt";
  char line[BUF_SIZE];

  FILE *fd;

  fd = fopen(filename, "r");

  if (fd == NULL){
    printf("File %s not found\n", filename);
  }else{

    struct Data *item = malloc(sizeof(struct Data));

    while(fgets(line, BUF_SIZE, fd)){


      buildData(item, line);
      //add item to b-tree
      insert(item);


      free(item);
      item = malloc(sizeof(struct Data));

    }

  fclose(fd);
  }
  return 0;
}

