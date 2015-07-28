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
int exeAction(char *action, char *code, char *line, int i);

int sale(char *action, char *code, char *line, int i);
int updateHistory(struct Data *item, int sale);

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
#if DEBUG
      printf("transaction: %s  ", action);
      printf("code: %s\n", code);
      printf("next character at: %d\n", i);
#endif

      //perform transaction
      exeAction(action, code, line, i);


    }
  }

  fclose(fd);  
  return 0;
}

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

int exeAction(char *action, char *code, char *line, int i){

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
#if DEBUG
            printf("sale\n");
#endif
            sale(action, code, line, i);
            break;
        case 1:
#if DEBUG
            printf("delivery\n");
#endif

            break;
        case 2:      
#if DEBUG
            printf("new product\n");
#endif

            break;
        case 3:      
#if DEBUG
            printf("delete\n");
#endif

            break;
        case 4:      
#if DEBUG
            printf("price\n");
#endif

            break;
    }
  
  return 0;

}

int sale(char *action, char *code, char *line, int i){

  int j = 0;
  int newStock;
  int dataIndex;
  struct Node *root = getNode(0);
  struct Node *node = malloc(sizeof(struct Node));

  if ((dataIndex = search(root, code, node)) == -1){
    printf("Can't find item code: %s\n", code);
    return 0;
  }else{
    char amount[4];
    i++;
    while(line[i] != '\0'){
      amount[j] = line[i];
      j++;
      i++;
    }

    line[i] = '\0';
    newStock = (node->data[dataIndex].stock) - atoi(amount); 
 
#if DEBUG
    printf("old stock: %d\n", node->data[dataIndex].stock);
    printf("sale amount: %d\n", atoi(amount));
    printf("newStock: %d\n", newStock);
#endif

    node->data[dataIndex].stock = newStock;
    updateHistory(&(node->data[dataIndex]), atoi(amount));

    saveNode(node);

  }

  free(root);
  free(node);
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


