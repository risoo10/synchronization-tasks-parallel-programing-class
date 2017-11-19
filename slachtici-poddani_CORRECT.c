/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V malom kralovstve korunovali noveho krala a chodia sa mu neustale klanat styria slachtici a desiati poddani. Prejavovanie ucty kralovi trva nejaky cas (v simulacii 1s) a nejaky cas si slahctic ci poddany dava prestavku (v simulacii 4s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko krat sa kralovi poklonili slachtici; a pocitadlo pocitajuce, kolko krat sa kralovi poklonili poddani. [2b]

2. Zabezpecte, aby sa kralovi sucasne klanali maximalne dvaja slachtici a tiez aby sa kralovi neklanal slachtic spolu s poddanym (cize alebo max. 2 slachtici, alebo lubovolne vela poddanych). Ak je pred kralom rad, slachtici maju samozrejme prednost. [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc poslovia_a_pisari -o poslovia_a_pisari -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0

// signal na zastavenie simulacie
int stoj = 0, slachticiCount = 0, poddaniCount = 0;
int cakajuciSlachtici = 0, slachticiVnutri = 0, poddaniVnutri = 0;

// Synchornizacne konstrukcie
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPoddani = PTHREAD_COND_INITIALIZER;
pthread_cond_t condSlachtici = PTHREAD_COND_INITIALIZER;



// klananie sa
void klananie(void) {
    sleep(1);
}

// prestavka medzi klananiami
void prestavka(void) {
    sleep(4);
}

// slachtic
void *slachtic( void *ptr ) {

    // pokial nie je zastaveny
    while(!stoj) {

        // Zaznamenaj cakajucich slachticov
        pthread_mutex_lock(&mutex);

        cakajuciSlachtici++;
        printf("SLA: cakam na klananie | cakajuci slachtici: %d\n", cakajuciSlachtici);

        // Pusti ku kralovi ak nie su dnu iny dvaja slachtici alebo poddani
        while(slachticiVnutri == 2 || poddaniVnutri > 1){

            // Ukonci ak skoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutex);
                return NULL;
            }

            pthread_cond_wait(&condSlachtici, &mutex);
        }

        // Idu sa klanat ku kralovi
        cakajuciSlachtici--;

        slachticiVnutri++;

        printf("SLA: Idem sa klanat | vnutri slachticov: %d\n", slachticiVnutri);
        pthread_mutex_unlock(&mutex);

        // poklona u krala
        klananie();

        // Zaznamenaj cakajucich slachticov
        pthread_mutex_lock(&mutex);

        // Zaznamenaj odchod slachtica
        slachticiVnutri--;
        slachticiCount++;
        printf("SLA: Odchadzam | poklony celkovo: %d\n", slachticiCount);


        // Odchadzajuci slachtic sinalizuje cakajucim slachticom alebo poddanym
        if(cakajuciSlachtici > 0){
            pthread_cond_broadcast(&condSlachtici);
        } else if(cakajuciSlachtici == 0){
            pthread_cond_broadcast(&condPoddani);
        }


        pthread_mutex_unlock(&mutex);

        prestavka();
    }
    return NULL;
}

// poddany
void *poddany( void *ptr ) {

    // pokial nie je zastaveny
    while(!stoj) {

        // Vzajomne vylucuj
        pthread_mutex_lock(&mutex);

        while(cakajuciSlachtici > 0 || slachticiVnutri > 0){

            // Ukonci ak skoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutex);
                return NULL;
            }

            pthread_cond_wait(&condPoddani, &mutex);
        }

        // Eviduj poddanych vnutri
        poddaniVnutri++;
        printf("POD: Idem sa klanat | vnutri poddanych: %d\n", poddaniVnutri);

        pthread_mutex_unlock(&mutex);

        // Klananie poddaneho
        klananie();

        // Vzajomne vylucuj
        pthread_mutex_lock(&mutex);

        poddaniVnutri--;
        poddaniCount++;
        printf("POD: Odchadzam | poklony celkovo: %d\n", poddaniCount);

        // Posledny poddany vnutri signalizuje slachticom ze mozu ist dnu
        if(poddaniVnutri == 0 && cakajuciSlachtici == 0){
            pthread_cond_broadcast(&condPoddani);
        }

        pthread_mutex_unlock(&mutex);

        prestavka();
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t slachtici[4];
    pthread_t poddani[10];

    for (i=0;i<4;i++) pthread_create( &slachtici[i], NULL, &slachtic, NULL);
    for (i=0;i<10;i++) pthread_create( &poddani[i], NULL, &poddany, NULL);

    sleep(30);

    // Zastav simulaciu
    pthread_mutex_lock(&mutex);
    printf("Koniec simulacie !!! \n");
    stoj = 1;
    pthread_cond_broadcast(&condSlachtici);
    pthread_cond_broadcast(&condPoddani);
    pthread_mutex_unlock(&mutex);

    for (i=0;i<4;i++) pthread_join( slachtici[i], NULL);
    for (i=0;i<10;i++) pthread_join( poddani[i], NULL);


    printf("Poddani: %d klanani\n", poddaniCount);
    printf("Slachtici: %d klanani\n", slachticiCount);

    exit(EXIT_SUCCESS);
}
