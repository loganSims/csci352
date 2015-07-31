/*
   Logan Sims
   CSCI 352
   Assignment 2
   7/24/2015

   reads transactions.txt and edits a b-tree

 */

#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024
#define DEBUG 1


int getTransaction(char *line, char *action, char *code);
int exeAction(char *action, char *code, char *line, int linepos);

int itemChange(char *action, char *code, char *line, int linepos);
int updateHistory(struct Data *item, int sale);
int changePrice(char *line, struct Node *node, int datapos, int linepos);
int addItem(char *line);


int main (int argc, char** argv) {


  if (argc < 2){
    printf("usage: ./editbtree [TRANSACTIONS FILE]\n");
    return 0;
  }

  char *filename = argv[1];
  char line[BUF_SIZE];

  char action[14];
  char code[9];
  int i;

  FILE *fd;

  fd = fopen(filename, "r");

  if (fd == NULL){
    printf("File %s not found\n", filename);
  }else{
    while(fgets(line, BUF_SIZE, fd)){

      //get transaction
      i = getTransaction(line, action, code);

      //perform transaction
      exeAction(action, code, line, i);


    }
  }

  fclose(fd);  
  return 0;
}

/*
  function: getTransaction
  Input: 1. line: The transaction line.
         2. action: A null char* to be filled with action in lin.
         3. code: A null char* to be filled with item code in line.
  Return: The index in line after item code. Used in other functions
          for quick access.

  Reads through line character by 
  character filling other inputs with data. 
 */
int getTransaction(char *line, char *action, char *code){

  int i = 0;
  
  while (line[i] != ' '){
    action[i] = line[i];
    i++;
  }
  
  action[i] = '\0';
  i++;

  int j = 0;

  while (j < 8){
    code[j] = line[i];
    j++;
    i++;
  }

  code[j] = '\0';
  return i;

}

int exeAction(char *action, char *code, char *line, int linepos){

    int j;
    int act = -1;

    char* acts[5];
    acts[0] = "SALE";
    acts[1] = "DELIVERY";
    acts[2] = "NEWPRODUCT";
    acts[3] = "DELETE";
    acts[4] = "PRICE";

    //locate index of user command in opts array for switch
    for (j = 0; j < 5; j++){
        if (strcmp(action, acts[j]) == 0){
            act = j;
        }
    }

    switch(act){
        case 0:
            itemChange(action, code, line, linepos);
            break;
        case 1:
            itemChange(action, code, line, linepos);
            break;
        case 2: 
#if DEBUG
            printf("adding item..\n");
#endif     
            addItem(line);
            break;
        case 3:      
#if DEBUG
            printf("deleting item..\n");
#endif

            break;
        case 4:      
            itemChange(action, code, line, linepos);

            break;
    }
  
  return 0;

}

int itemChange(char *action, char *code, char *line, int linepos){

  int j = 0;
  int newStock;
  int datapos;
  char amount[4];
  struct Node *root = getNode(0);
  struct Node *node = malloc(sizeof(struct Node));

  //Part (a) search for item in question
  if ((datapos = search(root, code, node)) == -1){
    printf("No item with code %s.\n", code);
    return 0;
  }else{


    if (strcmp(action, "PRICE") != 0){
      linepos++;

      while(line[linepos] != '\0'){
        amount[j] = line[linepos];
        j++;
        linepos++;
      }
      line[linepos] = '\0';
    }


    //Part (b) perform transaction
    if (strcmp(action, "SALE") == 0){
      newStock = (node->data[datapos].stock) - atoi(amount); 
      if (newStock < 0){
        printf("Quantity Sold of item %s is greater than current stock.\n", code);
      }else{
        node->data[datapos].stock = newStock;
        updateHistory(&(node->data[datapos]), atoi(amount));
      }
    }else if (strcmp(action, "DELIVERY") == 0){
      newStock = (node->data[datapos].stock) + atoi(amount); 
      node->data[datapos].stock = newStock;
    }else{
      changePrice(line, node, datapos, linepos);
    }

    saveNode(node);

  }

  free(root);
  free(node);
  return 0;
}

int changePrice(char *line, struct Node *node, int datapos, int linepos){

  int i = 0;
  char dollars[5];
  char cents[3];

  linepos++;

  while(line[linepos] != '.'){
    dollars[i] = line[linepos];
    linepos++;
    i++;
  }

  i = 0;
  linepos++;

  while((line[linepos] != '\n') && (line[linepos] != EOF)){
    cents[i] = line[linepos];
    linepos++;
    i++;
  }

  node->data[datapos].dollar = atoi(dollars);
  node->data[datapos].cent = atoi(cents);

  saveNode(node);

  return 0;

}



int updateHistory(struct Data *item, int sale){ 
  
  int i;
  for (i = 10; i >= 0; i--){
    item->history[i+1] = item->history[i];
  }
  item->history[0] = sale;
  return 0;
}


int addItem(char *line){

 int i;
 struct Data item;
 buildData(&item, &line[11]);
 for(i = 0; i < 11; i++){
   if ((item.cate[i] >= 97) && (item.cate[i] <= 122)){
     item.cate[i] = item.cate[i] - 32;
   }
 }


 btree = getNode(0);
 insert(&item);
 free(btree);

 return 0;


}

