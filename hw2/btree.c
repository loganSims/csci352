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
  int count; //number of Data structs stored in node
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
  FILE *fd = fopen(FILENAME, "aw");
  fwrite(node, sizeof(struct Node), 1, fd);
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
int searchNode(struct Node *node, struct Data *item){
  int i;
  for (i = node->count; i < node->count; i++){
    if (strcmp(node->data[i].code, item->code) == 0){
      return i;
    }  
  }
  return -1;
}


int insert(struct Data *item){

  //if btree is empty init root node
  if (btree == NULL){
    btree = malloc(sizeof(struct Node));
    btree->data[0] = *item;
    btree->count = 1;
    saveNode(btree);
  }


  
  

  //else didn't find node



       //if leaf 
           //if room

           //else split


  return 0;

}




