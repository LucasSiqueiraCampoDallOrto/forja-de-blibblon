#ifndef TP_METRICAS_H
#define TP_METRICAS_H

#include "catalogo.h"

enum class TipoMetrica { 
    PODER, // soma os modificadores de um item
    EQUILIBRIO, // baseia na dispersão dos modificadores do item
    SINERGIA, // compara o item com a build do aventureiro
};

struct MetricaConsulta { // define uma metrica e seu peso no calculo
    TipoMetrica tipo; // guarda a metrica a ser usada
    double peso; // peso da metrica
};

struct ItemOrdenado { // id + valor arredondado para ordenar e desempatar
    int id_item;
    double valor_arredondado;
};

double ler_modificador(Matriz_modificadores& matriz, int id_item, int id_atributo); // consulta o modificador de um item em um atributo

// calcula o poder somando os modificadores do item
double calcular_poder(Matriz_modificadores& matriz, int id_item, int qntd_atributo);

// calcula 1 dividido por 1 mais o desvio padrão dos modificadores
double calcular_equilibrio(Matriz_modificadores& matriz, int id_item, int qntd_atributo);

// mede a semelhança entre os modificadores do item e a build pelo cosseno entre esses vetores
double calcular_sinergia(Matriz_modificadores& matriz, int id_item, int qntd_atributo, const double* build);

// decide a ordem por valor decrescente, usando o menor id para desempatar
bool item_vem_antes(ItemOrdenado& a, ItemOrdenado& b);

void ordenar_itens(ItemOrdenado* itens,ItemOrdenado* auxiliar, int inicio, int fim); // ordena os itens com merge sort

// calcula a pontuação final de cada item usando sua posição em cada métrica e o peso dela.
double* calcular_pontuacoes(Matriz_modificadores& matriz, int total_itens, int total_atributos, const double* build, const MetricaConsulta* metricas, int quantidade_metricas);
// transforma suas posições nos rankings em pontos, multiplica pelos pesos e soma tudo

#endif
