#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>

volatile int runnig = 1;
int *ids;
int *eat;
int *think;
int *tried;
int numthr;

pthread_mutex_t *mutex;

void eating(void *id) {
    int threadId = *(int *)id;
    int previousFork = threadId - 1;
    if (previousFork < 0)
        previousFork = numthr - 1;
    
    eat[threadId] += 1;
    usleep(2000); // 2 microsegundos
}

void thinking(void *id) {
    int threadId = *(int *)id;
    think[threadId] += 1;
    sleep(5); // 5 segundos
}

void tryEating(void *id) {
    int threadId = *(int *)id;
    tried[threadId] += 1;
    int r = rand() % 4; // numero aleatorio entre 0 e 3
    sleep(r); // entre 0 e 3 segundos
}

void *dinner(void *id) {
    int threadId = *(int *)id;

    while (runnig) {

        // tenta lockar um talher
        if (pthread_mutex_trylock(&mutex[threadId]) == 0) {
            int previousFork = threadId - 1;

            // problema é modelado em uma mesa circular.
            if (previousFork < 0)
                previousFork = numthr;

            // tenta lockar outro talher
            if (pthread_mutex_trylock(&mutex[previousFork]) == 0) {
                eating(id);

                // apos comer libera os 2 talheres
                pthread_mutex_unlock(&mutex[threadId]);
                pthread_mutex_unlock(&mutex[previousFork]);

                // pensa
                thinking(id);
            }
            // se nao conseguir, libera talher
            else {
                pthread_mutex_unlock(&mutex[threadId]);
                tryEating(id);
            }
        }
        else 
            tryEating(id);    
    }
}

int main(int argc, char **argv) {
    pthread_t *philosopher;
    int i, runningTime;

    numthr = atoi(argv[1]); // filosofos
    runningTime = atoi(argv[2]); // tempo de execucao

    ids = (int *)malloc(numthr * sizeof(int));
    eat = (int *)malloc(numthr * sizeof(int));
    think = (int *)malloc(numthr * sizeof(int));
    tried = (int *)malloc(numthr * sizeof(int));

    // inicializa controladores
    for (i = 0; i < numthr; i++) {
        ids[i] = i;
        eat[i] = tried[i] = think[i] = 0;
    }

    philosopher = (pthread_t *)malloc(numthr * sizeof(pthread_t));
    mutex = (pthread_mutex_t *)malloc(numthr * sizeof(pthread_mutex_t));

    // um mutex para cada garfo
    for (i = 0; i < numthr; i++) 
        pthread_mutex_init(&mutex[i], NULL);
    

    // cria uma thread p/ cada filosofo
    for (i = 0; i < numthr; i++) {
        if (pthread_create(&philosopher[i], NULL, dinner, (void *)&ids[i])) {
            fprintf(stderr, "Error creating thread\n");
            return 2;
        }
    }

    sleep(runningTime);
    runnig = 0;

    // relatorio do jantar.
    for (i = 0; i < numthr; i++) {
        printf("Filosofo %d: \n pensou %d \n comeu %d \n tentou %d \n",
               i, think[i], eat[i], tried[i]);
    }

    return 0;
}
