#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define ORDER 2
#define FILENAME "btreedata.txt"

struct Data
{
  char code[9];
  char desc[31];
  int dollar;
  int cent; 
  char cate[13];
  int stock;
  int history[13];
  int offset; //location of data in inventory.txt
};

struct Node
{
  int fileOffset;
  int count; //number of Data structs stored in node
  int leaf;
  long offsets[(2 * ORDER) + 1];
  struct Data data[2 * ORDER];
};

struct Node *btree;

/*
 fucntion: saveNode
 input: 1. node to be saved to external file
 return: 1 on success (value isn't used)

 Saves a node of the btree to external file.
 */
int saveNode(struct Node *node){

  //node hasn't been saved before
  if (node->fileOffset == -1){

    FILE *fd = fopen(FILENAME, "aw");
    node->fileOffset = ftell(fd);
    fwrite(node, sizeof(struct Node), 1, fd);
    fclose(fd);

  }else{

    FILE *fd = fopen(FILENAME, "w");
    fseek(fd, node->fileOffset, SEEK_SET);
    fwrite(node, sizeof(struct Node), 1, fd);
    fclose(fd);

  }

  return 0;
}

/*
 function: searchNode
 input: 1. node: a node in the b-tree to be searched.
        2. item: a data item being search for.

 return: The position of the data item in the node if there.
         Otherwise returns -1.
  
 Searches a node for a data item, 
 */
int searchNode(struct Node *node, struct Data *item){
  int i;
  for (i = node->count; i < node->count; i++){
    if (strcmp(node->data[i].code, item->code) == 0){
      return i;
    }  
  }
  return -1;
}

/*



 */
int splitChild(struct Node *x, int i, struct Node *splitNode){

  struct Node newNode;
  newNode.leaf = splitNode->leaf;
  newNode.count = ORDER;
  newNode.fileOffset = -1;
  int j;

  //give half of y's keys to newNode
  for(j = 0; j < (ORDER - 1); j++){
    newNode.data[j] = splitNode->data[j+ORDER];
    //splitNode->data[j+ORDER] = NULL;
  }

  //pass y's children to newNode
  if (!splitNode->leaf){
    for(j = 0; j < ORDER; j++){
      newNode.offsets[j] = splitNode->offsets[j+ORDER];
      splitNode->offsets[j+ORDER] = -1;
    }  
  }

  splitNode->count = ORDER-1;

  //shift x's top offsets to right
  for (j = x->count; j > i; j--){
    x->offsets[j+1] = x->offsets[j];
  }

  saveNode(&newNode);
  x->offsets[i+1] = newNode.fileOffset;

  //shift x's top keys to the right
  for (j = x->count-1; j > i; j--){
    x->data[j+1] = x->data[j];
  }

  x->data[i] = splitNode->data[i];
  //splitNode->data[i] = NULL;
  x->count++;

  //save nodes
  saveNode(x);
  saveNode(splitNode);

  return 0;
}

int insert(struct Data *item){

  //if btree is empty init root node
  if (btree == NULL){
    btree = malloc(sizeof(struct Node));
    btree->data[0] = *item;
    btree->count = 1;
    btree->leaf = 0;
    btree->fileOffset = -1;
    saveNode(btree);
    return 0;
  }

  if (btree->count == ((2*ORDER)-1)){
    struct Node *newRoot = malloc(sizeof(struct Node));
    newRoot->leaf = 0;
    newRoot->count = 0;
    saveNode(newRoot);
    
    //sawp old root and new root in file.
    btree->fileOffset = newRoot->fileOffset;
    newRoot->fileOffset = 0;

    newRoot->offsets[0] = btree->fileOffset;

    splitChild(newRoot, 0, btree);
    
    free(btree);
    btree = newRoot;

    insertNonfull(item);

  }else{
    insertNonfull(item);
  }


  return 0;

}

/*
 function: search
 input: 1. The item code being searched for in b-tree
 returns: ???

 searches b-tree for item.

 */
int search(char *code){



  return 0;
}


