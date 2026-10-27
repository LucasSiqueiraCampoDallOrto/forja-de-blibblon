#ifndef ANALISE_PRODUTOS_AUXILIARES_H
#define ANALISE_PRODUTOS_AUXILIARES_H

#include "catalogo.h"

// calcula o produto interno de dois itens usando um vetor auxiliar já alocado
double produto_denso(Matriz_modificadores& m, int i, int j, double* auxiliar, int atributos);

#endif