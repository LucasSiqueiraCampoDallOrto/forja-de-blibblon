#ifndef TP_PERCURSO_H
#define TP_PERCURSO_H

#include "instrumentacao.h"
#include "tipos_catalogo.h"
#include "comprimida_linhas.h"


class Percurso { // permite ler as células de uma linha ou coluna, uma por vez
public:

    // aponta para os dados do modo C
    const ComprimidaLinhas* comprimida = nullptr;

    // aponta para os contadores usados na análise de desempenho
    EstatisticasCatalogo* estatisticas = nullptr;

    // indica o modo de armazenamento usado no percurso: L, T ou C
    char modo = 'L';

    // se for true percorre uma coluna e se for false percorre uma linha
    bool por_coluna = false;

    // guarda o id do item ao percorrer uma linha ou do atributo ao percorrer uma coluna
    int id = 0;

    // aponta para a próxima célula que será lida no modo L
    const Celula* atual = nullptr;

    // aponta para o vetor de tripletas que será lido no modo T
    const Tripleta* vetor = nullptr;

    // indica a posição de leitura nos vetores ou a linha examinada no modo C por coluna
    int posicao = 0;

    // indica o limite da leitura: essa posição não faz parte do percurso
    int fim = 0;

    // coloca a próxima célula em resultado e avança ou retorna false quando não há mais células
    bool proximo(Tripleta& resultado);
};

#endif