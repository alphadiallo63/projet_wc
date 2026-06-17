# legacy_rescue

Projet de système TP — remise en état et extension d'un outil Unix legacy.

On est partis d'un `wc.c` monolithique des années 80 et on a abouti à une
architecture modulaire avec une bibliothèque dynamique réutilisable.

---

## Ce que fait le projet

`wc` compte les lignes, mots et caractères d'un fichier ou d'un flux de texte,
comme l'outil Unix classique. On y a ajouté l'option `-S` qui affiche tous les
mots uniques du texte avec leur nombre d'occurrences, triés du plus fréquent
au moins fréquent.

Le dictionnaire qui permet ça est compilé séparément en bibliothèque dynamique
(`libdico.so`) pour pouvoir être réutilisé dans d'autres projets.

---

## Arborescence

```
legacy_rescue/
├── Makefile                  ← compile tout depuis la racine
│
├── libdico/                  ← MODULE 1 : la bibliothèque dictionnaire
│   ├── Makefile
│   ├── src/
│   │   ├── dico.h            ← interface publique (type opaque)
│   │   ├── dico.c            ← implémentation privée
│   │   └── benchmark.c       ← tests de performance du dictionnaire
│   └── lib/
│       └── libdico.so        ← bibliothèque dynamique compilée
│
└── wc/                       ← MODULE 2 : l'application
    ├── Makefile
    ├── src/
    │   ├── wc.h              ← structure wc_counts_t + prototypes
    │   ├── wc_core.c         ← logique de comptage
    │   └── main.c            ← point d'entrée, gestion des options
    └── bin/
        └── wc                ← exécutable final
```

---

## Compilation

Depuis la racine du projet :

```bash
make          # compile libdico.so puis wc
make clean    # supprime les fichiers compilés
```

L'ordre est important : `libdico` est compilé en premier car `wc` en dépend.

---

## Utilisation

```bash
# Comportement classique (lignes, mots, caractères)
./wc/bin/wc fichier.txt

# Flags disponibles
./wc/bin/wc -l fichier.txt     # lignes seulement
./wc/bin/wc -w fichier.txt     # mots seulement
./wc/bin/wc -c fichier.txt     # caractères seulement
./wc/bin/wc -S fichier.txt     # mots uniques triés par occurrences

# Combinaisons
./wc/bin/wc -lwc fichier.txt
./wc/bin/wc -lwcS fichier.txt

# Plusieurs fichiers (affiche un total)
./wc/bin/wc -lwc fichier1.txt fichier2.txt

# Depuis stdin
echo "le chat mange le chat" | ./wc/bin/wc -S
```

### Exemple de sortie avec `-S`

```
$ echo "le chat mange le chat et le chien mange le chat" | ./wc/bin/wc -S

=== Mots uniques — stdin (5 mots distincts) ===
  le                             4
  chat                           3
  mange                          2
  chien                          1
  et                             1
```

---

## Ce qu'on a fait et pourquoi

### Phase 1 — Modularisation de wc

Le `wc.c` original était un fichier monolithique avec des variables globales
partout. On l'a découpé en trois fichiers avec des responsabilités claires et
on a regroupé les compteurs dans une structure `wc_counts_t` passée par
pointeur — plus de variables globales.

### Phase 2 — Le dictionnaire

`dico.c` implémente une table de hachage avec chaînage. Points importants :

- **Type opaque** : l'utilisateur ne voit pas les champs internes de `dict_t`,
  il est obligé de passer par les fonctions publiques. Tentative d'accès direct
  = erreur de compilation.
- **Taille puissance de 2** : permet de remplacer le modulo (`%`, ~40 cycles CPU)
  par un AND bitwise (`&`, ~1 cycle). Gain mesuré ×3 sur le test intensif.
- **Redimensionnement automatique** : quand le facteur de charge dépasse 3.0,
  la table double de taille. Les nœuds existants sont réutilisés tels quels,
  zéro malloc pendant le resize.
- **`dict_remove`** : suppression propre grâce au double pointeur de la fonction
  de recherche interne.

### Phase 3 — La greffe

On a branché `libdico.so` sur `wc` :

- `getopt` remplace le parsing manuel des options.
- L'option `-S` crée un dictionnaire par fichier, le remplit pendant le
  comptage, puis l'affiche trié par occurrences décroissantes via `qsort`.
- `wc` est lié **dynamiquement** contre `libdico.so` — le code du dictionnaire
  n'est pas dans le binaire `wc`, il est chargé au démarrage.

---

## Choix techniques notables

**`dict_foreach`** — plutôt que d'exposer les internals du dictionnaire pour
pouvoir itérer dessus, on expose une fonction qui prend un callback. Le code
appelant ne voit jamais `dict_entry_t`.

**Tri de dernière minute** — `dict_foreach` collecte les paires (mot, count)
dans un tableau dynamique, `qsort` trie avec un comparateur décroissant.
Ajout propre sans toucher au dictionnaire.

**Un dictionnaire par fichier** — volontaire. Fusionner les dictionnaires de
plusieurs fichiers aurait complexifié le code pour un cas d'usage pas demandé.

---

## Valgrind

```
==648== HEAP SUMMARY:
==648==   total heap usage: 8,895 allocs, 8,895 frees, 73,288 bytes allocated
==648== All heap blocks were freed -- no leaks are possible
==648== ERROR SUMMARY: 0 errors from 0 contexts
```

Zéro fuite mémoire.
