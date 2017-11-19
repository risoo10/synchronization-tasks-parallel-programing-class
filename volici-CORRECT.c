/*
https://goo.gl/ESYaxB

Meno:
Datum:

Simulujte nasledujucu situaciu. V obci su volby a 100 volicov ide volit do volebnej miestnosti, kde su 3 plenty a 1 volebna urna. Volic prichadza do miestnosti (v simulacii kazdu 1s) najskor kruzkuje kandidatov (v simulacii 2s) a nasledne vhadzuje hlasovaci listok do urny (v simulacii 1s). Simulacia konci ked odhlasuju vsetci volici.

1. Doplnte do programu premennu pocitajucu pocet volicov, ktori uz odvolili; hodnota nech je programom vypisovana kazdych 5s. [4b]

2. Zabezpecte synchronizaciu, tak, aby subezne mohli iba traja volici kruzkovat kandidatov za plentami a iba jeden volic vhadzovat listok do urny. [6b]

Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne; resp monitory
- nespoliehajte sa na uvedene casy ci pocty, simulacia by mala fungovat aj s inymi casmi alebo s nahodne generovanymi casmi alebo poctami
- build (console): gcc volici.c -o volici -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define POCET_VOLICOV 100
#define CAS_KRUZKUJ 2
#define CAS_VHADZUJ 1
#define CAS_VOLIC 1

#define TRUE 1
#define FALSE 0

// POcitadla
int pocetVolicov = 0, kruzkuje = 0;

// Stavy
int koniec = FALSE;

// Sycnchr. konstrukcie
pthread_mutex_t mutexPremenne = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexUrna = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condKruzkovat = PTHREAD_COND_INITIALIZER;

// volic
void kruzkuj(void) {

    // Zaspi ak uz kruzkuju traja volici
    pthread_mutex_lock(&mutexPremenne);
    while(kruzkuje == 3){
        pthread_cond_wait(&condKruzkovat, &mutexPremenne);
    }
    // Zapocitaj ze kruzkuje
    kruzkuje++;

    pthread_mutex_unlock(&mutexPremenne);

    // Kruzkuje za urnou
    sleep(CAS_KRUZKUJ);

    pthread_mutex_lock(&mutexPremenne);

    // Uvolni miesto dalsiemu volicovi
    kruzkuje--;
    pthread_cond_signal(&condKruzkovat);

    pthread_mutex_unlock(&mutexPremenne);

}

void vhadzuj(void) {
    pthread_mutex_lock(&mutexUrna);
    sleep(CAS_VHADZUJ);
    pocetVolicov++;
    pthread_mutex_unlock(&mutexUrna);
}

void *volic(void *ptr) {
    kruzkuj();
    vhadzuj();
    return NULL;
}

void *pocitaj(void *ptr){
    while(!koniec){
        printf("Pocet uspesnych volicov: %d\n", pocetVolicov);
        sleep(5);
    }

    // Vypis nakoniec
    printf("Pocet uspesnych volicov: %d\n", pocetVolicov);
    return NULL;
}

// main f.
int main(void) {
    pthread_t volici[POCET_VOLICOV];

    // Spusti pocitadlo
    pthread_t pocitadlo;
    pthread_create(&pocitadlo, NULL, &pocitaj, NULL);

    for (int i=0; i<POCET_VOLICOV; i++) {
        pthread_create(&volici[i], NULL, &volic, NULL);
        sleep(CAS_VOLIC);
    }

    for (int i=0; i<POCET_VOLICOV; i++) pthread_join(volici[i], NULL);


    // Ukonci pocitadlo
    pthread_mutex_lock(&mutexPremenne);
    koniec = TRUE;
    pthread_mutex_unlock(&mutexPremenne);

    pthread_join(pocitadlo, NULL);


    exit(EXIT_SUCCESS);
}
