#ifndef DICO_H
#define DICO_H

#include <stddef.h>  /* size_t */

/* =========================================================================
 * TYPE OPAQUE
 *
 * L'utilisateur sait que dict_t existe, mais ne voit pas ce qu'il contient.
 * Il est obligé de passer par les fonctions ci-dessous pour manipuler
 * le dictionnaire. Toute tentative d'accès direct à un champ (dict->size)
 * provoquera une erreur de compilation : "dereferencing pointer to
 * incomplete type".
 * ========================================================================= */
typedef struct dict_struct dict_t;

/* =========================================================================
 * CODES DE RETOUR
 *
 * Toutes les fonctions retournent un de ces codes pour signaler
 * ce qui s'est passé. Pas de valeurs magiques (-1, 0, 1...) éparpillées.
 * ========================================================================= */
typedef enum {
    DICT_NOK = 0,          /* Échec générique                  */
    DICT_OK,               /* Succès                           */
    DICT_VALUE_UPDATED,    /* Clé existante, valeur mise à jour */
    DICT_ERR_NOT_FOUND,    /* Clé absente du dictionnaire      */
    DICT_ERR_MALLOC        /* Échec d'allocation mémoire       */
} dict_status_t;

/* =========================================================================
 * INTERFACE PUBLIQUE
 *
 * Ce sont les SEULES fonctions que l'utilisateur (benchmark.c, main.c...)
 * a le droit d'appeler. Tout le reste (fnv1a_32, internal_*, dict_entry_t)
 * reste caché dans dico.c.
 * ========================================================================= */

/* Crée un dictionnaire vide. Retourne NULL en cas d'échec mémoire. */
dict_t *dict_create(void);

/* Libère toute la mémoire du dictionnaire (clés, valeurs, entrées, table). */
void dict_destroy(dict_t *dict);

/* Retourne le nombre de clés uniques stockées. */
size_t dict_len(const dict_t *dict);

/*
 * Insère ou met à jour une entrée.
 *
 * - Si value == NULL ou value_len == 0 : fonctionne comme un "set"
 *   (on stocke juste la clé, sans valeur associée).
 * - Si la clé existe déjà : la valeur est remplacée, retourne DICT_VALUE_UPDATED.
 * - Si c'est une nouvelle clé : retourne DICT_OK.
 * - En cas d'échec mémoire : retourne DICT_ERR_MALLOC.
 */
dict_status_t dict_add(dict_t *dict, void *key, size_t key_len,
                       void *value, size_t value_len);

/*
 * Vérifie si une clé est présente.
 * Retourne DICT_OK si trouvée, DICT_ERR_NOT_FOUND sinon.
 */
dict_status_t dict_contains(const dict_t *dict, const void *key, size_t key_len);

/*
 * Récupère la valeur associée à une clé.
 *
 * Si trouvée : *value_ptr pointe sur la valeur, *value_len est renseigné,
 *              retourne DICT_OK.
 * Si absente : *value_ptr = NULL, *value_len = 0, retourne DICT_ERR_NOT_FOUND.
 */
dict_status_t dict_get_value(const dict_t *dict, const void *key, size_t key_len,
                             const void **value_ptr, size_t *value_len);

#endif /* DICO_H */
