CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS1 = btree.o initbtree.o 
OBJECTS2 = btree.o readtree.o 
OBJECTS3 = btree.o editbtree.o 
EXES = initbtree readtree editbtree

all: $(EXES)

initbtree:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1)

readtree:	$(OBJECTS2)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS2)

editbtree:	$(OBJECTS3)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS3)

%.o:	%.c %.h
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
