
/*
Meno: Richard Mocak
Datum: 8/11/2017
 
 Simulujte nasledujucu situaciu. Vo firme pracuje 7 programatorov a 5 testerov (3 pomali a 2 rychli). Programovanie trva programatorovi nejaky cas (v simululacii 3s) a potom si da prestavku (v simulacii 2s); a potom zase ide programator programovat. Tester nejaky cas testuje (pomaly 3s a rychly 2s) a potom si tiez dava prestavku (3s); po skonceni ktorej ide zase testovat. Cela simulacia nech trva 30s.
 
1. Doplnte do programu pocitadlo pocitajuce, kolko krat bolo pocas simulacie vykonavane testovanie a kolko krat programovanie. [1b]
 
2. Zabezpecte, aby na programe nepracovali testeri a programatori naraz, cize na programe mozu pracovat alebo iba programatori alebo iba testeri. Ani jedna skupina nema prioritu. [4b]
 
3. Zabezpecte, aby si testeri davali prestavku po skupinach (aby sa pockali v ramci skupiny), vzdy pomali ako jedna skupina a rychli ako druha skupina. [4b]
 
4. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu. [1b]
 
Poznamky:
- na synchronizaciu pouzite iba mutexy a podmienene premenne
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi
- build (console): gcc programatori_a_testeri_2.c -o programatori_a_testeri_2 -lpthread
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>


#define TRUE 1
#define FALSE 0

#define POMALI 3
#define RYCHLI 2
#define PROGRAMATORI 7


// signal na zastavenie simulacie
int stoj = 0;

int programuje = 0, testuje = 0;
int pocetTestovani = 0, pocetProgramovani = 0;
int cakajuPomali = 0, cakajuRychli = 0;
int vsetciPomali = FALSE, vsetciRychli = FALSE;

// Mutexy a Condy
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexRychli = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexPomali = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condPraca = PTHREAD_COND_INITIALIZER;
pthread_cond_t condRychly = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPomali = PTHREAD_COND_INITIALIZER;



// programovanie
void programovanie() {

    // Writer thread (readers-writers problem)
    pthread_mutex_lock(&mutex);
    while(testuje > 0){
        // Ukonci ak skoncila simulacia
        if(stoj){
            pthread_mutex_unlock(&mutex);
            return;
        }
        pthread_cond_wait(&condPraca, &mutex);
    }

    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }

    programuje++;
    printf("P: idem programovat | ako %d\n", programuje);
    pthread_mutex_unlock(&mutex);

    // Programuje
    sleep(3);

    pthread_mutex_lock(&mutex);
    pocetProgramovani++;
    programuje--;

    printf("P: koniec programovania | celkovo %d\n", pocetProgramovani);

    if(programuje == 0){
        pthread_cond_broadcast(&condPraca);
    }

    pthread_mutex_unlock(&mutex);
}

// prestavka programatora
void prestavka_programator() {

    pthread_mutex_lock(&mutex);
    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }
    pthread_mutex_unlock(&mutex);

    // prestavka
    sleep(2);
}

// testovanie
void testovanie_pomaly() {

    // Reader thread (readers-writers problem)
    pthread_mutex_lock(&mutex);
    while(programuje > 0){
        // Ukonci ak skoncila simulacia
        if(stoj){
            pthread_mutex_unlock(&mutex);
            return;
        }
        pthread_cond_wait(&condPraca, &mutex);
    }

    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }

    testuje++;
    printf("T: idem testovat | ako %d\n", testuje);
    pthread_mutex_unlock(&mutex);

    // Testuje pomaly
    sleep(3);

    // Skoncil testovanie
    pthread_mutex_lock(&mutex);
    testuje--;
    pocetTestovani++;
    printf("T: koniec testovania | celkovo %d\n", pocetTestovani);
    if(testuje == 0){
        pthread_cond_broadcast(&condPraca);
    }
    pthread_mutex_unlock(&mutex);


    // Pocka na kolegov - ZNOVUPOUZITELNA BARIERA
    pthread_mutex_lock(&mutexPomali);
    cakajuPomali++;
    printf("T-POMALY: cakam na vsetkych | ako %d\n", cakajuPomali);
    if(cakajuPomali == POMALI){ // Vsetci dnu
        vsetciPomali = TRUE;
        pthread_cond_broadcast(&condPomali);
    } else {
        while(!vsetciPomali){
            // Ukonci ak skoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutexPomali);
                return;
            }
            pthread_cond_wait(&condPomali, &mutexPomali);
        }
    }

    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutexPomali);
        return;
    }

    cakajuPomali--;
    if(cakajuPomali == 0){ // Vsetci von
        vsetciPomali = FALSE;
        printf("T-POMALY: ideme vsetci na prestavku\n");
    }
    pthread_mutex_unlock(&mutexPomali);

}

void testovanie_rychly() {

    // Reader thread (readers-writers problem)
    pthread_mutex_lock(&mutex);
    while(programuje > 0){
        // Ukonci ak skoncila simulacia
        if(stoj){
            pthread_mutex_unlock(&mutex);
            return;
        }

        pthread_cond_wait(&condPraca, &mutex);
    }

    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }

    testuje++;
    printf("T: idem testovat | ako %d\n", testuje);
    pthread_mutex_unlock(&mutex);

    // Testuje pomaly
    sleep(2);

    // Skoncil testovanie
    pthread_mutex_lock(&mutex);
    testuje--;
    pocetTestovani++;
    printf("T: koniec testovania | celkovo %d\n", pocetTestovani);
    if(testuje == 0){
        pthread_cond_broadcast(&condPraca);
    }
    pthread_mutex_unlock(&mutex);


    // Pocka na kolegov - ZNOVUPOUZITELNA BARIERA
    pthread_mutex_lock(&mutexRychli);
    cakajuRychli++;
    printf("T-RYCHLY: cakam na vsetkych | ako %d\n", cakajuRychli);
    if(cakajuRychli == RYCHLI){ // Vsetci dnu
        vsetciRychli = TRUE;
        pthread_cond_broadcast(&condRychly);
    } else {
        while(!vsetciRychli){
            // Ukonci ak skoncila simulacia
            if(stoj){
                pthread_mutex_unlock(&mutexRychli);
                return;
            }

            pthread_cond_wait(&condRychly, &mutexRychli);
        }
    }

    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutexRychli);
        return;
    }

    cakajuRychli--;
    if(cakajuRychli == 0){ // Vsetci von
        vsetciRychli = FALSE;
        printf("T-RYCHLY: ideme vsetci na prestavku\n");
    }
    pthread_mutex_unlock(&mutexRychli);

    // Idu si davat presstavku
}

// prestavka testera
void prestavka_tester()
{
    pthread_mutex_lock(&mutex);
    // Ukonci ak skoncila simulacia
    if(stoj){
        pthread_mutex_unlock(&mutex);
        return;
    }
    pthread_mutex_unlock(&mutex);

    // Prestavka
    sleep(3);
}

// programator
void* programator(void * ptr) {

    while(!stoj) {
        programovanie();
        prestavka_programator();
    }
    pthread_exit(0);
}

// tester rychly
void* tester_rychly() {

    while(!stoj) {
        testovanie_rychly();
        prestavka_tester();
    }
    pthread_exit(0);
}

// tester pomaly
void* tester_pomaly(void * ptr) {

    while(!stoj) {
        testovanie_pomaly();
        prestavka_tester();
    }
    pthread_exit(0);
}

int main(void) {
    int i;

    pthread_t programatori[PROGRAMATORI];
    pthread_t testeri_rychli[RYCHLI];
    pthread_t testeri_pomali[POMALI];

    for (i = 0; i < PROGRAMATORI; ++i) pthread_create(&programatori[i], NULL, programator, NULL);
    for (i = 0; i < RYCHLI; ++i) pthread_create(&testeri_rychli[i], NULL, tester_rychly, NULL);
    for (i = 0; i < POMALI; ++i) pthread_create(&testeri_pomali[i], NULL, tester_pomaly, NULL);


    sleep(30);

    pthread_mutex_lock(&mutex);
    pthread_mutex_lock(&mutexPomali);
    pthread_mutex_lock(&mutexRychli);
    // Konniec simulacie
    stoj = 1;
    printf("Koniec simulacie !!! \n");
    // Zobud vsetkych uspatych
    pthread_cond_broadcast(&condPraca);
    pthread_cond_broadcast(&condRychly);
    pthread_cond_broadcast(&condPomali);

    pthread_mutex_unlock(&mutexRychli);
    pthread_mutex_unlock(&mutexPomali);
    pthread_mutex_unlock(&mutex);


    for (i = 0; i < PROGRAMATORI; ++i) pthread_join(programatori[i], NULL);
    for (i = 0; i < RYCHLI; ++i) pthread_join(testeri_rychli[i], NULL);
    for (i = 0; i < POMALI; ++i) pthread_join(testeri_pomali[i], NULL);

    printf("Pocet testovani: %d \n", pocetTestovani);
    printf("Pocet programovani: %d \n", pocetProgramovani);


    exit(EXIT_SUCCESS);
}