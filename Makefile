CC = gcc
CFLAGS = -Wall -Wextra -g
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
