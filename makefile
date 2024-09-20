TARGET = nxensite
CFLAGS = -O2 -g0 -fno-record-gcc-switches -fno-ident
CC = gcc

export SOURCE_DATE_EPOCH := 1726544855358 
all: $(TARGET)

$(TARGET): nxensite.o
	$(CC) $(CFLAGS) -o $(TARGET) nxensite.o
	strip --strip-all $(TARGET)
	rm nxensite.o
	sha256sum nxensite > nxensite.sha256

nxensite.o: nxensite.c
	$(CC) $(CFLAGS) -c nxensite.c

clean:
	rm -f *.o $(TARGET)
	
