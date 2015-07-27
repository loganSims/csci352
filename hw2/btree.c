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
  long fileOffset;
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

  //node is root
  if (node->fileOffset == -2){
    FILE *fd = fopen(FILENAME, "w");
    node->fileOffset = 0;
    fwrite(node, sizeof(struct Node), 1, fd);
    fclose(fd);

  //node hadnt been saved yet
  }else if (node->fileOffset == -1){

    FILE *fd = fopen(FILENAME, "a");
    node->fileOffset = ftell(fd);
    fwrite(node, sizeof(struct Node), 1, fd);
    fclose(fd);

  }else{

    FILE *fd = fopen(FILENAME, "r+");
    fseek(fd, node->fileOffset, SEEK_SET);
    fwrite(node, sizeof(struct Node), 1, fd);
    fclose(fd);

  }

  return 0;
}


/*
 fucntion: getNode
 input: 1. offset of node in file.
 return: Pointer to the node at offset.

 Searches btree data file for a node.
 */
struct Node *getNode(long offset){

  FILE *fd = fopen(FILENAME, "r");
  
  fseek(fd, offset, SEEK_SET);
  struct Node *node = malloc(sizeof(struct Node));
  fread(node, sizeof(struct Node), 1, fd);
  fclose(fd);

  return node;
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

  struct Node *newNode = malloc(sizeof(struct Node));
  newNode->leaf = splitNode->leaf;
  newNode->count = (ORDER - 1);
  newNode->fileOffset = -1;

  int j;

  //give some of splitNode's keys to newNode
  for(j = 0; j < (ORDER - 1); j++){
    newNode->data[j] = splitNode->data[j+ORDER];
  }

  //pass splitNode's children to newNode
  if (!(splitNode->leaf)){
    for(j = 0; j < ORDER; j++){
      newNode->offsets[j] = splitNode->offsets[j+ORDER];
      splitNode->offsets[j+ORDER] = -1;
    }  
  }

  splitNode->count = (ORDER - 1);

  //shift x's top offsets to right
  for (j = x->count; j > i; j--){
    x->offsets[j+1] = x->offsets[j];
  }

  saveNode(newNode);
  x->offsets[i+1] = newNode->fileOffset;

  //shift x's top keys to the right
  for (j = x->count-1; j > i; j--){
    x->data[j+1] = x->data[j];
  }

  x->data[i] = splitNode->data[ORDER-1];
  x->count = ((x->count) + 1);

  //save nodes
  saveNode(x);
  saveNode(splitNode);
  saveNode(newNode);

  return 0;
}

int insertNonfull(struct Node *node, struct Data *item){
  int i = 0; 
  i = node->count;
  if (node->leaf){
    while((i>=1)&&(strcmp(item->code, node->data[i-1].code) < 0)){
      node->data[i] = node->data[i-1];
      i--;
    }
    node->data[i] = *item;
    (node->count)++;
    saveNode(node);
  }else{ 
    while((i>=1)&&(strcmp(item->code, node->data[i-1].code) < 0)){
      i--;
    }

    struct Node *nextNode = getNode(node->offsets[i]); 

 
    if (nextNode->count == ((2*ORDER)-1)){
      splitChild(node, i, nextNode);
      if (strcmp(item->code, node->data[i].code) > 0){
        i++;
      }
    }

    insertNonfull(nextNode, item);

    free(nextNode);
  }
  return 0;
}


int insert(struct Data *item){

  //if btree is empty init root node
  if (btree == NULL){
    printf("setting up btree\n");
    btree = malloc(sizeof(struct Node));
    btree->data[0] = *item;
    btree->count = 1;
    btree->leaf = 1;
    btree->fileOffset = -2;
    saveNode(btree);
    return 0;
  }

  if (btree->count == ((2*ORDER)-1)){

    struct Node *newRoot = malloc(sizeof(struct Node));
    newRoot->leaf = 0;
    newRoot->count = 0;
    newRoot->fileOffset = -1;
    saveNode(newRoot);
 
    //sawp old root and new root in file.
    btree->fileOffset = newRoot->fileOffset;
    newRoot->fileOffset = 0;
    
    newRoot->offsets[0] = btree->fileOffset; 

    splitChild(newRoot, 0, btree);
    
    insertNonfull(newRoot, item);
    
    btree = getNode(0);

  }else{
    insertNonfull(btree, item);
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


