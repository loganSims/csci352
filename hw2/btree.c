#include "btree.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define FILENAME "btreedata.txt"

int initNode(struct Node *node){
  int i;
  node->leaf = 0;
  node->fileOffset = -1;
  node->count = 0;

  for (i = 0; i <= (ORDER*2); i++){
    node->offsets[i] = -1;
  }

  return 0;
}

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
int getNode(int offset, struct Node *node){

  FILE *fd = fopen(FILENAME, "r");

  initNode(node);
  
  lseek(fileno(fd), offset, SEEK_CUR);
  fread(node, sizeof(struct Node), 1, fd);
  fclose(fd);

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
int searchNode(struct Node *node, char *code){
  int i;
  for (i = node->count; i < node->count; i++){
    if (strcmp(node->data[i].code, code) == 0){
      return i;
    }  
  }
  return -1;
}

/*



 */
int splitChild(struct Node *x, int i, struct Node *splitNode){

  struct Node newNode;
  initNode(&newNode);
  newNode.leaf = splitNode->leaf;
  newNode.count = ORDER;
  newNode.fileOffset = -1;

  int j;

  //give some of splitNode's data to newNode
  for(j = 0; j < ORDER; j++){
    newNode.data[j] = splitNode->data[j+ORDER];
  }

  //pass splitNode's children to newNode
  if (!(splitNode->leaf)){
    for(j = 0; j <= ORDER; j++){
      newNode.offsets[j] = splitNode->offsets[j+ORDER];
      splitNode->offsets[j+ORDER] = -1;
    }  
  }

  splitNode->count = ORDER;

  //shift x's offsets to right
  for (j = x->count; j > i; j--){
    x->offsets[j+1] = x->offsets[j];
  }

  saveNode(&newNode);
  x->offsets[i+1] = newNode.fileOffset;

  //shift x's data to the right
  for (j = x->count; j >= i; j--){
    x->data[j] = x->data[j-1];
  }

  x->data[i] = splitNode->data[ORDER-1];
  (splitNode->count)--;
  (x->count)++;

  //save nodes
  saveNode(x);
  saveNode(splitNode);
  saveNode(&newNode);

  return 0;
}

int insertNonfull(struct Node *node, struct Data *item){
 
  int i = 0; 
  struct Node nextNode;
 
  i = node->count;
  if (node->leaf){
    while((i >= 1)&&(strcmp(item->code, node->data[i-1].code) < 0)){
      node->data[i] = node->data[i-1];
      i--;
    }
    node->data[i] = *item;
    node->count = (node->count) + 1;
    saveNode(node);
  }else{ 
    while(( i>=1 )&&(strcmp(item->code, node->data[i-1].code) < 0)){
      i--;
    } 
 
    getNode(node->offsets[i], &nextNode); 
 
    if (nextNode.count == (2*ORDER)){
      splitChild(node, i, &nextNode);
      if (strcmp(item->code, node->data[i].code) > 0){
        i++;
      }
    }

    getNode(node->offsets[i], &nextNode);

    insertNonfull(&nextNode, item);
  }

  return 0;
}


int initBtree(struct Data *item){

  struct Node btree;
  initNode(&btree);
  btree.data[0] = *item;
  btree.count = 1;
  btree.leaf = 1;
  btree.fileOffset = -2;
  saveNode(&btree);
  return 0;

}


int insert(struct Data *item){

  struct Node root;  
  struct Node newRoot;

  getNode(0, &root);

  if (root.count == (2*ORDER)){
    
    initNode(&newRoot);
    saveNode(&newRoot);
 
    //sawp old root and new root in file.
    root.fileOffset = newRoot.fileOffset;
    newRoot.fileOffset = 0;
    
    newRoot.offsets[0] = root.fileOffset; 

    splitChild(&newRoot, 0, &root);
    
    insertNonfull(&newRoot, item);
     
  }else{
    insertNonfull(&root, item);
  }

  return 0;
}

/*
 function: search
 input: 1. The item code being searched for in b-tree
 returns: index of data item in found node. 

 searches b-tree for item. sets found to the node with the 
 data item.

 */
int search(struct Node *node, char *code, struct Node *found){
  int i = 0;
  while ((i < node->count) && (strcmp(code, node->data[i].code) > 0 )){
    i++;
  }

  if ((i < node->count) && (strcmp(code, node->data[i].code) == 0)){
    *found = *node;
    return i;
  }

  if (node->leaf){ 
    return -1;
  }else{
    struct Node nextNode;
    getNode(node->offsets[i], &nextNode);
    return search(&nextNode, code, found);  
  }
}

/*
 function: removeKey
 input: 1. The code of item to be removed.
        2. The node where the code is to be removed from.
           must be a leaf node.
 return: 0 on success.
  

 */
int removekey(char *code, struct Node *node){

  int dataIndex = searchNode(node, code);  
  int i;

  for(i = dataIndex; i < ((node->count) - 1); i++){

    node->data[i] = node->data[i+1];

  } 

  (node->count)--;
  saveNode(node);
  return 0;
}




/*
 function: getParentOffset
 input: 1. The candidate for childs parent, starts as root.
        2. The code for the first data item in child.
        3. The node whos parent is being searched for.
 return: offset of parent node, -1 on failure.
  
 */
int getParentOffset(struct Node *node, char* code, struct Node *child){

  //node has no children, therefore child cannot be it's child
  if (node->leaf){
    return -1;
  }
 
  int i = 0;
  while ((i < node->count) && (strcmp(code, node->data[i].code) > 0 )){
    i++;
  }

  if (node->offsets[i] == child->fileOffset){
    return node->fileOffset;
  }else{
    struct Node nextNode;
    getNode(node->offsets[i], &nextNode);
    return getParentOffset(&nextNode, code, child);  
  }

}


/*
 function: getSibOffset
 input: 1. The node who's sibling we want.
        2. Specifies which sibling.
           "right" for right sibbling.
            otherwise left sibling
 return: offset of sibling node, -1 on failure.
  
 If there is no sibling to right the offset will be -1.

 */
int getSibOffset(struct Node *node, char *choice){

  //Part (a) Setup.

  int i = 0;
  int parentOffset;
  struct Node parent;
  struct Node root;
  

  getNode(0, &root);

  parentOffset = getParentOffset(&root, node->data[0].code, node);

  // Part (b) Locate nodes offset in parent.
  if (parentOffset == -1){
    return -1;
  }

  getNode(parentOffset, &parent);

  while ((i < parent.count) && 
         (strcmp(node->data[0].code, parent.data[i].code) > 0 )){
    i++;
  }
  
  // Part (c) Return requested sibling.
  // Also checks that node can have left/right sibling. 
  if (strcmp(choice, "right") == 0){
    if ((i+1) > parent.count){
      return -1;
    }else{
      return parent.offsets[i+1];
    }
  }else{
    if ((i-1) >= 0){
      return parent.offsets[i-1];
    }else{
      return -1;
    }
  }
}


/* Beginning of inventory item set functions */


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


  while (line[i] == ' '){i++;}

  while ((line[i] != ' ') && (line[i] != '\n')){
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
 
  if ((line[i] != '\n') && (line[i] != EOF)){

    while(line[i] == ' '){i++;} //advance to stock 

    while(line[i] != ' '){
      stock[j] = line[i];
      j++;
      i++;
    }

    stock[j] = '\0';

    item->stock = atoi(stock);

    *pos = i;

  }else{
    item->stock = 0;
  }

  return 0;
}


int setHist(struct Data* item, char* line, int* pos){
  
  int i = *pos;
  int j = 0;
  int k = 0; //month
  char sale[5];

  if ((line[i] != '\n') && (line[i] != EOF)){

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

  }else{
    for (j = 0; j < 12; j++){
      item->history[j] = 0;
    }
    item->history[j] = '\0';
  }
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
