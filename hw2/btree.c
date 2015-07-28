#include "btree.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define FILENAME "btreedata.txt"

struct Node *btree;
struct Node *found;

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

    FILE *fd = fopen(FILENAME, "r+");
    lseek(fileno(fd), 0, SEEK_END);
    node->fileOffset = lseek(fileno(fd), 0, SEEK_END);
    fwrite(node, sizeof(struct Node), 1, fd);
    fclose(fd);

  }else{

    FILE *fd = fopen(FILENAME, "r+");
    lseek(fileno(fd), node->fileOffset, SEEK_SET);
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
struct Node *getNode(int offset){

  FILE *fd = fopen(FILENAME, "r");
  
  lseek(fileno(fd), offset, SEEK_CUR);
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
  newNode->count = ORDER;
  newNode->fileOffset = -1;

  int j;

  //give some of splitNode's data to newNode
  for(j = 0; j < ORDER; j++){
    newNode->data[j] = splitNode->data[j+ORDER];
    (splitNode->count)--;
  }

  //pass splitNode's children to newNode
  if (!(splitNode->leaf)){
    for(j = 0; j <= ORDER; j++){
      newNode->offsets[j] = splitNode->offsets[j+ORDER];
      splitNode->offsets[j+ORDER] = -1;
    }  
  }

  //shift x's offsets to right
  for (j = x->count; j > i; j--){
    x->offsets[j+1] = x->offsets[j];
  }

  saveNode(newNode);
  x->offsets[i+1] = newNode->fileOffset;

  //TODO
  //shift x's data to the right
  for (j = x->count; j >= i; j--){
    x->data[j+1] = x->data[j];
  }

  x->data[i] = splitNode->data[ORDER-1];
  (splitNode->count)--;
  (x->count)++;

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
    while((i >= 1)&&(strcmp(item->code, node->data[i-1].code) < 0)){
      node->data[i] = node->data[i-1];
      i--;
    }
    node->data[i] = *item;
    (node->count)++;
    saveNode(node);
  }else{ 
    while(( i>=1 )&&(strcmp(item->code, node->data[i-1].code) < 0)){
      i--;
    }
    
    struct Node *nextNode = getNode(node->offsets[i]); 
 
    if (nextNode->count == (2*ORDER)){
      splitChild(node, i, nextNode);
      if (strcmp(item->code, node->data[i].code) > 0){
        i++;
      }
    }

    insertNonfull(getNode(node->offsets[i]), item);

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

  if (btree->count == (2*ORDER)){
    
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
int search(struct Node *node, char *code, struct Node *found){
  int i = 0;
  while ((i < node->count) && (strcmp(code, node->data[i].code) > 0 )){
    i++;
  }

  if ((i <= node->count) && (strcmp(code, node->data[i].code) == 0)){
    found = node;
    return i;
  }

  if (node->leaf){ 
    return -1;
  }else{
    return search(getNode(node->offsets[i]), code, found);  
  }
}

//////////////////////////////




int setNumber(struct Data* item, char* line, int* pos){
  
  int i;

  for (i = 0; i < 8; i++){
    item->code[i] = line[i];
  }

  item->code[i] = '\0';

  *pos = i;

  return 0;
}

int setDesc(struct Data* item, char* line, int *pos){

  int i = *pos;
  int j = 0;

  for(i = *pos; j < 30; i++){
    item->desc[j] = line[i];
    j++;
  }

  *pos = i;
  item->desc[j] = '\0';

  return 0;
};

int setPrice(struct Data* item, char* line, int* pos){

  int i = *pos;
  int j = 0;

  char dollar[5];
  char cent[2]; 

  while(line[i] != '.'){
    dollar[j] = line[i];
    i++;
    j++;
  }

  item->dollar = atoi(dollar);

  cent[0] = line[i+1];
  cent[1] = line[i+2];

  *pos = i+3;

  item->cent = atoi(cent);

  return 0;
};


int setCategory(struct Data* item, char* line, int* pos){

  int i = *pos;
  int j = 0;

  while (line[i] != ' '){
    item->cate[j] = line[i];
    j++;
    i++;
  }

  while (j < 11){
    item->cate[j] = ' ';
    j++;
  }

  item->cate[j] = '\0';

  *pos = i;

  return 0;
};


int setStock(struct Data* item, char* line, int* pos){
  
  int i = *pos;
  int j = 0;
  char stock[4];

  while(line[i] == ' '){i++;} //advance to stock 

  while(line[i] != ' '){
    stock[j] = line[i];
    j++;
    i++;
  }

  stock[j] = '\0';

  item->stock = atoi(stock);

  *pos = i;

  return 0;
}


int setHist(struct Data* item, char* line, int* pos){
  
  int i = *pos;
  int j = 0;
  int k = 0; //month
  char sale[5];

  while(k < 12){
    j = 0;

    while(line[i] == ' '){i++;} //advance next int

    while(line[i] != ' ' && line[i] != EOF && line[i] != '\n'){
      sale[j] = line[i];
      j++;
      i++;
    }

    sale[j] = '\0';

    item->history[k] = atoi(sale);

    k++;

  }

  item->history[k] = '\0';
  return 0;
}

int saveItem(struct Data *item){

  FILE *fd = fopen("data.txt", "aw");

  fwrite(item, sizeof(struct Data), 1, fd);

  fclose(fd);

  return 0;

}

int buildData(struct Data *item, char *line){

  int p = 0;
  int *pos = &p;

  setNumber(item, line, pos);
  setDesc(item, line, pos);
  setPrice(item, line, pos);      
  setCategory(item, line, pos);      
  setStock(item, line, pos);    
  setHist(item, line, pos);

  return 0;
}
