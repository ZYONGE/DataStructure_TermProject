CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wno-unused-function -Iinclude -Isrc

SRCS    = src/main.c                    \
          src/option.c                  \
          src/family/family_tree.c      \
          src/algorithm/dfs.c           \
          src/algorithm/relation.c      \
          src/data/keywords.c           \
          src/ui/ui.c                   \
          src/ui/deque_view.c           \
          src/util/util.c

ifeq ($(OS),Windows_NT)
    TARGET  = family_tree.exe
    LDFLAGS = -static
else
    TARGET  = family_tree
    LDFLAGS =
endif

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -f family_tree.exe family_tree

.PHONY: all clean
