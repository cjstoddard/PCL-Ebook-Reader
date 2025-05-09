# Makefile for textreader (non-wide version)

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lncurses
TARGET = textreader
SRC = textreader.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

