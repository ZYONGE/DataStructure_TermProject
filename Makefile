CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wno-unused-function -Iinclude -Isrc

SRCS    = src/main.c                    \
          src/family/family_tree.c      \
          src/algorithm/dfs.c           \
          src/algorithm/relation.c      \
          src/data/keywords.c           \
          src/ui/ui.c                   \
          src/ui/deque_view.c           \
          src/util/util.c

ifeq ($(OS),Windows_NT)
    TARGET  = build/family_tree.exe
    LDFLAGS =
else
    TARGET  = build/family_tree
    LDFLAGS =
endif

all: $(TARGET)

$(TARGET): $(SRCS) | build
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

build:
	mkdir -p build

clean:
ifeq ($(OS),Windows_NT)
	del /Q build\*
else
	rm -rf build/
endif

.PHONY: all clean
