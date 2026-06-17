#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wc.h"

#define WORD_BUF_SIZE 1024

static int is_separator(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r'
            || c == ',' || c == '.' || c == ';' || c == ':' || c == '!'
            || c == '?' || c == '"' || c == '\'' || c == '(' || c == ')'
            || c == '-' || c == '_');
}

void count(FILE *f, wc_counts_t *c, dict_t *dict)
{
    int    ch;
    int    in_word = 0;
    char   word[WORD_BUF_SIZE];
    size_t wlen    = 0;

    c->lcount = 0;
    c->wcount = 0;
    c->ccount = 0;

    while ((ch = getc(f)) != EOF) {
        c->ccount++;

        if (ch == '\n' || ch == '\f')
            c->lcount++;

        if (is_separator(ch)) {
            if (in_word) {
                c->wcount++;

                if (dict && wlen > 0) {
                    word[wlen] = '\0';
                    const void *val_ptr;
                    size_t      val_len;
                    int         occ;

                    if (dict_get_value(dict, word, wlen + 1,
                                       &val_ptr, &val_len) == DICT_OK)
                        occ = *(const int *)val_ptr + 1;
                    else
                        occ = 1;

                    dict_add(dict, word, wlen + 1, &occ, sizeof(int));
                }

                in_word = 0;
                wlen    = 0;
            }
        } else {
            in_word = 1;
            if (wlen < WORD_BUF_SIZE - 1)
                word[wlen++] = (char)ch;
        }
    }

    if (in_word) {
        c->wcount++;
        if (dict && wlen > 0) {
            word[wlen] = '\0';
            const void *val_ptr;
            size_t      val_len;
            int         occ;

            if (dict_get_value(dict, word, wlen + 1,
                               &val_ptr, &val_len) == DICT_OK)
                occ = *(const int *)val_ptr + 1;
            else
                occ = 1;

            dict_add(dict, word, wlen + 1, &occ, sizeof(int));
        }
    }

    c->ltotal += c->lcount;
    c->wtotal += c->wcount;
    c->ctotal += c->ccount;
}

void usage(void)
{
    fprintf(stderr, "Usage: wc [-lwcS] [fichier ...]\n");
    fprintf(stderr, "  -l  compter les lignes\n");
    fprintf(stderr, "  -w  compter les mots\n");
    fprintf(stderr, "  -c  compter les caracteres\n");
    fprintf(stderr, "  -S  afficher les mots uniques et leurs occurrences\n");
    exit(1);
}