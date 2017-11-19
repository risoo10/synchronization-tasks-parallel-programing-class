/*
Meno:
Datum:
Simulujte nasledujcu situaciu. Dvadsat robotnikov pracuje na stavbe. Kazdy robotnik nejaky cas pracuje (v simulacii 3s) a potom sa obcerstvuje v pohostinstve, kde si da jedno pivo (v simulacii 5s).
1. Doplnte do programu premennu pocitajucu celkovy pocet vypitych piv. [2b]
2. Simulujte situacu, ze do pohostinstva sa zmesti iba desať robotnikov, ti co chcu pit a nezmestia sa, musia pockat pred pohostinstvom. [4b]
3. Zabezpecte, aby sa robotnici po obcerstveni vsetci pockali a az potom zacali pracovat. [4b]
Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne alebo semafory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi alebo s nahodne generovanymi casmi
- build (console): gcc robotnici.c -o robotnici -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define TRUE 1
#define FALSE 0

#define POCET_ROBOSOV 20
#define LIMIT_MIEST 10


// signal na zastavenie simulacie
int stoj = 0;
int cakaju = 0, volneMiesta = LIMIT_MIEST;
int vsetciCakaju = FALSE;

int pocetPiv = 0;


// Mutex a COND
pthread_mutex_t mutexPivo = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCakanie = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPivo = PTHREAD_COND_INITIALIZER;
pthread_cond_t condCakaj = PTHREAD_COND_INITIALIZER;


//  robotnik
void robotnik_pracuj(void) {
    sleep(3);
}

void robotnik_pi(void) {


    // Multiplex pre volne miesta v pohostinstve
    pthread_mutex_lock(&mutexPivo);

    while(volneMiesta == 0){
        // Ukonci ak bolo stoj
        if(stoj){
            pthread_mutex_unlock(&mutexPivo);
            return;
        }
        pthread_cond_wait(&condPivo, &mutexPivo);
    }

    // Ukonci ak bolo stoj
    if(stoj){
        pthread_mutex_unlock(&mutexCakanie);
        return;
    }

    volneMiesta--;
    pthread_mutex_unlock(&mutexPivo);

    // Oddychuje na pive
    sleep(5);

    pthread_mutex_lock(&mutexPivo);
    pocetPiv++;
    volneMiesta++;
    printf("ROB: vypil som pivo (celkovo %d)\n", pocetPiv);
    pthread_cond_signal(&condPivo);
    pthread_mutex_unlock(&mutexPivo);



    // Bariera(Znovupouzitelna) pre cakanie na vsetkych kym budu po oddychu
    pthread_mutex_lock(&mutexCakanie);

    cakaju++;
    printf("Cakaju: %d\n", cakaju);
    if(cakaju == POCET_ROBOSOV){
        vsetciCakaju = TRUE;
        printf("Uz cakame vsetci !\n");
        pthread_cond_broadcast(&condCakaj);
    } else {
        while(!vsetciCakaju){
            // Ukonci ak bolo stoj
            if(stoj){
                pthread_mutex_unlock(&mutexCakanie);
                return;
            }
            pthread_cond_wait(&condCakaj, &mutexCakanie);
        }
    }

    // Ukonci ak bolo stoj
    if(stoj){
        pthread_mutex_unlock(&mutexCakanie);
        return;
    }

    cakaju--;
    printf("Cakaju: %d\n", cakaju);
    if(cakaju == 0){
        vsetciCakaju = FALSE;
        printf("Uz odisli vsetci\n");
    }
    pthread_mutex_unlock(&mutexCakanie);

}

void *robotnik(void *ptr) {

    while(!stoj) {
        robotnik_pracuj();
        robotnik_pi();
    }
    return NULL;
}

// main f.
int main(void) {
    int i;

    pthread_t robotnici[POCET_ROBOSOV];

    for (i=0;i<POCET_ROBOSOV;i++) pthread_create(&robotnici[i], NULL, &robotnik, NULL);

    sleep(30);
    pthread_mutex_lock(&mutexPivo);
    pthread_mutex_lock(&mutexCakanie);

    stoj = 1;
    printf("Koniec simulacie !!!\n");
    pthread_cond_broadcast(&condCakaj);
    pthread_cond_broadcast(&condPivo);

    pthread_mutex_unlock(&mutexCakanie);
    pthread_mutex_unlock(&mutexPivo);

    for (i=0;i<POCET_ROBOSOV;i++) pthread_join(robotnici[i], NULL);

    printf("Celkovy pocet vypitych piv: %d\n", pocetPiv);

    exit(EXIT_SUCCESS);
}