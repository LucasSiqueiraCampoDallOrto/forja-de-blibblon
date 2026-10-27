#ifndef TP_SISTEMA_FORJA_H
#define TP_SISTEMA_FORJA_H

#include "atributo.h"
#include "item.h"
#include "aventureiro.h"
#include "catalogo.h"
#include "fila.h"
#include "metricas.h"

class SistemaForja {
    public:
    
        Matriz_modificadores matriz; // cria o catalogo de itens, atributos e valores
        Atributo** atributos; // vetor dinamico que aponta para todos os atributos
        Item** itens; // vetor dinamico que aponta para todos os itens
        Aventureiro** aventureiros; // vetor dinamico que aponta para todos os aventureiros

        // total cadastrado
        int total_atributos;
        int total_itens;
        int total_aventureiros;

        // capacidade maxima
        int capacidade_atributos;
        int capacidade_itens;
        int capacidade_aventureiros;

        Fila fila; // fila de encomenda compartilhada por todos os aventureiros
        SistemaForja(); // inicializa a forja com cadastros vazios, fila vazia e catalogo inicial
        ~SistemaForja(); // libera os objetos cadastrados e seus vetores ao encerrar a forja
    
        void cadastrar_atributo(Texto nome); // cadastra um atributo
        void cadastrar_item(Texto nome); // cadastra um item
        void cadastrar_aventureiro(Texto nome); // cadastra um aventureiro

        void modificar(int id_item, int id_atributo, int valor); // insere ou atualiza o modificador de um item em um atributo
        // ex: modificar(0, 1, 5) define que o item 0 concede +5 no atributo 1.
        
        void encomendar(int id_aventureiro, int id_item); // coloca o pedido no fim da fila e retorna sua posiçao
        void trabalhar(int k); // atende até k encomendas e empilha cada item para seu aventureiro
        void desequipar(int id_aventureiro); // remove a peça mais recente do aventureiro

        void listar_item(int id_item); // lista os modificadores n nulos do item
        void listar_atributo(int id_atributo); // lista os itens que modificam o atributo
        void listar_build(int id_aventureiro); // mostra os totais de cada atributo da build do aventureiro
        
        void recomendar(int id_aventureiro, int n, int m, const MetricaConsulta* metricas); 
        // recomenda n itens ao calcular build e pontos -> excluir equipados -> ordenar candidatos -> imprimir melhores
        void listar_faixa(int atributo, int minimo, int maximo); // imprime os itens cujos modificadores estao na faixa pedida
        void executar(); // le o comenda e executa a operação correspondende 

};
#endif
