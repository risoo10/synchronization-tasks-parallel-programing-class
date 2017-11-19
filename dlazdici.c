/*
Meno:
Datum:
Simulujte nasledujucu situaciu. Styria dlazdici kladu dlazbu a dvaja pomocnici im pripravuju dlazdice. Jeden pomocnik pravuje velke dlazdice (kazdu za nejaky cas, v simulacii 2s) a druhy pomocnik pripravuje male dlazdice (kazdu, za nejaky cas, v simulacii 1s) a davaju dlazdice na kopu, kde sa zmesti 10 dlazdic. Dlazdic velku dlazdicu kladie nejaky cas (v simulacii 3s) a malu dlazdicu tiez nejaky cas (2s), dlazdice si berie z kopy. Cela simulacia nech trva 30s.

1. Doplnte do programu vypis na obrazovku v ktorom bude cislo robotnika 1 az N, cinnost, a ci cinnost zacina alebo konci, napr. "robotnik c.2 uloz zaciatok". Doplnte do programu pocitadla pokladenych malych a velkych dlazdic, na konci simulaci vypiste hodnoty pocitadiel. [3b]

2. Zabezpecte, aby na kopu bolo ulozene maximalne 10 dlazdic a zabezpecte spravny pristup ku kope dlazdic. Kopu simulujte vhodne zvolenou udajovou strukturou. [4b]
3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby murar po stanovenom case uz nezacal dalsiu cinnost. [2b]
Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc dlazdici.c -o dlazdici -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define VELKA 3
#define MALA 2
#define KOPA_LIMIT 10
#define POCET_DLAZDICOV 4

typedef struct dlazdica{
    int velkost;
} Dlazdica;

// Kopa
int pocetDlazdic = 0;
int pozicia_na_umiestnenie = 0;
int pozicia_na_zobratie = 0;
Dlazdica *kopa[KOPA_LIMIT];


int ulozeneDlazdiceMale = 0, ulozeneDlazdiceVelke = 0;

// Mutexy a Condy
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPomocnici = PTHREAD_COND_INITIALIZER;
pthread_cond_t condDlazdici = PTHREAD_COND_INITIALIZER;


// signal na zastavenie simulacie
int stoj = 0;

// dlazdic
void uloz(int dlazdic) {

    pthread_mutex_lock(&mutex);

    printf("Robotnik %d zaciatok UKLADANIA \n", dlazdic);
    while(pocetDlazdic == 0){
        pthread_cond_wait(&condDlazdici, &mutex);
    }

    // Zober dlazdicu
    Dlazdica *dlazdica = kopa[pozicia_na_zobratie];
    int casSpania = dlazdica->velkost;
    // Posun hlavicu
    pozicia_na_zobratie = (pozicia_na_zobratie + 1) % KOPA_LIMIT;
    pocetDlazdic--;

    printf("Robotnik %d spracoval som DLAZDICU [%d] \n", dlazdic, dlazdica->velkost);
    pthread_cond_signal(&condPomocnici);
    pthread_mutex_unlock(&mutex);

    // Pracuj s dlazdicou
    sleep((unsigned int)casSpania);

    // Uvolni miesto
    printf("Robotnik %d ukoncil som UKLADANIE dlazdice [%d]\n", dlazdic, dlazdica->velkost);
    free(dlazdica);
}

void *dlazdic(void *ptr) {

    int dlazdic = (int) ptr;

    while(!stoj) {
        uloz(dlazdic);
    }
    return NULL;
}

// pomocnik
void poloz_malu(int pomocnik) {

    // Producer s odmedzenou velkostou BUFFRA
    // Produkuj
    sleep(1);

    pthread_mutex_unlock(&mutex);
    printf("Pomocnik %d idem polozit MALU\n", pomocnik);
    while(pocetDlazdic == KOPA_LIMIT){
        pthread_cond_wait(&condPomocnici, &mutex);
    }
    // Vloz dlazdicu na kopu
    kopa[pozicia_na_umiestnenie] = malloc(sizeof(Dlazdica));
    kopa[pozicia_na_umiestnenie]->velkost = MALA;
    // Posun zapisovaciu polohu
    pozicia_na_umiestnenie = (pozicia_na_umiestnenie + 1) % KOPA_LIMIT;
    pocetDlazdic++;

    // Signalizuj
    pthread_cond_signal(&condDlazdici);
    pthread_mutex_unlock(&mutex);

}

void poloz_velku(int pomocnik) {

    // Producer s odmedzenou velkostou BUFFRA
    // Produkuj

    sleep(2);

    pthread_mutex_unlock(&mutex);
    printf("Pomocnik %d idem polozit VELKU\n", pomocnik);
    while(pocetDlazdic == KOPA_LIMIT){
        pthread_cond_wait(&condPomocnici, &mutex);
    }
    // Vloz dlazdicu na kopu
    kopa[pozicia_na_umiestnenie] = malloc(sizeof(Dlazdica));
    kopa[pozicia_na_umiestnenie]->velkost = VELKA;
    // Posun zapisovaciu polohu
    pozicia_na_umiestnenie = (pozicia_na_umiestnenie + 1) % KOPA_LIMIT;
    pocetDlazdic++;

    // Signalizuj
    pthread_cond_signal(&condDlazdici);
    pthread_mutex_unlock(&mutex);

}

void *pomocnik1(void *ptr) {

    int pomocnik = (int) ptr;

    while(!stoj) {
        poloz_malu(pomocnik);
    }
    return NULL;
}

void *pomocnik2(void *ptr) {

    int pomocnik = (int) ptr;

    while(!stoj) {
        poloz_velku(pomocnik);
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t robotnici[POCET_DLAZDICOV + 2];

    for (i=0;i<POCET_DLAZDICOV;i++) pthread_create(&robotnici[i], NULL, &dlazdic, (void *)i);
    pthread_create(&robotnici[4], NULL, &pomocnik1, (void *)i);
    pthread_create(&robotnici[5], NULL, &pomocnik2, (void *)i);

    sleep(30);
    stoj = 1;

    for (i=0;i<POCET_DLAZDICOV + 2;i++) pthread_join(robotnici[i], NULL);

    printf("Pocet dlazdic: VELKE %d | MALE %d\n", pocetDlazdic);

    exit(EXIT_SUCCESS);
}