/*
Meno:
Datum:
Simulujte nasledujucu situaciu. Styria murari omietaju miestnosti, pracuju po dvojiciach. Vzdy dvaja murari nahadzuju omietky (v simulacii 2s) a sucane druhi dvaja ich zarovnavaju (v simulacii 3s); a potom sa dvojice vymenia, a prvi dvaja zarovnavaju (3s) a druhi dvaja nahadzuju (2s). Cela simulacia nech trva 30s.
1. Doplnte do programu vypis na obrazovku v ktorom bude cislo robotnika 1 az N, cinnost, a ci cinnost zacina alebo konci, napr. "robotnik c.2 hadz zaciatok". Nakoniec do programu doplnte vypis, kolko vypisov na brazovku bolo vykonanych. [3b]
2. Zabezpecte, aby sa dvojice murarov spravne striedali (a zacali so spravnou cinnostou), cize naraz zacinaju dvaja nahadzovat a zaroven dvaja zarovnavat, potom sa pockaju a vystriedaju. [4b]
3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby murar po stanovenom case uz nezacal dalsiu cinnost. [2b]
Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc murari.c -o murari -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define TRUE 1
#define FALSE 0
#define HADZE 0
#define ZAROVNAVA 1
#define POCET_MURAROV 4

// signal na zastavenie simulacie
int stoj = 0;

// Cykly

int zameranie[POCET_MURAROV] = {HADZE, HADZE, ZAROVNAVA, ZAROVNAVA};
int pocetVypisov[4] = {0};
int cakaju = 0;
int vsetciCakaju = FALSE;



// Synchronizacne cykly
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condCakaju = PTHREAD_COND_INITIALIZER;



// murar
void hadz(int murar) {

    pthread_mutex_lock(&mutex);

    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }
    pthread_mutex_unlock(&mutex);

    // Pracuje iba murar so zameranim na HADZANIE
    if(zameranie[murar] == HADZE){

        pthread_mutex_lock(&mutex);
        printf("MURAR %d: zacinam HADZAT\n", murar);
        pocetVypisov[murar]++;
        pthread_mutex_unlock(&mutex);

        // hadzu
        sleep(2);

        pthread_mutex_lock(&mutex);
        printf("MURAR %d: skoncil somm HADZANIE\n", murar);
        pocetVypisov[murar]++;
        pthread_mutex_unlock(&mutex);
    }


}

void zarovnavaj(int murar) {

    pthread_mutex_lock(&mutex);

    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }
    pthread_mutex_unlock(&mutex);

    // Ak je zameranie ZAROVNAVANIE
    if(zameranie[murar] == ZAROVNAVA){

        pthread_mutex_lock(&mutex);
        printf("MURAR %d: zacinam ZAROVNAVAT\n", murar);
        pocetVypisov[murar]++;
        pthread_mutex_unlock(&mutex);

        // hadzu
        sleep(2);

        pthread_mutex_lock(&mutex);
        printf("MURAR %d: skoncil somm ZAROVNAVANIE\n", murar);
        pocetVypisov[murar]++;
        pthread_mutex_unlock(&mutex);
    }


}

void *murar(void *ptr) {

    int murar = (int) ptr;
    int i;

    while(!stoj) {
        hadz(murar);
        zarovnavaj(murar);

        // Pockaju sa a vymenia
        pthread_mutex_lock(&mutex);
        cakaju++;
        if(cakaju == 4){
            // Zmen zameranie
            for(i=0; i<POCET_MURAROV; i++){
                // Prepni zameranie
                zameranie[i] = zameranie[i] == HADZE ? ZAROVNAVA : HADZE;
            }
            vsetciCakaju = TRUE;
            printf("Novy cyklus !!!\n");
            pthread_cond_broadcast(&condCakaju);

        } else {
            while(!vsetciCakaju){
                if(stoj){
                    pthread_mutex_unlock(&mutex);
                    return NULL;
                }
                pthread_cond_wait(&condCakaju, &mutex);
            }
        }
        cakaju--;
        if(cakaju == 0){
            vsetciCakaju = FALSE;
        }
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(void) {
    int i;

    pthread_t murari[POCET_MURAROV];

    for (i=0;i<POCET_MURAROV;i++) pthread_create(&murari[i], NULL, &murar, (void *) i);

    sleep(27);

    pthread_mutex_lock(&mutex);
    stoj = 1;
    printf("Koniec Simulacie !!!!\n");

    pthread_cond_broadcast(&condCakaju);
    pthread_mutex_unlock(&mutex);


    for (i=0;i<POCET_MURAROV;i++) pthread_join(murari[i], NULL);

    for (i=0;i<POCET_MURAROV;i++) printf("Murar %d vypisal: %dkrat\n", i, pocetVypisov[i]);

    exit(EXIT_SUCCESS);
}