CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS1 = access.o explorer.o
EXES = explorer
PKG = `pkg-config --cflags --libs gtk+-3.0`
all: $(EXES)

explorer:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1) $(PKG)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(PKG)

clean:
	rm -f *.o $(EXES)
