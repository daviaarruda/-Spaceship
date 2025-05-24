#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <wchar.h>
#include <locale.h>
#include "screen.h"
#include "keyboard.h"
#include "timer.h"
#include "time.h"

#define AREA_MIN_X 10
#define AREA_MAX_X 70
#define AREA_MIN_Y 5
#define AREA_MAX_Y 20
#define MAX_OBSTACULOS 5
#define FPS 50


int nivelDificuldade = 1;

typedef struct TopScore {
    char *iniciais;
    int score;
    struct TopScore *prox;
} TopScore;

struct Nave {
    int posicao_x, posicao_y;
    int posicao_x_anterior, posicao_y_anterior;
};

typedef struct {
    float posicao_y;
    int posicao_x;
    int ativo;
} Obstaculo;

float distancia_total = 0;
int pontuacao = 0;
Obstaculo obstaculos[MAX_OBSTACULOS];

// Protótipos
void iniciarJogo(struct Nave *nave);
void desenharNave(struct Nave *nave);
void limparPosicaoAnterior(struct Nave *nave);
void desenharObstaculos(float posicao_y, int posicao_x);
void desenharBorda();
void verificarColisao(struct Nave *nave, int *jogo_encerrado);
void imprimirDistancia(int metros);
float selecionarDificuldade();
void inicializarObstaculos();
void atualizarObstaculos(float velocidade);
void adicionarTopScore(TopScore **lista, char *iniciais, int score);
void salvarTopScores(const char *arquivo, TopScore *lista);
TopScore* carregarTopScores(const char *arquivo);
void exibirTopScores(TopScore *lista);
void liberarLista(TopScore *lista);

int main() {
    setlocale(LC_ALL, "");
    TopScore *listaTopScores = carregarTopScores("topscores.txt");
    char jogar_novamente;

    do {
    
        float velocidade_obstaculos = selecionarDificuldade();
        struct Nave nave;
        int jogo_encerrado = 0;
        int tecla = 0;
        distancia_total = 0;
        pontuacao = 0;

        screenInit(1);
        keyboardInit();
        timerInit(0);
        srand(time(NULL));

        iniciarJogo(&nave);
        inicializarObstaculos();
        screenUpdate();

        unsigned int lastTime = timerGetTicks();
        unsigned int currentTime;

        while (!jogo_encerrado) {
            currentTime = timerGetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            if (keyhit()) {
                tecla = readch();
                if (tecla == 10) break;

                nave.posicao_x_anterior = nave.posicao_x;
                if ((tecla == 'd' || tecla == 'D') && nave.posicao_x < AREA_MAX_X - 1) nave.posicao_x++;
                if ((tecla == 'a' || tecla == 'A') && nave.posicao_x > AREA_MIN_X + 1) nave.posicao_x--;
                limparPosicaoAnterior(&nave);
            }

            distancia_total += 5.0f * deltaTime;
            pontuacao = (int)distancia_total;
            atualizarObstaculos(velocidade_obstaculos * deltaTime * 20);

            screenClear();
            desenharBorda();
            for (int i = 0; i < MAX_OBSTACULOS; i++)
                if (obstaculos[i].ativo)
                    desenharObstaculos(obstaculos[i].posicao_y, obstaculos[i].posicao_x);
            desenharNave(&nave);
            imprimirDistancia(pontuacao);
            verificarColisao(&nave, &jogo_encerrado);
            screenUpdate();

            unsigned int frameTime = timerGetTicks() - currentTime;
            if (frameTime < 1000 / FPS)
                timerDelay((1000 / FPS) - frameTime);
        }

        keyboardDestroy();
        screenDestroy();
        timerDestroy();

        
        printf("Jogo finalizado!\nPontuação final: %d\n", pontuacao);
        char iniciais[4];
        printf("Digite suas iniciais (3 letras): ");
        scanf("%3s", iniciais);
        while ((getchar()) != '\n');

        
        int scoreFinal = pontuacao * nivelDificuldade;
        adicionarTopScore(&listaTopScores, strdup(iniciais), scoreFinal);
        salvarTopScores("topscores.txt", listaTopScores);
        exibirTopScores(listaTopScores);

        printf("(Dificuldade %d aplicado *%d no placar)\n", nivelDificuldade, nivelDificuldade);
        printf("Deseja jogar novamente? (s/n): ");
        scanf(" %c", &jogar_novamente);
        while ((getchar()) != '\n');

    } while (jogar_novamente == 's' || jogar_novamente == 'S');

    liberarLista(listaTopScores);
    return 0;
}

void iniciarJogo(struct Nave *nave) {
    nave->posicao_x = (AREA_MIN_X + AREA_MAX_X) / 2;
    nave->posicao_y = AREA_MAX_Y - 1;
    nave->posicao_x_anterior = nave->posicao_x;
    nave->posicao_y_anterior = nave->posicao_y;
    screenClear();
    desenharBorda();
}

void desenharNave(struct Nave *nave) {
    screenSetColor(CYAN, BLACK);
    screenGotoxy(nave->posicao_x, nave->posicao_y);
    printf("🚀");
}

void limparPosicaoAnterior(struct Nave *nave) {
    screenGotoxy(nave->posicao_x_anterior, nave->posicao_y_anterior);
    printf(" ");
}

void inicializarObstaculos() {
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        obstaculos[i].ativo = 1;
        obstaculos[i].posicao_y = AREA_MIN_Y + 1 + (i * 3);
        obstaculos[i].posicao_x = rand() % (AREA_MAX_X - AREA_MIN_X - 2) + AREA_MIN_X + 1;
    }
}

void atualizarObstaculos(float velocidade) {
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (obstaculos[i].ativo) {
            obstaculos[i].posicao_y += velocidade;
            if (obstaculos[i].posicao_y > AREA_MAX_Y) {
                obstaculos[i].posicao_y = AREA_MIN_Y;
                obstaculos[i].posicao_x = rand() % (AREA_MAX_X - AREA_MIN_X - 2) + AREA_MIN_X + 1;
            }
        }
    }
}

void desenharObstaculos(float posicao_y, int posicao_x) {
    int linha_y = (int)(posicao_y + 0.5f);
    if (linha_y >= AREA_MIN_Y && linha_y <= AREA_MAX_Y) {
        screenSetColor(RED, BLACK);
        screenGotoxy(posicao_x, linha_y);
        printf("💥");
    }
}

void desenharBorda() {
    screenSetColor(YELLOW, BLACK);
    for (int y = AREA_MIN_Y; y <= AREA_MAX_Y; y++) {
        screenGotoxy(AREA_MIN_X, y); printf("🟨");
        screenGotoxy(AREA_MAX_X, y); printf("🟨");
    }
}

void verificarColisao(struct Nave *nave, int *jogo_encerrado) {
    if (nave->posicao_x <= AREA_MIN_X + 1 || nave->posicao_x >= AREA_MAX_X - 1) {
        *jogo_encerrado = 1;
        return;
    }
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (!obstaculos[i].ativo) continue;
        int obs_x = obstaculos[i].posicao_x;
        int obs_y = (int)(obstaculos[i].posicao_y + 0.5f);
        if (nave->posicao_x == obs_x && nave->posicao_y == obs_y) {
            *jogo_encerrado = 1;
            return;
        }
    }
}

void imprimirDistancia(int metros) {
    screenSetColor(WHITE, BLACK);
    screenGotoxy(AREA_MIN_X + 2, AREA_MIN_Y - 1);
    printf("Distância: %d metros", metros);
}

float selecionarDificuldade() {
    int escolha = 0;
    while (escolha < 1 || escolha > 4) {
        screenClear();
        screenSetColor(WHITE, BLACK);
        screenGotoxy(AREA_MIN_X + 15, AREA_MIN_Y - 2);
        printf("Bem-vindo ao Spaceship Escape!");
        screenGotoxy(AREA_MIN_X + 10, AREA_MIN_Y + 1);
        printf("Escolha a dificuldade:");
        screenGotoxy(AREA_MIN_X + 10, AREA_MIN_Y + 3);
        printf("1 - Navegação Tranquila");
        screenGotoxy(AREA_MIN_X + 10, AREA_MIN_Y + 4);
        printf("2 - Acelerando no Espaço");
        screenGotoxy(AREA_MIN_X + 10, AREA_MIN_Y + 5);
        printf("3 - Desafio Estelar");
        screenGotoxy(AREA_MIN_X + 10, AREA_MIN_Y + 6);
        printf("4 - Apocalipse Espacial");
        screenGotoxy(AREA_MIN_X + 10, AREA_MIN_Y + 8);
        printf("Escolha (1-4): ");
        scanf("%d", &escolha);
    }
    nivelDificuldade = escolha;
    switch (escolha) {
        case 1: return 0.3f;
        case 2: return 0.5f;
        case 3: return 0.7f;
        case 4: return 1.0f;
    }
    return 0.5f;
}

void adicionarTopScore(TopScore **lista, char *iniciais, int score) {
    TopScore *novo = malloc(sizeof(TopScore));
    novo->iniciais = iniciais;
    novo->score = score;
    novo->prox = NULL;
    if (*lista == NULL || (*lista)->score < score) {
        novo->prox = *lista;
        *lista = novo;
        return;
    }
    TopScore *atual = *lista;
    while (atual->prox != NULL && atual->prox->score >= score) atual = atual->prox;
    novo->prox = atual->prox;
    atual->prox = novo;
}

void salvarTopScores(const char *arquivo, TopScore *lista) {
    FILE *fp = fopen(arquivo, "w");
    if (fp == NULL) return;
    while (lista != NULL) {
        fprintf(fp, "%s %d\n", lista->iniciais, lista->score);
        lista = lista->prox;
    }
    fclose(fp);
}

TopScore* carregarTopScores(const char *arquivo) {
    FILE *fp = fopen(arquivo, "r");
    if (fp == NULL) return NULL;
    TopScore *lista = NULL;
    char iniciais[4]; int score;
    while (fscanf(fp, "%3s %d", iniciais, &score) == 2) {
        adicionarTopScore(&lista, strdup(iniciais), score);
    }
    fclose(fp);
    return lista;
}

void exibirTopScores(TopScore *lista) {
    printf("\n=== TOP SCORES ===\n");
    int rank = 1;
    while (lista != NULL && rank <= 10) {
        printf("%d. %s - %d\n", rank, lista->iniciais, lista->score);
        lista = lista->prox; rank++;
    }
    printf("==================\n\n");
}

void liberarLista(TopScore *lista) {
    while (lista != NULL) {
        TopScore *prox = lista->prox; free(lista->iniciais); free(lista);
        lista = prox;
    }
}
