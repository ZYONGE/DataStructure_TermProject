CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Iinclude
TARGET  = bin/main
SRCS    = src/main.c

all: $(TARGET)

$(TARGET): $(SRCS) | bin
	$(CC) $(CFLAGS) -o $@ $(SRCS)

bin:
	mkdir -p bin

clean:
	rm -rf bin/

.PHONY: all clean
