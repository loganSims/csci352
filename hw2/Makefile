CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS1 = btree.o initbtree.o 
OBJECTS2 = btree.o readtree.o 
EXES = initbtree readtree

all: $(EXES)

initbtree:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1)

readtree:	$(OBJECTS2)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS2)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
