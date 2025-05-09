# Makefile for textreader package

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lncurses
TARGET = textreader
SRC = textreader.c

BIN_SCRIPTS = ebook epub2ascii html2text
INSTALL_DIR = /usr/bin

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

install: all
	@echo "Installing to $(INSTALL_DIR)..."
	install -m 0755 -o root -g root $(TARGET) $(INSTALL_DIR)/
	for f in $(BIN_SCRIPTS); do \
		install -m 0755 -o root -g root $$f $(INSTALL_DIR)/; \
	done
	@echo "Installation complete."

clean:
	rm -f $(TARGET)

