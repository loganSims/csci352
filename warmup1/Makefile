CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -g
OBJECTS = regexp.o fref.o
EXES = fref

fref:	$(OBJECTS)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS)
	

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
