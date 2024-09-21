CC = gcc
CFLAGS = -Wall -Wextra -g -Wno-unused-parameter -Wno-unused-result
SRCS = main.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(OBJS) $(TARGET)
