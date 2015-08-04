/*

   loadbtree.c

   Logan Sims
   CSCI 352
   Assignment 2
   08/05/2015

   Reads inventory.txt and loads btree with data.
   Opens file adn begins reading line by line.
   Each line is processed using functions from btree.c.
   functions used:
     initBtree()
     search()
     insertSearch()
     insert()

   These functions can be found in btree.c

 */

#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024

int main (int argc, char** argv) {

  int start = 1;
  
  char *filename = argv[1];
  char line[BUF_SIZE];

  FILE *fd;

  struct Data item;
  struct Node node;
  struct Node root;

  if (argc < 2){
    printf("usage: ./loadbtree [INVENTORY FILENAME]\n");
    return 0;
  }

  fd = fopen(filename, "r");

  if (fd == NULL){
    printf("File %s not found\n", filename);
  }else{

    while(fgets(line, BUF_SIZE, fd)){

      buildData(&item, line);

      if (start){
        initBtree(&item);
        start = 0;
      }else{
        getNode(0, &root);
        if(search(&root, item.code, &node) != -1){
          printf("Item %s is already in database.\n", item.code);
        }else{
          insertSearch(&root, &item, &node);
          insert(&node ,&item);
        }
      }

    }

  fclose(fd);
  }

  return 0;
}

