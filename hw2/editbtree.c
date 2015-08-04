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

struct PriceChange
{
  char code[9];
  int sold;
  int dollar;
  int cent;
  int sales;
};

struct PriceChange *pricechanges;

int monthReport(struct Node *node);

int getTransaction(char *line, char *action, char *code);
int exeAction(char *action, char *code, char *line, int linepos);

int itemChange(char *action, char *code, char *line, int linepos);
int updateHistory(struct Data *item, int sale);
int changePrice(char *line, struct Node *node, int datapos, int linepos);
int addItem(char *line);
int deleteItem(char *code);

int main (int argc, char** argv) {

  if (argc < 2){
    printf("usage: ./editbtree [TRANSACTIONS FILE]\n");
    return 0;
  }

  struct Node root;

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

  getNode(0, &root);

  printf("----------------MONTH SALES REPORT----------------\n");
  monthReport(&root);  
  printf("--------------------------------------------------\n");
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
            addItem(line);
            break;
        case 3:      
            deleteItem(code);
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
  struct Node root;
  struct Node node;

  getNode(0, &root);

  //Part (a) search for item in question
  if ((datapos = search(&root, code, &node)) == -1){
    printf("ERROR: No item with code %s. Cannot complete %s.\n", code, action);
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
      newStock = (node.data[datapos].stock) - atoi(amount); 
      if (newStock < 0){
        printf("ERROR: Quantity Sold of item %s is greater than current stock.\n", code);
      }else{
        node.data[datapos].stock = newStock;
        updateHistory(&(node.data[datapos]), atoi(amount));
        node.data[datapos].sold = 1;


      }
    }else if (strcmp(action, "DELIVERY") == 0){
      newStock = (node.data[datapos].stock) + atoi(amount); 
      node.data[datapos].stock = newStock;
    }else{
      changePrice(line, &node, datapos, linepos);
    }

    saveNode(&node);

  }

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
 struct Node root;
 struct Node found;
 struct Data item;

 getNode(0, &root);

 buildData(&item, &line[11]);
 
 i = search(&root, item.code, &found);

 if(i != -1){
   printf("ERROR: Cannot Add item, ");
   printf("there is already an item with code: %s.\n", item.code);
   return 0;
 }

 for(i = 0; i < 11; i++){
   if ((item.cate[i] >= 97) && (item.cate[i] <= 122)){
     item.cate[i] = item.cate[i] - 32;
   }
 }

 insertSearch(&root, &item, &found);

 insert(&found, &item);

 return 0;
}

int deleteItem(char *code){

  struct Node found;
  struct Node root;
  int index;

  getNode(0, &root);

  index = search(&root, code, &found);

  if(index == -1){
    printf("ERROR: Item with code %s is not in database.\n", code);
    return 0;
  }

  deleteKey(&found, code);

  return 0;
}


int printItem(struct Node *node, int i){

  char price[10];

  //item had no sale for the month
  if(node->data[i].sold != 1){
    updateHistory(&(node->data[i]), 0);
  }

  //check from price changes


  //calculate sales profit

  sprintf(price, "%d.%d", node->data[i].dollar, node->data[i].cent);
 
  float saleAmount = node->data[i].history[0] * strtof(price, NULL);

  printf("%s:%s Sales: $%.2f\n", node->data[i].code, node->data[i].desc, saleAmount);

  node->data[i].sold = 0;

  return 0;
}

int monthReport(struct Node *node){
  
  int i;
  struct Node nextNode;


  for (i = 0; i < node->count; i++){

    if (node->offsets[0] != -1){
      if(node->offsets[i] != -1){
        getNode(node->offsets[i], &nextNode);
        monthReport(&nextNode);
      }
    }

    printItem(node, i);

  }

  if (node->offsets[0] != -1){
    if(node->offsets[i] != -1){
      getNode(node->offsets[i], &nextNode);
      monthReport(&nextNode);
    }
  }

  saveNode(node);

  return 0;
}


