CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -g
OBJECTS1 = tokenizer.o cli.o 
OBJECTS2 = regexp.o fref.o
EXES = cli fref

all: $(EXES)

cli:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1)

fref:	$(OBJECTS2)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS2)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
