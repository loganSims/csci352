CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS1 = btree.o initbtree.o 
EXES = initbtree

all: $(EXES)

initbtree:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
