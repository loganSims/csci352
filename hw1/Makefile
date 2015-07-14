CC = gcc
CFLAGS = -Wall -g -O3
LDFLAGS = -g
OBJECTS1 = tokenizer.o cli.o 
OBJECTS2 = regexp.o fref.o
OBJECTS3 = regexp.o showenv.o
EXES = cli fref showenv

all: $(EXES)

cli:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1)

fref:	$(OBJECTS2)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS2)

showenv:$(OBJECTS3)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS3)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(EXES)
