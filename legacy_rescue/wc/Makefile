CC       = gcc
CFLAGS   = -Wall -Wextra -Werror -g -I./src -I../libdico/src
LDFLAGS  = -L../libdico/lib -ldico -Wl,-rpath,$(abspath ../libdico/lib)

OBJ      = src/main.o src/wc_core.o
EXEC     = bin/wc

all: $(EXEC)

$(EXEC): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

src/main.o: src/main.c src/wc.h
	$(CC) $(CFLAGS) -c src/main.c -o src/main.o

src/wc_core.o: src/wc_core.c src/wc.h
	$(CC) $(CFLAGS) -c src/wc_core.c -o src/wc_core.o

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean