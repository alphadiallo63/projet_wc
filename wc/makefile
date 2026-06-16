CC = gcc
CFLAGS = -Wall -Wextra -Werror -g

SRC = main.c wc_core.c
OBJ = $(SRC:.c=.o)
EXEC = wc

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean
