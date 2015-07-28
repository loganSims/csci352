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
  printf("root\n");
  for(i = 0; i < root->count; i++){ 
    printf("Code %d: %s\n", i, root->data[i].code);
  }
  
  printf("offsets[0]: %d\n", root->offsets[0]);
  printf("offsets[1]: %d\n", root->offsets[1]);
  printf("offsets[2]: %d\n", root->offsets[2]);
  printf("offsets[3]: %d\n", root->offsets[3]);
  printf("offsets[4]: %d\n", root->offsets[4]);

  printf("Count: %d\n", root->count);
  printf("Stock: %d\n", root->data[0].stock);

  printf("leaf: %d\n\n\n", root->leaf);

  struct Node *node1 = getNode(root->offsets[0]);
  printf("node 1\n");

  for(i = 0; i < node1->count; i++){ 
    printf("Code %d: %s\n", i, node1->data[i].code);
  }

  printf("Count: %d\n", node1->count);

  printf("leaf: %d\n\n\n", node1->leaf);

  struct Node *node2 = getNode(root->offsets[1]);
  printf("node 2\n");

  for(i = 0; i < node2->count; i++){ 
    printf("Code %d: %s\n", i, node2->data[i].code);
  }

  printf("Count: %d\n", node2->count);

  printf("leaf: %d\n\n", node2->leaf);


  struct Node *node3 = getNode(root->offsets[2]);
  printf("node 3\n");

  for(i = 0; i < node3->count; i++){ 
    printf("Code %d: %s\n", i, node3->data[i].code);
  }

  printf("Count: %d\n", node3->count);

  printf("leaf: %d\n", node3->leaf);


  return 0;


}
