#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "wc.h"

typedef struct {
    char *word;
    int   count;
} word_count_t;

typedef struct {
    word_count_t *entries;
    size_t        size;
    size_t        capacity;
} collect_ctx_t;

static void collect_entry(const char *key, int value, void *arg)
{
    collect_ctx_t *ctx = (collect_ctx_t *)arg;
    if (ctx->size >= ctx->capacity) {
        ctx->capacity *= 2;
        ctx->entries = realloc(ctx->entries,
                               ctx->capacity * sizeof(word_count_t));
        if (!ctx->entries) { perror("realloc"); exit(1); }
    }
    ctx->entries[ctx->size].word  = strdup(key);
    ctx->entries[ctx->size].count = value;
    ctx->size++;
}

static int cmp_desc(const void *a, const void *b)
{
    const word_count_t *wa = (const word_count_t *)a;
    const word_count_t *wb = (const word_count_t *)b;
    if (wb->count != wa->count)
        return wb->count - wa->count;
    return strcmp(wa->word, wb->word);
}

static void print_sorted_unique(const dict_t *dict, const char *filename)
{
    collect_ctx_t ctx;
    ctx.capacity = 256;
    ctx.size     = 0;
    ctx.entries  = malloc(ctx.capacity * sizeof(word_count_t));
    if (!ctx.entries) { perror("malloc"); return; }

    dict_foreach(dict, collect_entry, &ctx);
    qsort(ctx.entries, ctx.size, sizeof(word_count_t), cmp_desc);

    printf("\n=== Mots uniques — %s (%zu mots distincts) ===\n",
           filename, ctx.size);
    for (size_t i = 0; i < ctx.size; i++)
        printf("  %-30s %d\n", ctx.entries[i].word, ctx.entries[i].count);

    for (size_t i = 0; i < ctx.size; i++)
        free(ctx.entries[i].word);
    free(ctx.entries);
}

int main(int argc, char *argv[])
{
    int lflag = 0, wflag = 0, cflag = 0, sflag = 0;
    int opt;

    while ((opt = getopt(argc, argv, "lwcS")) != -1) {
        switch (opt) {
            case 'l': lflag = 1; break;
            case 'w': wflag = 1; break;
            case 'c': cflag = 1; break;
            case 'S': sflag = 1; break;
            default:  usage();
        }
    }

    if (!lflag && !wflag && !cflag) {
        lflag = 1; wflag = 1; cflag = 1;
    }

    int         file_start = optind;
    int         file_count = argc - optind;
    int         tflag      = file_count >= 2;
    wc_counts_t counts     = {0};

    if (file_count == 0) {
        dict_t *dict = sflag ? dict_create() : NULL;
        count(stdin, &counts, dict);
        if (lflag) printf(" %6ld", counts.lcount);
        if (wflag) printf(" %6ld", counts.wcount);
        if (cflag) printf(" %6ld", counts.ccount);
        printf("\n");
        if (sflag && dict) { print_sorted_unique(dict, "stdin"); dict_destroy(dict); }
        fflush(stdout);
        return 0;
    }

    for (int i = file_start; i < argc; i++) {
        FILE *f = fopen(argv[i], "r");
        if (!f) { fprintf(stderr, "wc: cannot open %s\n", argv[i]); continue; }

        dict_t *dict = sflag ? dict_create() : NULL;
        count(f, &counts, dict);
        fclose(f);

        if (lflag) printf(" %6ld", counts.lcount);
        if (wflag) printf(" %6ld", counts.wcount);
        if (cflag) printf(" %6ld", counts.ccount);
        printf(" %s\n", argv[i]);

        if (sflag && dict) {
            print_sorted_unique(dict, argv[i]);
            dict_destroy(dict);
        }
    }

    if (tflag) {
        if (lflag) printf(" %6ld", counts.ltotal);
        if (wflag) printf(" %6ld", counts.wtotal);
        if (cflag) printf(" %6ld", counts.ctotal);
        printf(" total\n");
    }

    fflush(stdout);
    return 0;
}