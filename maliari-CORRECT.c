
/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Desiati maliari maluju steny.
Maliarovi trva nejaky cas, kym stenu maluje (v simulacii 2s) a nejaky cas, kym si ide nabrat farbu do vedra (v simulacii 1s). Cela simulacia nech trva nejaky cas (30s).

1. Doplnte do programu pocitadlo celkoveho poctu vedier minutej farby a tiez nech si kazdy maliar pocita, kolko vedier farby uz minul preniesol, na konci simulacie vypiste hodnoty pocitadiel. [2b]

2. Ked maliar minie 4 vedra, pocka na dvoch svojich kolegov a kazdy si spravi prestavku na nejaky cas (v simulacii 2s). [5b]

3. Osetrite v programe spravne ukoncenie simulacie hned po uplynuti stanoveneho casu (nezacne sa dalsia cinnost). [3b]


Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc maliari.c -o maliari -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0
#define POCET_MALIAROV 10
#define CAS_ODDYCHU 2

int pocetVedier = 0;
int vedra[POCET_MALIAROV] = {0};

// Stavy
int stoj = FALSE;
int ODDYCHOVAT = TRUE, POSLEDNY_VNUTRI = FALSE;
int count = 0;



pthread_mutex_t mutexVedra = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexSynchronizacia = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condVsetci = PTHREAD_COND_INITIALIZER;
pthread_cond_t condVnutri = PTHREAD_COND_INITIALIZER;

// maliar - malovanie steny
void maluj(void) {
    sleep(2);
}

//  maliar - branie farby
void ber(int maliar) {

    printf("Tu je maliar %d.  a VEDRO: %d \n", maliar, vedra[maliar]);

    // Po stvrtom vedre
    if(vedra[maliar] % 4 == 0 && vedra[maliar] > 0){

        // Zaspi ak sa neda oddychovat ak je koniec ta skonci
        pthread_mutex_lock(&mutexSynchronizacia);
        while(!ODDYCHOVAT){
            if(stoj){
                return;
            }
            pthread_cond_wait(&condVsetci, &mutexSynchronizacia);
        }

        count++;
        printf("Tu je maliar %d. a idem oddychovat ako %d \n", maliar, count);

        // Over ci je treti ktory ide oddychovat
        if(count == 3){
            printf("Tu je 3. maliar podme oddychovat \n");
            POSLEDNY_VNUTRI = TRUE;
            ODDYCHOVAT = FALSE;
            pthread_cond_broadcast(&condVnutri);
        } else {
            // Zaspi kym sa neprida aj treti maliar ak je koniec skonci
            while(!POSLEDNY_VNUTRI){
                if(stoj){
                    return;
                }
                pthread_cond_wait(&condVnutri, &mutexSynchronizacia);
            }
        }

        pthread_mutex_unlock(&mutexSynchronizacia);


        pthread_mutex_lock(&mutexSynchronizacia);

        count--;
        printf("Tu je maliar %d. a odchadzam ako %d \n", maliar, 3-count);
        // Over zi uz ide posledny z troch oddychovat
        if(count == 0) {
            printf("Tu je 3. maliar ODCHADZAME \n");
            POSLEDNY_VNUTRI = FALSE;
            ODDYCHOVAT = TRUE;
            pthread_cond_broadcast(&condVsetci);

        } else {
            // Zaspi kym neodide posledny ak je koniec skonci
            while(POSLEDNY_VNUTRI) {
                if(stoj){
                    return;
                }
                pthread_cond_wait(&condVsetci, &mutexSynchronizacia);
            }
        }
        pthread_mutex_unlock(&mutexSynchronizacia);

        sleep(2); //Oddychuje
    }


    sleep(1);
    pthread_mutex_lock(&mutexVedra);
    vedra[maliar]++;
    pocetVedier++;
    pthread_mutex_unlock(&mutexVedra);
}

// maliar
void *maliar( void *ptr ) {

    int maliarov_index = (int) ptr;


    // pokial nie je zastaveny
    while(!stoj) {
        maluj();
        ber(maliarov_index);
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t maliari[POCET_MALIAROV];

    for (i=0;i<POCET_MALIAROV;i++) pthread_create(&maliari[i], NULL, &maliar, (void *)i);

    sleep(30);


    // Zobud vsetkych maliarov - koncime
    pthread_mutex_lock(&mutexSynchronizacia);
    stoj = 1;
    pthread_cond_broadcast(&condVsetci);
    pthread_cond_broadcast(&condVnutri);
    pthread_mutex_unlock(&mutexSynchronizacia);

    for (i=0;i<10;i++) pthread_join(maliari[i], NULL);

    printf("Celkovy pocet: %d\n", pocetVedier);
    for (i=0;i<10;i++) printf("Maliar %d spravil %d vedier\n", i, vedra[i]);

    exit(EXIT_SUCCESS);
}
