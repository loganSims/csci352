/*
  readtree.c

  Logan Sims
  CSCI 352
  Assignment 2
  08/05/2014

  Prints out tree with in order traversal. 

  lists node offset in btree.data file, followed by
  nodes children, followed by data. Each data element printed
  lists the offset in the btree file of the node it is stored in. 

 */
#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024

int traverse(struct Node *node);
int printData(struct Data data);

int main (int argc, char** argv) {

  struct Node root; 
  getNode(0, &root);

  traverse(&root);   

  return 0;

}

/*
  function: traverse
  Input: node: starts as root of btree.
  Return: 0 on complete (value not used)

  Performs in order traversal.

 */
int traverse(struct Node *node){

  int k;
  int i;
  struct Node nextNode;


    printf("fileOffset: %d", node->fileOffset);
    printf(" | Children: ");

    for (k = 0; k <= node->count; k++){
      printf("%d ", node->offsets[k]);
    }
    printf("\n");

    for (i = 0; i < node->count; i++){

      if (node->offsets[0] != -1){
        if(node->offsets[i] != -1){
          getNode(node->offsets[i], &nextNode);
          traverse(&nextNode);
        }
      }

      printf("In node: %d  ", node->fileOffset);
      printData(node->data[i]);

    }

    if (node->offsets[0] != -1){
      if(node->offsets[i] != -1){
        getNode(node->offsets[i], &nextNode);
        traverse(&nextNode);
      }
    }


  return 0;
}

/*
  function: printData
  Input: data: item to be printed
  Returns: 0 on complete. (value not used)

  prints out information about data item.

 */
int printData(struct Data data){

  int i;

  printf("%s", data.code);
  printf("%s", data.desc);
  printf("%d.", data.dollar);
  printf("%d", data.cent);
  printf("%s", data.cate);
  printf("%d ", data.stock);
  
  for (i = 0; i < 12; i++){
    printf("%d ", data.history[i]);
  }

  printf("\n");  

  return 0;
}

