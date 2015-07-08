/* tokenizer.c

     Logan Sims
     7/6/2015
     warmup2

     A function that generates a NULL-terminated array of
     subtstrings from the input line. Substrings are separeated by
     spaces. Double quotes are used to include spaces withing the
     substrings.
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DEBUG 1

//returns number of tokens in line
int collecttokens(char *line, char **output);

char *gettoken(char *start, char *end);

/*
  first call to get collecttokens counts the 
  number of tokens so that memery can be allocated. 
*/
char **gettokens (char *line){

  char **output = (char **)malloc(sizeof(char*));
  int tokens;

  tokens = collecttokens(line, output); 

  output = (char **)malloc((tokens * sizeof(char*)) + 1);

  collecttokens(line, output); 

  output[tokens] = '\0';
  
  return output;
}

/*
  reads through line collecting all tokens. 
  returns the number of tokens in line
*/

int collecttokens(char *line, char **output){

  int q = 0; //flag for quote check
  int readingToken = 0; //flag to check if reading token
  
  char *pos;
  char *start;

  int tokens = 0;

  pos = line; //used to read through line
 
  while (*pos != '\0'){ 
    if((*pos == ' ') && (!q)){ 
      if (readingToken == 1){ 
        output[tokens] = gettoken(start, pos);
        tokens++;
        readingToken = 0;
      }
      pos++; 
    }else if (*pos == '"'){ //enter "quote mode"
      if (q) {
        q = 0;
        output[tokens] = gettoken(start, pos);
        tokens++;
        readingToken = 0;
      }else{
        q = 1;
      }
      pos++;
    }else{ //characters
      if (!readingToken){
        readingToken = 1;
        start = pos;
      }
      pos++;
    }
  }
  
  //pick up last token if exists
  if(readingToken){
    output[tokens] = gettoken(start, pos);
    tokens++;
  }

  return tokens;
}

/*
  Given starting and ending pointers into a string
  returns a substring to the string.
  first reads through substring to check for quotes,
  no space will be allocated for them as they are not
  counted as a character in a token.
*/
char *gettoken(char *start, char *end){

  char *dstart = start;
  char *dend = end;

  int size = (end - start);

  while (dstart < dend){ 
    if(*dstart == '"'){
       size--;
    }    
    dstart++;
  }


  char *token = (char *)malloc((size * sizeof(char)) + 1);
  char *pos = token;

  while (start < end){ 
    if(*start != '"'){
       memcpy(pos, start, sizeof(char));
       pos++;
    }    
    start++;
  }

  *pos = '\0';

  return token;
}
