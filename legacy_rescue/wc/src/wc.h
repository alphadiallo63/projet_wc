#ifndef WC_H
#define WC_H

#include <stdio.h>
#include "../../libdico/src/dico.h"

typedef struct {
    long lcount;
    long wcount;
    long ccount;
    long ltotal;
    long wtotal;
    long ctotal;
} wc_counts_t;

void count(FILE *f, wc_counts_t *c, dict_t *dict);
void usage(void);

#endif /* WC_H */