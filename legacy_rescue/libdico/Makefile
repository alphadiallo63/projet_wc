CC         = gcc
CFLAGS     = -Wall -Wextra -Werror -g -fPIC -I./src
//LDFLAGS    = -shared -install_name @rpath/libdico.so
LDFLAGS    = -shared -install_name @rpath/libdico.so

SRC        = src/dico.c
OBJ        = $(SRC:.c=.o)
LIB        = lib/libdico.so

all: $(LIB)

$(LIB): $(OBJ)
	@mkdir -p lib
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(LIB)

.PHONY: all clean