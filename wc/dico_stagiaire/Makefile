CC     = gcc
CFLAGS = -Wall -Wextra -Werror -g

# --------------------------------------------------------------------------
# Cibles
# --------------------------------------------------------------------------

all: benchmark

# L'exécutable benchmark dépend des deux fichiers objets
benchmark: benchmark.o dico.o
	$(CC) $^ -o $@

# Règle générique : compile chaque .c en .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dépendances explicites des headers
# Si dico.h change, tout ce qui l'inclut est recompilé
dico.o:      dico.c dico.h
benchmark.o: benchmark.c dico.h

# --------------------------------------------------------------------------
clean:
	rm -f *.o benchmark

.PHONY: all clean
