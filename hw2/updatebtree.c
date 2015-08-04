/*

   updatebtree.c

   Logan Sims
   CSCI 352
   Assignment 2
   08/05/2015

   Reads transactions.txt and edits b-tree file.

 */

#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024


// Get transaction and execute transaction functions.
int getTransaction(char *line, char *action, char *code);
int exeAction(char *action, char *code, char *line, int linepos);

// Process transaction functions.
int itemChange(char *action, char *code, char *line, int linepos);
int updateHistory(struct Data *item, int sale);
int updateSales(struct Data *item, int sold);
int changePrice(char *line, struct Node *node, int datapos, int linepos);
int addItem(char *line);
int deleteItem(char *code);

// Report functions.
int monthReport(struct Node *node);
int printItem(struct Node *node, int i);

/*
  function: main

  Reads from transaction file line by line, processing each
  transaction. 

  At the end calls monthReport to display report.
  Uses getNode() from btree.c
 */

int main (int argc, char** argv) {

  if (argc < 2){
    printf("usage: ./updatebtree [TRANSACTIONS FILE]\n");
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


/*
  function: exeAction
  Input: 1. action: Name of the transaction being performed.
         2. code: Code of item involed in transaction
         3. line: Original line form transaction file
         4. linepos: The position in line of the data for transaction.
                     (The point in the line after transaction name and item code)

  Return: 0 on complete (value not used)

  Based on action uses switch statement to call correct function
  to perform action. 
 */
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

/*
  function: itemChange
  Input: 1. action: action to be perfomed.
         2. code: item changed by action. 
         3. line: original line from transaction file.
         4. linepos: position in line after transaction type and item code.

  Return: 0 on complete. (value not used)

  Handles sales, delivery, and price changes for items.
  Part (a): Search for the item in question.
  Part (b): If the action is not a price change pull out amount of
            sale or delivery for use.
  Part (c): Perform correct transaction based on action input.
            - Sale: Item's stock is updated and the profit field
                    of the item is calculated based on current price.
            - Delivery: Item's stock is updated.
            - Price change: function changePrice is called to handle change.

  Uses saveNode(), and getNode() from btree.c

 */
int itemChange(char *action, char *code, char *line, int linepos){

  int j = 0;
  int newStock;
  int datapos;
  char amount[4];
  struct Node root;
  struct Node node;

  getNode(0, &root);

  //Part (a)
  if ((datapos = search(&root, code, &node)) == -1){
    printf("ERROR: No item with code %s. Cannot complete %s.\n", code, action);
    return 0;
  }

  // Part (b)
  if (strcmp(action, "PRICE") != 0){
    linepos++;

    while(line[linepos] != '\0'){
      amount[j] = line[linepos];
      j++;
      linepos++;
    }
    line[linepos] = '\0';
  }

  //Part (c)
  if (strcmp(action, "SALE") == 0){
    newStock = (node.data[datapos].stock) - atoi(amount); 
    if (newStock < 0){
      printf("ERROR: Quantity Sold of item %s is greater than current stock.\n", code);
    }else{
      node.data[datapos].stock = newStock;
      updateHistory(&(node.data[datapos]), atoi(amount));
      updateSales(&(node.data[datapos]), atoi(amount));
    }

  }else if (strcmp(action, "DELIVERY") == 0){
    newStock = (node.data[datapos].stock) + atoi(amount); 
    node.data[datapos].stock = newStock;
  }else{
    changePrice(line, &node, datapos, linepos);
  }

  saveNode(&node);

  return 0;
}

/* 
  function changePrice
  input: 1. line: original line from transaction file.
         2. node: node in btree that holds item.
         3. datapos: position of the item being changed in node
         4. linepos: current position in line. 
                     (will be after action name and item code)
  Return: 0 on compelete. (value not used)

  Builds gets new price and updates the information in the Data struct.
  first while loop gathers dollar amount, second gets cent amount.
  Saves node once done.

  Uses saveNode() from btree.c

 */
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
  // skip passed '.'
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


/*
  function: updateSales
  Input: 1. item: item being updated.
         2. quanitiy of sale.

  Return: 0 on complete. (value not used)

  Updates the profit field of a item after sale.
  Performs string to double conversion.

 */
int updateSales(struct Data *item, int sold){

  char price[12];  

  sprintf(price, "%d.%d", item->dollar, item->cent);
 
  float saleAmount = sold * strtof(price, NULL);

  item->profit += saleAmount;

  return 0;

}

/*
  function: updateHistory
  Input: 1. item: item to be updated.
         2. sale: amount sold.
  Return: 0 on complete. (value not used)

  Updates the item histroy after sale.
  If item profit is 0 then assumes new months worth
  of data and makes new month entry. Once profits are
  being made assumes same month and updates that months
  history.

 */

int updateHistory(struct Data *item, int sale){ 
  
  int i;

  if(item->profit == 0){
    for (i = 10; i >= 0; i--){
      item->history[i+1] = item->history[i];
    }
    item->history[0] = sale;
  }else{
    item->history[0] += sale;
  }
  return 0;
}


/*
  function: addItem
  Input: line: line from transaction file.
  Return: 0 on complete. (value not used)
 
  Adds an item to the btree if there is not 
  already an item with same code in tree.
  
  Uses buildData() from btree.c to make item, 
  line[11] is used to skip past transation name 
  in line and get to the new item data.

  Also Uses search(), insertSearch(), and insert() from btree.c

 */
int addItem(char *line){

 int i; 
 struct Node root;
 struct Node found;
 struct Data item;

 getNode(0, &root);

 buildData(&item, &line[11]);
 
 i = search(&root, item.code, &found);

 if(i != -1){
   printf("ERROR: Cannot add item, ");
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

/*
  function: deleteItem
  Input: code: the item code to be deleted. 
  Return: 0 on complete (value not used)

  Deletes item from btree. First checks if 
  item is in tree.

  Uses deleteKey from btree.c

 */

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

/*
  function: monthReport
  Input: node: Starts as root for tree, node of current focus in
               recursive calls.
  Return: 0 on complete (value not used)
 
  Performs in order traversal to print out sales
  report for every item. calls printItem to print
  data in each item

  Uses getNode() and saveNode() from btree.c

 */

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


/*
  function: printItem
  Input: 1. node: the node the data item is in.
         2. i: the index in node that data item is in.
  Returns: 0 on complete. (value not used)

  Prints item code, desc, and profit fields for report.
  Updates history of items that did not sell in 
  current month. Sets profit back to 0 so data
  is ready for next months transactions.

 */
int printItem(struct Node *node, int i){

  //item had no sale for the month
  if(node->data[i].profit == 0){
    updateHistory(&(node->data[i]), 0);
  }

  printf("%s:%s Sales: $%.2f\n", node->data[i].code, node->data[i].desc, node->data[i].profit);

  node->data[i].profit = 0;

  return 0;
}
