/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Styria generovaci pismenok generuju pismenka, generovanie pismenka trva nejaky cas (1s) a ked ho vygeneruju, umiestnia ho na stol, kde sa zmesti 10 pismenok. Desiati testovaci beru pismenka zo stola a testuju ich, testovanie pismenka trva nejaky cas (2s). Cela simulacia nech trva 30s.

1. Doplnte do programu pocitadlo vygenerovanych a pocitadlo otestovanych pismenok, na konci simulacie vypiste hodnoty pocitadiel. [2b]

2. Osetrite v programe pristup k stolu - zmente umiestnovanie a branie pismenok tak, aby nehrozilo, ze generovac "prepise" pismenko, ktore nebolo otestovane, a ze testovac otestuje pismenko, ktore nebolo vygenerovane alebo uz bolo otestovane. [5b]

3. Osetrite v programe spravne ukoncenie generovacov a testovacov po uplynuti stanoveneho casu simulacie. [3b]

Poznamky:
- na synchronizaciu pouzite iba mutexy, podmienene premenne alebo semafory
- build (console): gcc generator_pismenok -o generator_pismenok -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#define TRUE 1
#define FALSE 0

// stol s pismenkami
char stol[10] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
int pozicia_na_umiestnenie = 0;
int pozicia_na_zobratie = 0;

// signal na zastavenie
int stoj = 0;
int pocetVygenerovanych = 0, pocetTestovanych = 0;





// Synchronizacne konstrukcie
pthread_mutex_t mutexSynchronizacia = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPlny = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPrazdny = PTHREAD_COND_INITIALIZER;


int jePlnyStol(){
    if(pocetVygenerovanych - pocetTestovanych == 10){
        return TRUE;
    }

    return FALSE;
}

int jePrazdnyStol(){
    if(pocetVygenerovanych == pocetTestovanych){
        return TRUE;
    }

    return FALSE;
}


// generovanie pismenka
char generuj_pismenko(void)
{
    sleep(1);
    return 'A';
}

// testovanie pismenka
void testuj_pismenko(char pismenko)
{
    sleep(2);
}

// generator pismenok
void *generovac_pismenok( void *ptr ) {

    // pokial nie je zastaveny
    while(!stoj) {

        // vygenerovanie pismenka
        char pismenko = generuj_pismenko();

        // Vzajomne vylucovanie
        pthread_mutex_lock(&mutexSynchronizacia);

        // Nezapisuj ak je plny stol
        while(jePlnyStol()){

            // Ukonci ak skoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutexSynchronizacia);
                return ptr;
            }

            printf("G: Plny stol ... cakam\n");
            pthread_cond_wait(&condPlny, &mutexSynchronizacia);
        }


        // umiestni pismenko na stol
        stol[pozicia_na_umiestnenie] = pismenko;

        // Zvys pocet vyenerovanych
        pocetVygenerovanych++;

        printf("G: Vygeneroval som pismenko | rozdiel: %d\n", pocetVygenerovanych - pocetTestovanych);

        // Posun zapisovanie
        pozicia_na_umiestnenie = (pozicia_na_umiestnenie + 1) % 10;

        // Signalizuj ze nie je prazdny
        pthread_cond_signal(&condPrazdny);

        // Uvolni pristup
        pthread_mutex_unlock(&mutexSynchronizacia);

    }
    return NULL;
}

// testovac pismenok
void *testovac_pismenok( void *ptr ) {

    // pokial nie je zastaveny
    while(!stoj) {

        // Vzajomne vylucovanie
        pthread_mutex_lock(&mutexSynchronizacia);

        // Zisti ci nie je prazdny stol a zaspi ak je prazdny
        while(jePrazdnyStol()){

            // Ukonci ak skoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutexSynchronizacia);
                return ptr;
            }

            printf("T: Prazdny stol ... cakam\n");

            pthread_cond_wait(&condPrazdny, &mutexSynchronizacia);
        }

        // vzatie pismenka zo stola
        char pismenko = stol[pozicia_na_zobratie];

        // Aktualizuj pocet otestovanych pismen
        pocetTestovanych++;

        printf("T: Otestoval som pismenko | rozdiel: %d\n", pocetVygenerovanych - pocetTestovanych);

        pozicia_na_zobratie = (pozicia_na_zobratie + 1) % 10;

        // Signalizuj ze nie je plny stol
        pthread_cond_signal(&condPlny);

        // Uvolni pristup
        pthread_mutex_unlock(&mutexSynchronizacia);

        // otestovanie pismenka
        testuj_pismenko(pismenko);
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t generovaci[4];
    pthread_t testovaci[10];

    for (i=0;i<4;i++) pthread_create( &generovaci[i], NULL, &generovac_pismenok, NULL);
    for (i=0;i<10;i++) pthread_create( &testovaci[i], NULL, &testovac_pismenok, NULL);

    sleep(30);

    printf("Koniec simulacie !!!\n");

    // Ukonci pracu generatorov a testerov
    pthread_mutex_lock(&mutexSynchronizacia);

    stoj = 1;

    // Prebud uspatych generatorov a testerov
    pthread_cond_broadcast(&condPlny);
    pthread_cond_broadcast(&condPrazdny);

    pthread_mutex_unlock(&mutexSynchronizacia);

    for (i=0;i<4;i++) pthread_join( generovaci[i], NULL);
    for (i=0;i<10;i++) pthread_join( testovaci[i], NULL);

    printf("Pocet vygenerovanych pismen: %d\n", pocetVygenerovanych);
    printf("Pocet otestovanych pismen: %d\n", pocetTestovanych);


    exit(EXIT_SUCCESS);
}
