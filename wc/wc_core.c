#include <stdio.h>
#include <stdlib.h>
#include "wc.h"
#include <ctype.h>



void count(FILE *f, wc_counts_t *c)
{
     int ch;
     int word = 0;
 
    c->lcount = 0;
    c->wcount = 0;
    c->ccount = 0;
 
    while ((ch = getc(f)) != EOF) {
        c->ccount++;
 
        if (isspace(ch)) {
            if (word) c->wcount++;
            word = 0;
        } else {
            word = 1;
        }
 
        if (ch == '\n' || ch == '\f') c->lcount++;
    }
 
    /* Cumulation des totaux */
    c->ltotal += c->lcount;
    c->wtotal += c->wcount;
    c->ctotal += c->ccount;
}

void usage(void)
{
  fprintf(stderr, "Usage: wc [-lwc] [name ...]\n");
  exit(1);
}
