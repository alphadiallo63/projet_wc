CC      = gcc
CFLAGS  = -Wall -Wextra -Werror -g
# -fPIC : Position Independent Code, obligatoire pour une .so
CFLAGS_PIC = $(CFLAGS) -fPIC

LIB     = libdico.so
BENCH   = benchmark

# --------------------------------------------------------------------------
all: $(LIB) $(BENCH)

# -- Bibliothèque dynamique ------------------------------------------------
# 1. Compile dico.c en objet relogeable (-fPIC)
dico.o: dico.c dico.h
	$(CC) $(CFLAGS_PIC) -c $< -o $@

# 2. Assemble le .so (-shared)
$(LIB): dico.o
	$(CC) -shared -o $@ $^

# -- Exécutable benchmark --------------------------------------------------
benchmark.o: benchmark.c dico.h
	$(CC) $(CFLAGS) -c $< -o $@

# -L. : cherche les .so dans le répertoire courant
# -ldico : lie avec libdico.so
# -Wl,-rpath,. : indique au programme où chercher libdico.so à l'exécution
$(BENCH): benchmark.o $(LIB)
	$(CC) $< -L. -ldico -Wl,-rpath,. -o $@

# --------------------------------------------------------------------------
clean:
	rm -f *.o $(LIB) $(BENCH)

.PHONY: all clean
