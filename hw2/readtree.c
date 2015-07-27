#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define DEBUG 0


int main (int argc, char** argv) {

  int i;

  printf("page size: %lu\n", sizeof(struct Node));

  struct Node *root = getNode(0);
  for(i = 0; i < root->count; i++){ 
    printf("Code %d: %s\n", i, root->data[i].code);
  }
  
  printf("offsets[0]: %lu\n", root->offsets[0]);
  printf("offsets[1]: %lu\n", root->offsets[1]);

  printf("Count: %d\n", root->count);

  printf("leaf: %d\n", root->leaf);

  struct Node *node1 = getNode(root->offsets[0]);

  for(i = 0; i < node1->count; i++){ 
    printf("Code %d: %s\n", i, node1->data[i].code);
  }

  printf("Count: %d\n", node1->count);

  printf("leaf: %d\n", node1->leaf);

  struct Node *node2 = getNode(root->offsets[1]);

  for(i = 0; i < node2->count; i++){ 
    printf("Code %d: %s\n", i, node2->data[i].code);
  }

  printf("Count: %d\n", node2->count);

  printf("leaf: %d\n", node2->leaf);
  return 0;


}