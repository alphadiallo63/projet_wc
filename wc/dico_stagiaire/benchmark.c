/*
 * benchmark.c — Mesure des performances du dictionnaire.
 *
 * Ce fichier ne contient QUE le code de test/benchmark.
 * Il n'a pas accès aux internals de dico.c : il passe uniquement
 * par l'interface publique déclarée dans dico.h.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "dico.h"

/* =========================================================================
 * UTILITAIRES BENCHMARK
 * ========================================================================= */

/* Horodatage en microsecondes (monotone, non affecté par les changements d'heure) */
static uint64_t now_us(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL
         + (uint64_t)ts.tv_nsec / 1000ULL;
}

/*
 * Charge toutes les lignes d'un fichier en mémoire AVANT de lancer
 * les chronos — pour ne pas mesurer la vitesse du disque.
 * Retourne un tableau de chaînes allouées par strdup().
 * *out_count est mis à jour avec le nombre de lignes lues.
 */
static char **load_dataset(const char *filename, size_t *out_count)
{
    FILE *f = fopen(filename, "r");
    if (!f) { perror(filename); return NULL; }

    size_t  capacity = 100000;
    char  **words    = malloc(capacity * sizeof(char *));
    size_t  count    = 0;
    char   *line     = NULL;
    size_t  len      = 0;

    while (getline(&line, &len, f) != -1) {
        if (count >= capacity) break;
        line[strcspn(line, "\r\n")] = '\0';   /* supprime le \n final */
        words[count++] = strdup(line);
    }

    free(line);
    fclose(f);
    *out_count = count;
    printf("[INFO] %zu mots chargés depuis '%s'\n", count, filename);
    return words;
}

/* =========================================================================
 * MAIN
 * ========================================================================= */

int main(void)
{
    /* --- Chargement du jeu de données --- */
    size_t  word_count = 0;
    char  **dataset    = load_dataset("mots_17479.txt", &word_count);

    if (!dataset || word_count == 0) {
        fprintf(stderr, "Erreur : impossible de charger le dataset.\n");
        return 1;
    }

    dict_t *dico = dict_create();
    if (!dico) {
        fprintf(stderr, "Erreur : dict_create() a échoué.\n");
        return 1;
    }

    printf("\n=== DEBUT DU BENCHMARK ===\n");

    /* ------------------------------------------------------------------ */
    /* TEST 1 : INSERTION                                                  */
    /* ------------------------------------------------------------------ */
    uint64_t t0 = now_us();
    for (size_t i = 0; i < word_count; i++) {
        int val = (int)i;
        dict_add(dico, dataset[i], strlen(dataset[i]) + 1, &val, sizeof(int));
    }
    size_t   uniq_word_count  = dict_len(dico);
    uint64_t t1               = now_us();

    double total_insert_ms = (t1 - t0) / 1000.0;
    double ns_per_insert   = ((double)(t1 - t0) * 1000.0) / word_count;

    printf("\n[INSERTION]\n");
    printf("Mots uniques : %zu  /  total lignes : %zu\n", uniq_word_count, word_count);
    printf("Total        : %.3f ms\n", total_insert_ms);
    printf("Moyenne      : %.2f ns / mot\n", ns_per_insert);

    /* ------------------------------------------------------------------ */
    /* TEST 2 : RECHERCHE POSITIVE (clés présentes)                       */
    /* ------------------------------------------------------------------ */
    t0 = now_us();
    int found_count = 0;
    for (size_t i = 0; i < word_count; i++) {
        if (dict_contains(dico, dataset[i], strlen(dataset[i]) + 1) == DICT_OK)
            found_count++;
    }
    t1 = now_us();

    double ns_per_find_hit = ((double)(t1 - t0) * 1000.0) / word_count;
    printf("\n[RECHERCHE HIT — mots existants]\n");
    printf("Trouvés  : %d / %zu\n", found_count, word_count);
    printf("Moyenne  : %.2f ns / recherche\n", ns_per_find_hit);

    /* ------------------------------------------------------------------ */
    /* TEST 3 : RECHERCHE NEGATIVE (clés absentes)                        */
    /* C'est souvent le pire cas : on parcourt toute la liste de collision */
    /* ------------------------------------------------------------------ */
    t0 = now_us();
    int   not_found_count = 0;
    char  buffer[256];

    for (size_t i = 0; i < word_count; i++) {
        snprintf(buffer, sizeof(buffer), "%s_X", dataset[i]);
        if (dict_contains(dico, buffer, strlen(buffer) + 1) != DICT_OK)
            not_found_count++;
    }
    t1 = now_us();

    double ns_per_find_miss = ((double)(t1 - t0) * 1000.0) / word_count;
    printf("\n[RECHERCHE MISS — mots inexistants]\n");
    printf("Non trouvés : %d / %zu\n", not_found_count, word_count);
    printf("Moyenne     : %.2f ns / recherche\n", ns_per_find_miss);

    /* ------------------------------------------------------------------ */
    /* ANALYSE : facteur de charge                                         */
    /* ------------------------------------------------------------------ */
    printf("\n=== ANALYSE ===\n");
    double load_factor = (double)uniq_word_count / (double)word_count;
    printf("Facteur de charge : %.2f\n", load_factor);
    if (load_factor > 5.0) {
        printf("ALERTE : facteur élevé → beaucoup de collisions"
               " (≈ %.0f éléments par case).\n", load_factor);
        printf("La recherche est lente à cause des longues listes chaînées.\n");
    }

    /* ------------------------------------------------------------------ */
    /* TEST 4 : RECHERCHE INTENSIVE sur une seule clé (chauffe le cache)  */
    /* ------------------------------------------------------------------ */
    printf("\nTest intensif sur une clé unique (10 000 000 itérations)\n");
    const char *hot_key    = dataset[uniq_word_count - 1];
    size_t      hot_key_len = strlen(hot_key) + 1;

    t0 = now_us();
    for (int i = 0; i < 10000000; i++)
        dict_contains(dico, hot_key, hot_key_len);
    t1 = now_us();

    double ns_per_find = ((double)(t1 - t0) * 1000.0) / 10000000;
    printf("Durée par recherche : %.2f ns\n", ns_per_find);

    /* ------------------------------------------------------------------ */
    /* NETTOYAGE                                                           */
    /* ------------------------------------------------------------------ */
    dict_destroy(dico);
    for (size_t i = 0; i < word_count; i++)
        free(dataset[i]);
    free(dataset);

    return 0;
}
