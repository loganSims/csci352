/* regexp.c
    
    a simple regular expression check 

    matches:    c   literal match with single character c
                .   match any single character
                ^   match start of line
                $   match end of line
                *   match zero or more occurrences of previous character

    extracted from "Beautiful Code", Oram & Wilson, Chapter 1
*/

#include <stdio.h>

#define DEBUG 0

int matchhere (char *regexp, char *text);
int matchstar (char c, char *regexp, char *text);

// match: search for regexp anywhere in text
int match (char *regexp, char *text) {
    
#if DEBUG
    printf ("match %s %s$\n", regexp, text);
#endif

    // match start of line
    if (regexp[0] == '^')
        return matchhere (regexp+1, text);

    do {    // check even if string is empty
        if (matchhere (regexp, text))
            return 1;
    } while (*text++ != '\0');
    return 0;
}

// matchhere: search for regexp at the beginning of text
int matchhere (char *regexp, char *text) {

#if DEBUG
    printf ("matchhere %s %s$\n", regexp, text);
#endif
    
    // end of regexp
    if (regexp[0] == '\0') return 1;
    
    // repeated character
    if (regexp[1] == '*')
        return matchstar(regexp[0], regexp+2, text);
    
    // match end of line
    if (regexp[0] == '$' && regexp[1] == '\0')
        return *text == '\0';
    
    // match characters
    if (*text != '\0' &&
        (regexp[0] == '.' || regexp[0] == *text))
        return matchhere (regexp+1, text+1);
    
    return 0;
}

// matchstar: leftmost longest search for c*regexp
int matchstar (char c, char *regexp, char *text) {

#if DEBUG
    printf ("matchstar %c %s %s$\n", c, regexp, text);
#endif
    
    char *t;
    
    // match repeated character
    for (t = text; *t != '\0' && (*t == c || c == '.'); t++);
    
    // resume matching regexp after repeated character
    do {
        if (matchhere (regexp, t))
            return 1;
    } while (t-- > text);
    
    return 0;
}
        
    
