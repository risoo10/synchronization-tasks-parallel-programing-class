/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V pekarni pracuju pekari (10 pekarov), ktori pecu chlieb v peciach (4 pece). Pekar pripravuje chlieb nejaky cas (v simulacii 4s) a potom ide k volnej peci a pecie v nej chlieb (2s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko chlebov bolo upecenych. [2b]

2. Zabezpecte, aby do obsadenej pece pekar vlozil chlieb az ked sa uvolni, cize aby poclal, kym nebude nejaka pec volna. Simulujte situaciu, ze ked pekar upecie 2 chleby, pocka na vsetkych kolegov a spravia si prestavku (v simulacii 4s). [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby pekar prerusil cinnost hned, ako je to mozne (ak uz zacal pripravu alebo pecenie moze ju dokoncit).  [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc pekari.c -o pekari -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define POCET_PEKAROV 10
#define POCET_PECI 4


// signal na zastavenie simulacie
int stoj = 0;
int smeVsetci = 0;

int volnePece = POCET_PECI, oddychujuci = 0;

int chleby[POCET_PEKAROV] = {0}, pocetChlebov = 0;

// Synchronizacne prostriedky
pthread_mutex_t mutexChleby = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSynchronizacia = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexOddychovanie = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condPecenie = PTHREAD_COND_INITIALIZER;
pthread_cond_t  condOddych = PTHREAD_COND_INITIALIZER;


// pekar
void priprava(int pekar) {



    // Priprava noveho chleba
    sleep(4);
}

void pecenie(int pekar) {

    // ukonci ak je koniec
    if(stoj){
        return;
    }

    pthread_mutex_lock(&mutexSynchronizacia);

    // Zaspi ak nie je volna pec alebo skonci ak je koniec
    while(volnePece == 0){
        if(stoj){
            return ;
        }
        pthread_cond_wait(&condPecenie, &mutexSynchronizacia);
    }

    printf("Pekar %d : idem piect | volne pece: %d \n", pekar, volnePece);

    // Obsad pec
    volnePece--;

    pthread_mutex_unlock(&mutexSynchronizacia);

    // Pecenie chleba
    sleep(2);
    // Dopiekol chleba

    // Uvolni Pec
    pthread_mutex_lock(&mutexSynchronizacia);

    // Zapis vysledok
    chleby[pekar]++;
    pocetChlebov++;
    printf("Pekar %d : Huraa mam %d. chlebik \n", pekar, chleby[pekar]);

    volnePece++;

    // Signalizuj Uvolnenie
    pthread_cond_broadcast(&condPecenie);

    pthread_mutex_unlock(&mutexSynchronizacia);

    // ODDYCH !!!!!
    // po upeceni 2 chlebov => oddychuje so vsetkymi
    if(chleby[pekar] % 2 == 0 && chleby[pekar] > 0){

        pthread_mutex_lock(&mutexOddychovanie);

        // Zvys pocet cakajucich
        oddychujuci++;
        printf("Pekar %d : Cakam na oddych ako %d. \n", pekar, oddychujuci);


        // Ak ide oddychovat posledny - zavel oddych alebo skonci ak je koniec
        if(oddychujuci == POCET_PEKAROV){
            smeVsetci = 1;
            printf("Pekar %d : Uz sme vsetci !\n", pekar);
            pthread_cond_broadcast(&condOddych);
        } else {

            // Cakaj kym nie su vsetci az potom zacni oddychovat
            while(!smeVsetci){
                if(stoj){
                    return;
                }
                pthread_cond_wait(&condOddych, &mutexOddychovanie);
            }
        }

        // Odpocitaj odchadzajucich

        oddychujuci--;

        printf("Pekar %d : Odchadzzam oddychovat ako %d. \n", pekar, 10 - oddychujuci);

        // Posledny ukonci cakanie
        if( oddychujuci == 0){
            smeVsetci = 0;
            printf("Pekar %d : A uz oddychujeme vsetci !\n", pekar);
        }

        pthread_mutex_unlock(&mutexOddychovanie);

        // PAUZA
        sleep(4);
    }

}




void *pekar( void *ptr ) {
    int pekar = (int) ptr;

    while(!stoj) {
        priprava(pekar);
        pecenie(pekar);
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t pekari[10];

    for (i=0;i<10;i++) pthread_create( &pekari[i], NULL, &pekar, (void *)i);

    sleep(30);


    pthread_mutex_lock(&mutexChleby);

    // Zastav simulaciu
    stoj = 1;

    printf("Koniec simulacie !!!!!\n");

    // Prebud pekarov aby skoncili ak zaspali
    pthread_cond_broadcast(&condOddych);
    pthread_cond_broadcast(&condPecenie);

    pthread_mutex_unlock(&mutexChleby);

    for (i=0;i<10;i++) pthread_join( pekari[i], NULL);

    // Konecne vysledky
    printf("Pocet chlebov: %d\n", pocetChlebov);
    for (i=0;i<10;i++) printf("Pekar %d spravil %d chlebov.\n", i, chleby[i]);

    exit(EXIT_SUCCESS);
}
