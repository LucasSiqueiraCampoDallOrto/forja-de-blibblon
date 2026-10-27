#ifndef TP_AVENTUREIRO_H
#define TP_AVENTUREIRO_H

#include "texto.h"
#include "pilha.h"
#include "catalogo.h"

class Aventureiro {
private:
    // mantém um contador compartilhado para numerar os aventureiros cadastrados
    static int contador;
public:
    int id_aventureiro;
    Texto nome_aventureiro;
    Pilha equipamentos; // cada aventureiro tem sua pilha de equipamentos
    Aventureiro(Texto nome);
    long long* calcular_build(Matriz_modificadores& matriz, int total_atributos);
    // cada aventureiro tem sua própria build que calcula o total de cada atributo ao somar os modificadores dos itens equipados
};

#endif
