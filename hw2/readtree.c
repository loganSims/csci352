#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define DEBUG 0

int traverse(struct Node *node);
int printData(struct Data data);

int main (int argc, char** argv) {

  struct Node *root = getNode(0);

  traverse(root);   

  return 0;

}

int traverse(struct Node *node){

  int i;


    for (i = 0; i < node->count; i++){


      if (!(node->leaf)){
          traverse(getNode(node->offsets[i]));
      }
      printData(node->data[i]);

    }

    if (!(node->leaf)){
        traverse(getNode(node->offsets[i]));
    }

  free(node);
  return 0;
}

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

