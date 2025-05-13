#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "passa_tempo.h"

//declara o tipo tabuleiro
typedef struct {
    pthread_mutex_t mutex; //mutex para exclusão mútua
    pthread_cond_t cond; //variável de condição
    int count; //contador de threads na casa do tabuleiro
    int grupo_atual; //marcação do grupo que está ocupando a casa do tabuleiro
} Tabuleiro;

//declara o tabuleiro globalmente
Tabuleiro **tabuleiro;

//declara o tipo de cada casa do tabuleiro
typedef struct{
    int x; //coordenada eixo x
    int y; //coordenada eixo y
    int tempo; //tempo que a thread deve passar ocupando a casa do tabuleiro
} Coordenada;

//declara o tipo de casa thread
typedef struct{
    pthread_t id; //id da thread no pthreads
    int tid; //id da thread passado na entrada
    int grupo; //grupo da thread
    int n_posicoes; //tamanho do trajeto da thread
    Coordenada *trajeto; //vetor com o trajeto que a thread deve seguir no tabuleiro
} Thread;

//inicializa cada casa do tabuleiro com os dados da entrada, inicializa o mutex e a variável de condição
void init_tab (Tabuleiro **tabuleiro, int N){
    for(int i = 0; i < N; i++){
        for(int j =0; j < N; j++){
            pthread_mutex_init(&tabuleiro[i][j].mutex, NULL);
            pthread_cond_init(&tabuleiro[i][j].cond, NULL);
            tabuleiro[i][j].count = 0;
            tabuleiro[i][j].grupo_atual = -1;
        }
    }
}

//entrada da thread na posição
void entra(int x, int y, int grupo, int tid, int tempo, Tabuleiro **tabuleiro){
    Tabuleiro *pos = &tabuleiro[x][y];
    //bloqueia a seção crítica para apenas a thread que está nela alterar as variáveis
    pthread_mutex_lock(&pos->mutex);
    
    //enquanto threads de outro grupo estão na casa do tabuleiro, a thread quue é de outro grupo e deseja entra tem que esperar a posição ficar vazia
    while(pos->grupo_atual != grupo && pos->count > 0){
        //variável de condição esperando
        pthread_cond_wait(&pos->cond, &pos->mutex);
    }

    //se a casa do tabuleiro está vazia, qualquer thread sempre pode entrar
    // if(pos->grupo_atual == -1){
        //posição atualizada com o grupo que entrou nela
    pos->grupo_atual = grupo;
    // }

    //entrada da thread, incrementando o contador
    pos->count++;

    //libera a seção crítica
    pthread_mutex_unlock(&pos->mutex);
}

//saída da thread da posição
void sai(int x, int y, Tabuleiro **tabuleiro){
    Tabuleiro *pos = &tabuleiro[x][y];

    //bloqueia a seção crítica para apenas a thread que está nela alterar as variáveis
    pthread_mutex_lock(&pos->mutex);

    //saída de threads, decrementando o contador
    pos->count--;
    // printf("%d", pos->count);

    //a casa do tabuleiro fica vazia e a thread que estava esperando pela liberação dela é avisada
    if(pos->count == 0){
        pos->grupo_atual = -1;
        pthread_cond_broadcast(&pos->cond);
    }

    //libera a seção crítica
    pthread_mutex_unlock(&pos->mutex);
}

//execução da thread
void *execucao_thread(void *arg) {
    Thread *t = (Thread *)arg; 
    Coordenada c = t->trajeto[0];

    //a primeira thread é um caso que sempre entra na primeira execução, passa o seu tempo lá e sai
    entra(c.x, c.y, t->grupo, t->tid, c.tempo, tabuleiro);
    passa_tempo(t->tid, c.x, c.y, c.tempo);

    //lógica para o resto do trajeto
    for(int i = 1; i < t->n_posicoes; i++){
        Coordenada prox = t->trajeto[i];
        entra(prox.x, prox.y, t->grupo, t->tid, prox.tempo, tabuleiro); //entra na próxima posição
        sai(c.x, c.y, tabuleiro); //sai da útima posição 
        passa_tempo(t->tid, prox.x, prox.y, prox.tempo); //passa o tempo na posição atual
        c = prox;
    }

    sai(c.x, c.y, tabuleiro);
    //a execução da thread chegou ao fim
    return NULL;
}

int main(){
    int N = 0;
    int n_threads = 0;
    Thread *threads;

    scanf("%d %d", &N, &n_threads);
    //alocando o espaço do tabuleiro
    tabuleiro = malloc(N * sizeof(Tabuleiro*));
    for (int i = 0; i < N; i++) {
        tabuleiro[i] = malloc(N * sizeof(Tabuleiro));
    }
    //iniciando o tabuleiro
    init_tab(tabuleiro, N);

    //alocando o espaço das threads
    threads = malloc(n_threads * sizeof(Thread));
    //pegando as informações de todas as threads
    for(int i = 0; i < n_threads; i++){
        scanf("%d %d %d", &threads[i].tid, &threads[i].grupo, &threads[i].n_posicoes);

        threads[i].trajeto = malloc(threads[i].n_posicoes * sizeof(Coordenada));
        for(int j = 0; j < threads[i].n_posicoes; j++) {
            scanf("%d %d %d", &threads[i].trajeto[j].x, &threads[i].trajeto[j].y, &threads[i].trajeto[j].tempo);
        }
    }
    //iniciando o lançamento das execuções das threads
    for(int i = 0; i < n_threads; i++){
        pthread_create(&threads[i].id, NULL, execucao_thread, &threads[i]);
    }

    //esperando todas as threads finalizarem sua execução
    for (int i = 0; i < n_threads; i++){
        pthread_join(threads[i].id, NULL);
    }

    //liberação de memória
    for(int i = 0; i < n_threads; i++){
        free(threads[i].trajeto);
    }
    free(threads);

    for (int i = 0; i < N; i++){
        free(tabuleiro[i]);
    }
    free(tabuleiro);

    return 0;
}
