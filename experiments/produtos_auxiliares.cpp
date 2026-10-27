#include "produtos_auxiliares.h"

// calcula o produto interno de dois itens usando um vetor auxiliar
double produto_denso(Matriz_modificadores& m, int i, int j, double* auxiliar, int atributos) {
    // zera vetor -> guardar modificadores do primeiro item -> multiplicar pelos do segundo -> soma

    // zera o vetor já alocado; essa limpeza faz parte do custo do algoritmo.
    for (int a = 0; a < atributos; a = a + 1) {
        auxiliar[a] = 0;
    }

    Percurso p = m.percorrer_linha(i); // prepara a leitura dos modificadores do primeiro item
    Tripleta c; // recebe uma célula por vez

    // guarda cada modificador na posição correspondente ao seu atributo
    while (p.proximo(c)) {
        auxiliar[c.id_atributo] = c.valor;
    }

    p = m.percorrer_linha(j); // passa a percorrer os modificadores do segundo item
    double soma = 0;

    // multiplica os valores do mesmo atributo e acumula os resultados
    while (p.proximo(c)) {
        soma = soma + auxiliar[c.id_atributo] * c.valor;
    }

    return soma; // devolve o produto interno dos dois itens
}