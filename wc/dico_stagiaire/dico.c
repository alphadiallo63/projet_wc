/*
 * dico.c — Implémentation privée du dictionnaire / set générique.
 *
 * Ne *PAS* utiliser cette implémentation pour du code de production !
 * Il s'agit d'une démonstration du concept de set / dictionnaire,
 * pas d'un code optimisé. Voir les commentaires dans le .h et le README
 * pour les pistes d'amélioration.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dico.h"

/* =========================================================================
 * CONSTANTES PRIVÉES  (invisibles depuis dico.h)
 * ========================================================================= */
#define HASH_TABLE_DEFAULT_SIZE 503

/* =========================================================================
 * TYPES PRIVÉS  (invisibles depuis dico.h)
 * ========================================================================= */

/* Nœud de la liste chaînée stocké dans chaque bucket */
typedef struct dict_entry {
    uint32_t      hash;
    void         *raw_key;
    size_t        raw_key_len;
    void         *value;
    size_t        value_len;
    struct dict_entry *next;
} dict_entry_t;

/*
 * Définition COMPLÈTE de dict_struct — connue uniquement de ce fichier.
 * Dans dico.h on n'a fait que déclarer "struct dict_struct existe",
 * sans révéler ses champs.
 */
struct dict_struct {
    dict_entry_t **table;
    size_t         table_len;
    size_t         key_nb;
};

/* =========================================================================
 * FONCTIONS PRIVÉES  (static = invisibles à l'éditeur de liens)
 * ========================================================================= */

/* Algorithme FNV-1a 32 bits : transforme n'importe quelle donnée en hash */
static uint32_t fnv1a_32(const void *data, size_t len)
{
    uint32_t hash = 0x811c9dc5;                /* offset_basis 32 bits */
    const unsigned char *ptr = (const unsigned char *)data;

    for (size_t i = 0; i < len; i++) {
        hash ^= ptr[i];        /* XOR avec l'octet courant  */
        hash *= 0x01000193;    /* multiplication FNV_prime  */
    }
    return hash;
}

/* Libère un nœud (clé + valeur + structure) */
static void dict_entry_destroy(dict_entry_t *entry)
{
    free(entry->value);
    free(entry->raw_key);
    free(entry);
}

/*
 * Compare un nœud existant avec une clé candidate.
 * On vérifie d'abord le hash (comparaison entière, rapide) avant
 * d'appeler memcmp (comparaison octet par octet, plus coûteuse).
 */
static int internal_dict_equal_key(const dict_entry_t *a, uint32_t hash,
                                   const void *raw_key, size_t raw_key_len)
{
    return (a->hash == hash
            && a->raw_key_len == raw_key_len
            && memcmp(a->raw_key, raw_key, raw_key_len) == 0);
}

/*
 * Retourne un pointeur vers le pointeur qui "tient" l'entrée cherchée,
 * ou vers le dernier NULL de la liste si la clé est absente.
 *
 * Ce double pointeur (dict_entry_t**) permet à dict_add d'insérer
 * directement sans cas particuliers : *ptr = new_node suffit.
 *
 * CORRECTION du bug original : on utilise dict->table_len
 * (taille réelle) et non la constante HASH_TABLE_DEFAULT_SIZE,
 * pour que le code survive à un futur redimensionnement.
 */
static dict_entry_t **internal_dict_find_entry_ptr(const dict_t *dict,
                                                    const void *raw_key,
                                                    size_t raw_key_len,
                                                    uint32_t hash)
{
    dict_entry_t **ptr = &(dict->table[hash % dict->table_len]);  /* BUG CORRIGÉ */
    while (*ptr) {
        if (internal_dict_equal_key(*ptr, hash, raw_key, raw_key_len))
            return ptr;
        ptr = &((*ptr)->next);
    }
    return ptr;
}

/* Copie src dans un nouveau bloc malloc, ou retourne NULL si src est vide */
static void *internal_helper_copy_or_null(const void *src, size_t len)
{
    if (!src || len == 0) return NULL;
    void *dest = malloc(len);
    if (dest) memcpy(dest, src, len);
    return dest;
}

/* =========================================================================
 * FONCTIONS PUBLIQUES  (déclarées dans dico.h)
 * ========================================================================= */

dict_t *dict_create(void)
{
    dict_t *dict = calloc(1, sizeof(*dict));
    if (!dict)
        return NULL;

    dict->table = calloc(HASH_TABLE_DEFAULT_SIZE, sizeof(dict_entry_t *));
    if (!dict->table) {
        free(dict);
        return NULL;
    }

    dict->table_len = HASH_TABLE_DEFAULT_SIZE;
    dict->key_nb    = 0;
    return dict;
}

void dict_destroy(dict_t *dict)
{
    if (!dict) return;

    for (size_t i = 0; i < dict->table_len; i++) {
        dict_entry_t *cur = dict->table[i];
        while (cur) {
            dict_entry_t *tmp = cur->next;
            dict_entry_destroy(cur);
            cur = tmp;
        }
    }
    free(dict->table);
    free(dict);
}

size_t dict_len(const dict_t *dict)
{
    return dict->key_nb;
}

dict_status_t dict_add(dict_t *dict, void *key, size_t key_len,
                       void *value, size_t value_len)
{
    uint32_t h = fnv1a_32(key, key_len);
    dict_entry_t **entry_ptr = internal_dict_find_entry_ptr(dict, key, key_len, h);

    if (*entry_ptr) {
        /* Clé déjà connue : on met à jour la valeur */
        void *new_val = internal_helper_copy_or_null(value, value_len);
        if (value && value_len && !new_val)
            return DICT_ERR_MALLOC;

        free((*entry_ptr)->value);
        (*entry_ptr)->value     = new_val;
        (*entry_ptr)->value_len = value_len;
        return DICT_VALUE_UPDATED;
    }

    /* Nouvelle clé (ou collision résolue par chaînage) */
    dict_entry_t *new_node = calloc(1, sizeof(*new_node));
    if (!new_node)
        return DICT_ERR_MALLOC;

    new_node->raw_key = internal_helper_copy_or_null(key, key_len);
    new_node->value   = internal_helper_copy_or_null(value, value_len);

    if (!new_node->raw_key || (value_len > 0 && !new_node->value)) {
        free(new_node->raw_key);
        free(new_node->value);
        free(new_node);
        return DICT_ERR_MALLOC;
    }

    new_node->hash        = h;
    new_node->raw_key_len = key_len;
    new_node->value_len   = value_len;

    *entry_ptr = new_node;   /* accrochage en fin de liste (ou dans le bucket vide) */
    dict->key_nb++;
    return DICT_OK;
}

dict_status_t dict_contains(const dict_t *dict, const void *key, size_t key_len)
{
    uint32_t h = fnv1a_32(key, key_len);
    const dict_entry_t *ret = *internal_dict_find_entry_ptr(dict, key, key_len, h);
    return ret ? DICT_OK : DICT_ERR_NOT_FOUND;
}

dict_status_t dict_get_value(const dict_t *dict, const void *key, size_t key_len,
                             const void **value_ptr, size_t *value_len)
{
    uint32_t h = fnv1a_32(key, key_len);
    const dict_entry_t *ret = *internal_dict_find_entry_ptr(dict, key, key_len, h);
    if (!ret) {
        *value_ptr = NULL;
        *value_len = 0;
        return DICT_ERR_NOT_FOUND;
    }
    *value_ptr = ret->value;
    *value_len = ret->value_len;
    return DICT_OK;
}
