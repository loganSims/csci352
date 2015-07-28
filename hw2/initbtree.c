/*
   Logan Sims
   CSCI 352
   Assignment 2
   7/24/2015

   reads inventory.txt and init b-tree

 */

#include "btree.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 1024
#define DEBUG 0

int setNumber(struct Data *item, char *line, int *pos);
int setDesc(struct Data *item, char *line, int *pos);
int setPrice(struct Data *item, char *line, int *pos);
int setCategory(struct Data *item, char *line, int *pos);
int setStock(struct Data *item, char *line, int *pos);
int setHist(struct Data *item, char *line, int *pos);

int saveItem(struct Data *item);

int main (int argc, char** argv) {

  char *filename = "inventory.txt";
  char line[BUF_SIZE];

  FILE *fd;

  int p = 0;
  int *pos = &p;

  fd = fopen(filename, "r");

  if (fd == NULL){
    printf("File %s not found\n", filename);
  }else{

    struct Data *item = malloc(sizeof(struct Data));

    while(fgets(line, BUF_SIZE, fd)){

      setNumber(item, line, pos);
      setDesc(item, line, pos);
      setPrice(item, line, pos);      
      setCategory(item, line, pos);      
      setStock(item, line, pos);    
      setHist(item, line, pos);

      //add item to b-tree
      insert(item);


      free(item);
      item = malloc(sizeof(struct Data));

    }

  fclose(fd);
  }


  return 0;
}

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
