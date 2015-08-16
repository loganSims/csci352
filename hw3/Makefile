CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -g
OBJECTS1 = browser.o
EXES = browser
PKG = `pkg-config --cflags --libs gtk+-3.0`
all: $(EXES)

browser:	$(OBJECTS1)
	$(CC) -o $@ $(LDFLAGS) $(OBJECTS1) $(PKG)

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@ $(PKG)

clean:
	rm -f *.o $(EXES)
