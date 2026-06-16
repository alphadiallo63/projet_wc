#ifndef WC_H
#define WC_H
#include <stdio.h>

extern int lflag;			/* Count lines */
extern int wflag;			/* Count words */
extern int cflag;	        /* Count characters */


// Definition d'une structure pour stocker les compteurs de lignes, mots et caractères, ainsi que les totaux pour plusieurs fichiers.

typedef struct {
        long lcount;    /* Count of lines */
        long wcount;    /* Count of words */
        long ccount;    /* Count of characters */
        
        long ltotal;    /* Total count of lines */
        long wtotal;    /* Total count of words */
        long ctotal;    /* Total count of characters */
} wc_counts_t;

void count(FILE *f, wc_counts_t *c);
void usage(void);

#endif /* WC_H */