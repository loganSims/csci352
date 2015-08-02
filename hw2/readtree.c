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

int traverse(struct Node *node){

  int k;
  int i;
  struct Node nextNode;


    printf("fileOffset: %d\n", node->fileOffset);
    printf("Children: ");

    for (k = 0; k < node->count; k++){
      printf("%d ", node->offsets[k]);
    }
    printf("\n");

    for (i = 0; i < node->count; i++){

      printf("node at: %d    ", node->fileOffset);
      printData(node->data[i]);

      if (!(node->leaf)){
          getNode(node->offsets[i], &nextNode);
          traverse(&nextNode);
      }
    }

    if (!(node->leaf)){
      getNode(node->offsets[i], &nextNode);
      traverse(&nextNode);
    }


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

