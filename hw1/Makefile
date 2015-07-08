CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -g
OBJECTS = tokenizer.o cli.o
EXES = cli

cli:	$(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
