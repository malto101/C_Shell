CC = gcc
CFLAGS = -Wall -g
TARGET = build/shell
VPATH = builtin_cmds
SRCS = $(wildcard *.c) $(wildcard builtin_cmds/*.c)
OBJS = $(addprefix build/, $(notdir $(SRCS:.c=.o)))

.PHONY: all clean

all: | build $(TARGET)

build:
	mkdir -p build

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)