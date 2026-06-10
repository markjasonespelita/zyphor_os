CC = gcc
CFLAGS = -Wall -Wextra -O2
GTKFLAGS = $(shell pkg-config --cflags gtk4)
GTKLIBS = $(shell pkg-config --libs gtk4)

TARGET_DIR = zyphor-whats-new/usr/bin
TARGET = $(TARGET_DIR)/zyphor-whats-new

all:
	mkdir -p $(TARGET_DIR)
	$(CC) main.c -o $(TARGET) $(CFLAGS) $(GTKFLAGS) $(GTKLIBS)

clean:
	rm -rf $(TARGET)