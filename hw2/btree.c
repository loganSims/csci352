#include <stdio.h>
#include <string.h>

#define ORDER 2

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




int insert(struct Node *root, struct Data *item){

  //search for item
  searchNode(root, item);
  //if search ends and leaf has room, add data item.

  //


  return 0;

}




