#include <stdio.h>
#include <stdlib.h>
#include "wc.h"

int main(int argc, char *argv[])
{
    int k;
    char *cp;
    int tflag, files;
    int lflag = 0, wflag = 0, cflag = 0;
    wc_counts_t counts = {0};   /* Tous les compteurs initialisés à 0 */

    /* Lecture des flags */
    files = argc - 1;
    k = 1;
    cp = argv[1];
    if (argc > 1 && *cp++ == '-') {
        files--;
        k++;    /* pointe sur le premier fichier */
        while (*cp != 0) {
            switch (*cp) {
                case 'l': lflag++; break;
                case 'w': wflag++; break;
                case 'c': cflag++; break;
                default:  usage();
            }
            cp++;
        }
    }

    /* Sans flag : comportement par défaut -lwc */
    if (!lflag && !wflag && !cflag) {
        lflag = 1;
        wflag = 1;
        cflag = 1;
    }

    tflag = files >= 2;     /* afficher le total si plusieurs fichiers */

    /* Lecture depuis stdin */
    if (k >= argc) {
        count(stdin, &counts);
        if (lflag) printf(" %6ld", counts.lcount);
        if (wflag) printf(" %6ld", counts.wcount);
        if (cflag) printf(" %6ld", counts.ccount);
        printf(" \n");
        fflush(stdout);
        exit(0);
    }

    /* Boucle sur les fichiers passés en argument */
    while (k < argc) {
        FILE *f;

        if ((f = fopen(argv[k], "r")) == NULL) {
            fprintf(stderr, "wc: cannot open %s\n", argv[k]);
        } else {
            count(f, &counts);
            if (lflag) printf(" %6ld", counts.lcount);
            if (wflag) printf(" %6ld", counts.wcount);
            if (cflag) printf(" %6ld", counts.ccount);
            printf(" %s\n", argv[k]);
            fclose(f);
        }
        k++;
    }

    /* Affichage du total si plusieurs fichiers */
    if (tflag) {
        if (lflag) printf(" %6ld", counts.ltotal);
        if (wflag) printf(" %6ld", counts.wtotal);
        if (cflag) printf(" %6ld", counts.ctotal);
        printf(" total\n");
    }

    fflush(stdout);
    exit(0);
}