/*
Meno:
Datum:

Simulujte nasledujucu situaciu. Organizmy su typu A,B a C a kazdy rastie nejaky cas (v simulacii 1, 2 a 3s) a potom sa rozmnozi; a potom zase dokola (nezanika). Na rozmnozenie su potrebne 2 organizmy a vznikaju 1 alebo 2 nove organizmy ostavajuceho typu (cize B a B splodia dva: A a C; C a B splodia jeden: A). Simulacia trva 10s a na zaciatku je z kazdeho typu vytvorene 1 A, 2 B a 3 C organizmy. Rozmnozenie netrva ziadny cas, cize uvazovat ci sa mozu rozmnozovat subezne nema zmysel.

1. Doplnte do programu premenne pocitajuce organizmy v systeme, pre kazdy typ jedno pocitadlo. Na konci simulacie vypiste pocty organizmov typu A, B a C. [2b]

2. Zabezpecte synchronizaciu tak, aby organizmus, ktory sa chce rozmnozit, pockal dalsi organizmus (ak treba); a aby potom oba organizmy vypisali, s akym typom sa rozmnozili. [6b]

3. Osetrite v programe spravne ukoncenie simulacie po uplynuti stanoveneho casu tak, aby uz ziadna cinnost (rast organizmu) nezacala. [2b]

Poznamky:
- na synchronizaciu pouzite iba mutexy+podmienene premenne; resp monitory
- nespoliehajte sa na uvedene casy, simulacia by mala fungovat aj s inymi casmi alebo s nahodne generovanymi casmi
- build (console): gcc organizmy.c -o organizmy -lpthread
*/


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define CAS_A 1
#define CAS_B 2
#define CAS_C 3
#define POCET_A 1
#define POCET_B 2
#define POCET_C 3
#define CAS_SIMULACIA 10

#define TRUE 1
#define FALSE 0

// signal na zastavenie simulacie
int stoj = 0;
char rodic1, rodic2;

int pocet = 0;
int organizmySuma[3] = {0};

pthread_t organizmy[1000];

int ROZMNOZOVAT = TRUE;
int PARIT = FALSE;
int rodicia = 0;

// synchronizacne premenne
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condRozmnozovanie = PTHREAD_COND_INITIALIZER;
pthread_cond_t condPartner = PTHREAD_COND_INITIALIZER;


void *organizmus_A();
void *organizmus_B();
void *organizmus_C();

void rozmnoz(char typ) {

    // Ukonci ak uz nastal koniec
    if(stoj){
        return;
    }

    // Zaspi ak sa nemoze rozmnozovat a skonci ak skonci simulacia
    pthread_mutex_lock(&mutex);
    while(!ROZMNOZOVAT){
        if(stoj){
            return;
        }
        pthread_cond_wait(&condRozmnozovanie, &mutex);
    }

    rodicia++;

    // Zapis rodica 1 a Pockaj na partnera a skonci ak skonci simulacii
    if(rodicia == 1){
        rodic1 = typ;
        printf("Organizmus %c: rozmnozujem sa ako rodic 1\n", rodic1);
        while(!PARIT){
            if(stoj){
                return;
            }
            pthread_cond_wait(&condPartner, &mutex);
        }
    } else if(rodicia == 2){
        // Uz su dvaja tak sa zacni parit
        rodic2 = typ;
        printf("Organizmus %c: rozmnozujem sa ako rodic 2\n", rodic2);
        ROZMNOZOVAT = FALSE;
        PARIT = TRUE;
        pthread_cond_signal(&condPartner);
    }


    rodicia--;

    // Ukonci rozmnozovanie
    if(rodicia == 0){

        // Vytvor novych jednincov iba RAZ !!

        // ak ani jeden z rodicov nie je A
        if(rodic1 != 'A' && rodic2 != 'A'){
            printf("Vytvoril sa novy organizmus A.\n");
            pthread_create(&organizmy[pocet], NULL, &organizmus_A, NULL);
            pocet++;
            organizmySuma[0]++;
        }
        // Ak ani jeden z rodicov nie je B
        if(rodic1 != 'B' && rodic2 != 'B'){
            printf("Vytvoril sa novy organizmus B.\n");
            pthread_create(&organizmy[pocet], NULL, &organizmus_B, NULL);
            pocet++;
            organizmySuma[1]++;
        }
        // Ak ani jeden z rodicov nie je C
        if(rodic1 != 'C' && rodic2 != 'C'){
            printf("Vytvoril sa novy organizmus C.\n");
            pthread_create(&organizmy[pocet], NULL, &organizmus_C, NULL);
            pocet++;
            organizmySuma[2]++;
        }

        printf("\n");

        // Povol ostatnym sa rozmnozovat
        PARIT = FALSE;
        ROZMNOZOVAT = TRUE;
        pthread_cond_broadcast(&condRozmnozovanie);
    }

    pthread_mutex_unlock(&mutex);

}

// organizmus A
void *organizmus_A() {
    while (!stoj) {
        sleep(CAS_A); // rast organizmu
        rozmnoz('A'); // rozmnozenie
    }
    return NULL;
}

// organizmus B
void *organizmus_B() {
    while (!stoj) {
        sleep(CAS_B); // rast organizmu
        rozmnoz('B'); // rozmnozenie
    }
    return NULL;
}

// organizmus C
void *organizmus_C() {
    while (!stoj) {
        sleep(CAS_C); // rast organizmu
        rozmnoz('C'); // rozmnozenie
    }
    return NULL;
}

// main f.
int main(void) {
    int i;


    for (i=0; i<POCET_A; i++){
        pthread_create(&organizmy[pocet], NULL, &organizmus_A, NULL);
        pocet++;
    }
    organizmySuma[0] = POCET_A;

    for (i=0; i<POCET_B; i++){
        pthread_create(&organizmy[pocet], NULL, &organizmus_B, NULL);
        pocet++;
    }
    organizmySuma[1] = POCET_B;

    for (i=0; i<POCET_C; i++){
        pthread_create(&organizmy[pocet], NULL, &organizmus_C, NULL);
        pocet++;
    }
    organizmySuma[2] = POCET_C;

    sleep(CAS_SIMULACIA);

    // Koniec simulacie
    pthread_mutex_lock(&mutex);
    printf("Koniec simulacie !!!\n");

    stoj = 1;

    pthread_cond_broadcast(&condRozmnozovanie);
    pthread_cond_broadcast(&condPartner);
    pthread_mutex_unlock(&mutex);

    for (i=0; i<pocet; i++) pthread_join(organizmy[i], NULL);

    // Vypis statistiku
    printf("Pocet Organimov: %d\n", pocet);
    printf("Organizmov A: %d\n", organizmySuma[0]);
    printf("Organizmov B: %d\n", organizmySuma[1]);
    printf("Organizmov C: %d\n", organizmySuma[2]);

    exit(EXIT_SUCCESS);
}
