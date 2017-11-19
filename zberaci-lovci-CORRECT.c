/*
Meno:
Datum:

Simulujte nasledujucu situaciu. V kmeni su dve kasty: lovci (6 lovcov) a zberaci (12 zberacov). Uctievaju bozstvo, ktoremu chodia davat dary do chramu. Lovec lovi zver nejaky cas (v simulacii 6s) a potom ide do chramu dat cast ulovku ako dar bozstvu, co tiez trva nejaky cas (v simulacii 2s). Zberac zbiera plody nejaky cas (v simulacii 4s) a potom ide do chramu dat cast plodov bozstvu, co tiez trva nejaky cas (v simulacii 1s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo pocitajuce, kolko krat bozstvu dali dar lovci a kolko krat zberaci. [2b]

2. Zabezpecte, aby do chramu sucasne mohli vojst maximalne dvaja lovci alebo styria zberaci, iba prislusnici jednej kasty naraz. Ak je pred chramom rad, zabezpecte spravodlivy pristup (kasty su si rovnocenne). [5b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc lovci_a_zberaci.c -o lovci_a_zberaci -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0


// signal na zastavenie simulacie
int stoj = 0, daryLovci = 0, daryZberaci = 0, ludiaVChrame = 0;
int VOLNY_CHRAM = TRUE, ZBERACI_DNU = FALSE, LOVCI_DNU = FALSE;

// Stnchronizacne konstrukcie
pthread_mutex_t  mutexSynchronizacia = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condChram = PTHREAD_COND_INITIALIZER;
pthread_cond_t condLovci = PTHREAD_COND_INITIALIZER;
pthread_cond_t condZberaci = PTHREAD_COND_INITIALIZER;



// lovec
void lov(void) {
    sleep(6);
}

void dar_lov(void) {

    // Zisti ci neskoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutexSynchronizacia);
        return ;
    }

    // Vzajomne vyluc vstup do chramu
    pthread_mutex_lock(&mutexSynchronizacia);

    // Uspi ak su v chrame ZBERACI alebo JE V CHRAME vela lovcov
    while(ZBERACI_DNU || (LOVCI_DNU && ludiaVChrame >= 2)){
        // Zisti ci neskoncila simulacia
        if(stoj){
            pthread_mutex_unlock(&mutexSynchronizacia);
            return ;
        }
        pthread_cond_wait(&condChram, &mutexSynchronizacia);
    }

    ludiaVChrame++;

    // Prvy v chrame nastavi ze su v chrame lovci
    if(ludiaVChrame == 1){
        printf("LOVCI vosli do chramu \n");
        LOVCI_DNU = TRUE;
        while(LOVCI_DNU && ludiaVChrame < 2){
            // Zisti ci neskoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutexSynchronizacia);
                return ;
            }
            pthread_cond_wait(&condLovci, &mutexSynchronizacia);
        }

    // Druhy lovec v chrame zobudi cakajuceho a idu dat dar
    } else if(ludiaVChrame == 2){
        pthread_cond_broadcast(&condLovci);
    }

    // Uvolni vylucovanie
    pthread_mutex_unlock(&mutexSynchronizacia);

    // Darovanie
    sleep(2);

    pthread_mutex_lock(&mutexSynchronizacia);

    // Uvolni miesto v chrame
    ludiaVChrame--;

    // Posledny umozni vstup do chramu pre ostatnych
    if(ludiaVChrame == 0){

        // Zapocitaj dar
        daryLovci++;

        printf("LOVCI vysli von | darovali: %d krat\n", daryLovci);
        LOVCI_DNU = FALSE;
        pthread_cond_broadcast(&condChram);
    }

    pthread_mutex_unlock(&mutexSynchronizacia);

}

void *lovec( void *ptr ) {

    while(!stoj) {
        lov();
        dar_lov();
    }
    return NULL;
}

// zberac
void zber(void) {
    sleep(4);
}

void dar_zber(void) {


    // Zisti ci neskoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutexSynchronizacia);
        return ;
    }

    // Vzajomne vyluc vstup do chramu
    pthread_mutex_lock(&mutexSynchronizacia);

    // Uspi ak su v chrame LOVCI alebo JE V CHRAME vela ZBERACOV
    while(LOVCI_DNU || (ZBERACI_DNU && ludiaVChrame >= 4)){
        // Zisti ci neskoncila simulacia
        if(stoj){
            pthread_mutex_unlock(&mutexSynchronizacia);
            return ;
        }
        pthread_cond_wait(&condChram, &mutexSynchronizacia);
    }

    ludiaVChrame++;

    // Prvy v chrame nastavi ze su v chrame ZBERACI
    if(ludiaVChrame == 1){
        ZBERACI_DNU = TRUE;
        printf("ZBERACI vosli do chramu\n");
        while(ZBERACI_DNU && ludiaVChrame < 4){
            // Zisti ci neskoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutexSynchronizacia);
                return ;
            }
            pthread_cond_wait(&condZberaci, &mutexSynchronizacia);
        }

    // POsledny zberac pusti vsetkych cakajucich zberacov do chramu
    } else if(ludiaVChrame == 4){
        pthread_cond_broadcast(&condZberaci);
    }

    // Uvolni vylucovanie
    pthread_mutex_unlock(&mutexSynchronizacia);

    // Darovanie
    sleep(1);

    pthread_mutex_lock(&mutexSynchronizacia);

    // Uvolni miesto v chrame
    ludiaVChrame--;


    // Posledny umozni vstup do chramu pre ostatnych
    if(ludiaVChrame == 0){

        // Zapocitaj dar
        daryZberaci++;

        ZBERACI_DNU = FALSE;
        printf("ZBERACI vysli von | darovali: %d krat\n", daryZberaci);
        pthread_cond_broadcast(&condChram);
    }

    pthread_mutex_unlock(&mutexSynchronizacia);

}

void *zberac( void *ptr ) {

    // pokial nie je zastaveny
    while(!stoj) {
        zber();
        dar_zber();
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t lovci[6];
    pthread_t zberaci[12];

    for (i=0;i<6;i++) pthread_create( &lovci[i], NULL, &lovec, NULL);
    for (i=0;i<12;i++) pthread_create( &zberaci[i], NULL, &zberac, NULL);

    sleep(30);

    pthread_mutex_lock(&mutexSynchronizacia);
    printf("Koniec Simulacie !!!\n");
    stoj = 1;
    pthread_cond_broadcast(&condChram);
    pthread_cond_broadcast(&condLovci);
    pthread_cond_broadcast(&condZberaci);
    pthread_mutex_unlock(&mutexSynchronizacia);


    for (i=0;i<6;i++) pthread_join( lovci[i], NULL);
    for (i=0;i<12;i++) pthread_join( zberaci[i], NULL);

    printf("LOVCI - Darovali: %d krat\n", daryLovci);
    printf("ZBERACI - Darovali: %d krat\n", daryZberaci);

    exit(EXIT_SUCCESS);
}
