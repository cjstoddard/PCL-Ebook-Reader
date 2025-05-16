# Makefile for ebook + rssreader package

CC = gcc
CFLAGS = -Wall -O2
LDFLAGS_TEXTREADER = -lncurses
LDFLAGS_RSSREADER = -lncurses -lcurl

BIN_DIR = /usr/local/bin

TEXTREADER_SRC = textreader.c
TEXTREADER_BIN = textreader

RSSREADER_SRC = rssreader.c
RSSREADER_BIN = rssreader

SCRIPTS = epub2ascii ebook html2text

.PHONY: all clean install

all: $(TEXTREADER_BIN) $(RSSREADER_BIN)

$(TEXTREADER_BIN): $(TEXTREADER_SRC)
	$(CC) $(CFLAGS) -o $(TEXTREADER_BIN) $(TEXTREADER_SRC) $(LDFLAGS_TEXTREADER)

$(RSSREADER_BIN): $(RSSREADER_SRC)
	$(CC) $(CFLAGS) -o $(RSSREADER_BIN) $(RSSREADER_SRC) $(LDFLAGS_RSSREADER)

install: all
	@echo "Installing binaries to $(BIN_DIR)..."
	install -m 0755 -o root -g root $(TEXTREADER_BIN) $(BIN_DIR)/
	install -m 0755 -o root -g root $(RSSREADER_BIN) $(BIN_DIR)/
	for script in $(SCRIPTS); do \
		if [ -f "$$script" ]; then \
			install -m 0755 -o root -g root $$script $(BIN_DIR)/; \
		else \
			echo "Warning: $$script not found, skipping."; \
		fi \
	done
	@echo "Installation complete."

clean:
	rm -f $(TEXTREADER_BIN) $(RSSREADER_BIN)

