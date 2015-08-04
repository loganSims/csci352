CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS1 = btree.o loadbtree.o 
OBJECTS2 = btree.o readtree.o 
OBJECTS3 = btree.o updatebtree.o 

EXES = loadbtree readtree updatebtree

all: $(EXES)

loadbtree:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1)

readtree:	$(OBJECTS2)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS2)

updatebtree:	$(OBJECTS3)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS3)

%.o:	%.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
