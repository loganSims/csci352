#include "btree.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define FILENAME "btreedata.txt"


int getParentOffset(struct Node *node, char* code, struct Node *child);
int adjustUnderflow(struct Node *node);

int initNode(struct Node *node){
  int i;
  //node->leaf = 1;
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
  for (i = 0; i < node->count; i++){
    if (strcmp(node->data[i].code, code) == 0){
      return i;
    }  
  }
  return -1;
}


int insertSearch(struct Node *node, struct Data *item, struct Node *found){
 
  int i = 0; 
  struct Node childNode;

  if (node->offsets[0] == -1){
    *found = *node;
    return 0;
  }else{ 

    while((i < node->count) && (strcmp(item->code, node->data[i].code) > 0)){
      i++;
    }

    getNode(node->offsets[i], &childNode);
    insertSearch(&childNode, item, found);

  }

  return 0;
}


int initBtree(struct Data *item){

  struct Node btree;
  initNode(&btree);
  btree.data[0] = *item;
  btree.count = 1;
  btree.fileOffset = -2;
  saveNode(&btree);
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

  if (node->offsets[i] == -1){ 
    return -1;
  }else{
    struct Node nextNode;
    getNode(node->offsets[i], &nextNode);
    return search(&nextNode, code, found);  
  }
}

int insertItem(struct Node *node, struct Data *item){

  int i = node->count;

  while((i >= 1)&&(strcmp(item->code, node->data[i-1].code) < 0)){
    node->data[i] = node->data[i-1];
    i--;
  }
  node->data[i] = *item;
  node->count = (node->count) + 1;

  saveNode(node);

  return 0;
}

int rebalance(struct Node *node){

  int i;
  struct Node child;

  for(i = 0; i < node->count; i++){
    if(!(node->offsets[0] == -1)){
      if ((node->offsets[i] != -1) && (node->offsets[i] != 0)){
        getNode(node->offsets[i], &child);
        rebalance(&child);
      }
    }else{
      if (node->count < ORDER){
        adjustUnderflow(node);
      }
    }
  }
  return 0;
}

int adjustOverflow(struct Node *node, struct Data *item){

  int i = 0;
  int j = 0;
  int added = 0;

  struct Node root;
  struct Node left;
  struct Node right;
  struct Node parent;

  struct Data parentItem;

  int parentOffset;

  struct Data overflowNode[(2*ORDER)+2];

  getNode(0, &root);

  parentOffset = getParentOffset(&root, node->data[0].code, node);
  //root overflow, create new root and make parent of old root.
  if (parentOffset == -1){
    initNode(&parent);         
    saveNode(&parent);

    node->fileOffset = parent.fileOffset;
    parent.fileOffset = 0;
    parent.offsets[0] = node->fileOffset;
    parent.count = 0;
   
    saveNode(node); 
    saveNode(&parent);

  }else{
    getNode(parentOffset, &parent);
  }

  initNode(&left);
  initNode(&right);

  // Make overflowNode

  for (i= 0; i < (ORDER*2); i++){

    if (strcmp(item->code, node->data[j].code) > 0){
      overflowNode[i] = node->data[j];
      j++;
    }else if(!(added)){
      overflowNode[i] = *item;
      added = 1;
    }else{ 
      overflowNode[i] = node->data[j];
      j++;
    }

  }

  if (!(added)){
    overflowNode[(ORDER*2)] = *item;
  }else{
    overflowNode[(ORDER*2)] = node->data[j];
  }
  
  //divide up children of node to left and right if not leaf
  if (node->offsets[0] != -1){
    for (j = 0; j <= ORDER; j++){
      left.offsets[j] = node->offsets[j];
    }
    // NOTE: this is for the case
    // where a full parent was given a data item and had 
    // to use its overflow offset.
    j = 0;
    while((j+ORDER) < ((ORDER*2)+1)){
      right.offsets[j] = node->offsets[j+ORDER+1];
      j++;
    }
  }

  // Add data to left and right
  // skipping middle value for parent
  i = 0;
  for(j = 0; j < ORDER; j++){
    left.data[j] = overflowNode[i];   
    left.count++;   
    i++;
  }
  left.fileOffset = node->fileOffset;
  saveNode(&left);
  
  parentItem = overflowNode[i];
  i++;

  for(j = 0; j < ORDER; j++){
    right.data[j] = overflowNode[i];    
    right.count++;  
    i++;
  }
  saveNode(&right);

  i = 0;
  // Locate node offset index in parent. 
  for(j = 0; j <= parent.count; j++){
    if(parent.offsets[j] == node->fileOffset){
      i = j;
    }
  }

  // Shift parent offsets to make room from left and right
  for (j = parent.count; j > i; j--){
    parent.offsets[j+1] = parent.offsets[j];
  }

  //fprintf(stderr, "index in parent for right: %d\n", (i+1));
  //fprintf(stderr, "index in parent for left: %d\n", (i));
  parent.offsets[i+1] = right.fileOffset;
  parent.offsets[i] = left.fileOffset;

  saveNode(&parent);

  insert(&parent, &parentItem);


  return 0;
}

int insert(struct Node *insertNode, struct Data *item){

  if(insertNode->count < (ORDER*2)){
    insertItem(insertNode, item);
  }else{
    adjustOverflow(insertNode, item);
  }
  return 0;
}



/*
 function: removeKey
 input: 1. The code of item to be removed.
        2. The node where the code is to be removed from.
           MUST BE A LEAF NODE.
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

  // child is root
  if (child->fileOffset == 0){
    return -1;
  }

  //node has no children, therefore child cannot be it's child
  if (node->offsets[0] == -1){
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


int adjustUnderflow(struct Node *node){


  char *left = "left";
  char *right = "right";
  int pickedleft;
  int i = 0;
  int k = 0;

  int leftSibOffset;
  int rightSibOffset;
  int parentOffset;
  int parentIndex = 0;
  
  int j = 0; //combindedNode index
  struct Data combinedNode[2*(2*ORDER)]; //TODO make this better

  struct Node leftsib;
  struct Node rightsib;
  struct Node parent;

  struct Node merger;

  struct Node root;

  getNode(0, &root);

  // Part (a) Find more populous sibling
  leftSibOffset = getSibOffset(node, left);
  rightSibOffset = getSibOffset(node, right);

  if (leftSibOffset != -1){
     getNode(leftSibOffset, &leftsib);
  } 

  if (rightSibOffset != -1){
     getNode(rightSibOffset, &rightsib);
  } 

  //item has no siblings, can be delete with no adjustment
  if ((rightSibOffset == -1) && (leftSibOffset == -1)){
    return 0;
  }else if (rightSibOffset == -1){
    getNode(leftSibOffset, &merger);
    pickedleft = 1;
  }else if (leftSibOffset == -1){
    getNode(rightSibOffset, &merger);
    pickedleft = 0;
  }else{
    if(leftsib.count > rightsib.count){
      getNode(leftSibOffset, &merger);
      pickedleft = 1;
    }else{
      getNode(rightSibOffset, &merger);
      pickedleft = 0;
    }
  }

  // Part (b) Get item from parent for merge
  parentOffset = getParentOffset(&root, node->data[0].code, node);
  getNode(parentOffset, &parent);
 
  i = 0;
  while ((strcmp(node->data[0].code, parent.data[i].code) > 0) && 
        (i < parent.count)){
    parentIndex++;
    i++;
  }

  if (pickedleft){
    parentIndex--;
  }
  
  // Part (c) Build combinedNode.
  
  // build combinedNode.
  if(pickedleft){

    for (i = 0; i < merger.count; i++){
      combinedNode[j] = merger.data[i];
      j++;
    }

    combinedNode[j] = parent.data[parentIndex];
    j++;

    for (i = 0; i < node->count; i++){
      combinedNode[j] = node->data[i];
      j++;
    }

  }else{

    for (i = 0; i < node->count; i++){
      combinedNode[j] = node->data[i];
      j++;
    }

    combinedNode[j] = parent.data[parentIndex];
    j++;

    for (i = 0; i < merger.count; i++){
      combinedNode[j] = merger.data[i];
      j++;
    }

  }


  // Part (d) Add combinedNode to b-tree.
 
  // node chosen to merger has more than ORDER values,
  // need to dived combinedNode in half.
  if (merger.count > ORDER){

    // Assign values from combinedNode to nodes.
    if(pickedleft){

      merger.count = 0;
      for (i = 0; i < (j/2); i++){
        merger.data[i] = combinedNode[i];
        (merger.count)++;
      }

      parent.data[parentIndex] = combinedNode[j/2];
      i++;

      node->count = 0;
      while (i < j){
        node->data[k] = combinedNode[i];
        (node->count)++;
        k++;
        i++;
      }

    }else{

      node->count = 0;
      for (i = 0; i < (j/2); i++){
        node->data[i] = combinedNode[i];
        (node->count)++;
      }

      parent.data[parentIndex] = combinedNode[j/2];
      i++;

      merger.count = 0;
      while (i < j){
        merger.data[k] = combinedNode[i];
        (merger.count)++;
        k++;
        i++;
      }

    }

    saveNode(node);
    saveNode(&parent);
    saveNode(&merger);

  // Node chosen to merge has ORDER values,
  // combinedNode can function as a node.
  }else{

    if (pickedleft) {
      // Give combined node data
      for (i = 0; i < (ORDER*2); i++){
        merger.data[i] = combinedNode[i];
      }
      // Give all children
      i = 0;
      while(i <= merger.count){
        i++;
      }
      j = 0;
      while(j <= node->count){
        merger.offsets[i] = node->offsets[j];
        i++;
        j++;
      }
      merger.count = (ORDER*2);
      saveNode(&merger);
    }else{
      // Give combined node data
      for (i = 0; i < (ORDER*2); i++){
        node->data[i] = combinedNode[i];
      }
      // Give all children
      i = 0;
      while(i <= node->count){
        i++;
      }
      j = 0;
      while(j <= merger.count){
        node->offsets[i] = merger.offsets[j];
        i++;
        j++;
      }

      node->count = (ORDER*2);
      saveNode(node);
    }
    //shift offsets and data left
    for (i = ((parent.count) - 1); i > parentIndex; i--){
      parent.data[i-1] = parent.data[i];
      parent.offsets[i] = parent.offsets[i+1];
    }
    
    parent.count--;

    //if parent has less than ORDER items and isn't root.
    if ((parent.count < ORDER) && (parent.fileOffset != 0)){
      saveNode(&parent);
      adjustUnderflow(&parent);
      saveNode(&parent);
    }else if((parent.fileOffset == 0) && (parent.count == 0)){
      if (pickedleft) {
        merger.fileOffset = 0;
        saveNode(&merger);
      }else{
        node->fileOffset = 0;
        saveNode(node);
      }
    }else{	
      saveNode(&parent);
    }


  }
  return 0;
}


int deleteKey(struct Node *node, char *code){

  struct Node root;
  struct Node maxnode;

  struct Data k1;
  struct Data k2;

  getNode(0, &root);

  // Part (a) Swap item for delete with greatest item in left subtree.
  if (node->offsets[0] != -1){
    
    int dataIndex =  searchNode(node, code);
    getNode(node->offsets[dataIndex], &maxnode);
   
    k1 = node->data[dataIndex];

    while(maxnode.offsets[0] != -1){
      getNode(maxnode.offsets[maxnode.count], &maxnode);     
    }

    k2 = maxnode.data[((maxnode.count) - 1)];

    node->data[dataIndex] = k2;
    maxnode.data[((maxnode.count) - 1)] = k1;

    saveNode(node);
    saveNode(&maxnode);

    node = &maxnode;

  }

  // Part (b) Remove item, check for underflow
  removekey(code, node);

  if ((node->count < ORDER) && (node->fileOffset != 0)){
    adjustUnderflow(node);    
  }

  if ((node->fileOffset == 0) && 
      (node->count == 0) &&
      (node->offsets[0] != -1)){

    getNode(node->offsets[0], node);
    node->fileOffset = 0;

    saveNode(node);

  }

  return 0;

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
  item->padding = 0;
  return 0;
}
