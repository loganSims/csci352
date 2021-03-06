#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tokenizer.h"

#define MAX_LINE    1024



int main (int argc, char** argv) {
    
    char line[MAX_LINE];
    char **tokens;
    int i;

    // get a line from stdin and get the tokens
    printf("> ");
    while (fgets(line, MAX_LINE, stdin)) {
        
        // remove the '\n' from the end of the line
        line[strlen(line)-1] = '\0';
        
        // get and display the tokens in line
        tokens = gettokens(line);
        for (i=0; tokens[i]; i++)
            printf("token %d is '%s'\n", i, tokens[i]);
        printf("> ");
        free(tokens);
    }
       
    return 0;

}
