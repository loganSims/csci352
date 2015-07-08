/* regexp.h
    
    a simple regular expression check 

    matches:    c   literal match with single character
                .   match any single character
                ^   match start of line
                $   match end of line
                *   match zero or more occurrences of previous character

    extracted from "Beautiful Code", Oram & Wilson, Chapter 1
*/

// match: search for regexp anywhere in text
int match (char *regexp, char *text);
