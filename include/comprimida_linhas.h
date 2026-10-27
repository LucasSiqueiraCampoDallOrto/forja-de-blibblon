#ifndef TP_COMPRIMIDA_LINHAS_H
#define TP_COMPRIMIDA_LINHAS_H

#include "instrumentacao.h"


class ComprimidaLinhas { 
  
    int* valores = nullptr; // esse vetor guarda os modificadores n nulos de cada item
    int* atributos = nullptr; // esse vetor guarda os atributos correspondente a cada modificador
    int* inicio = nullptr; // esse vetor indica qual a parte (quais modificadores) pertence a cada item

    // qntd de linhas e sua capacidade max
    int linhas = 0, capacidade_linhas = 0;

    // qntd de modificadores e sua capacidade max
    int quantidade = 0, capacidade = 0;

    void reservar_celulas(int minimo); // aumenta o vetor de modificadores se precisar de espaço

public:

    ComprimidaLinhas() = default; // inicializa a estrutura vazia

    // n deixa criar uma cópia que compartilhe os mesmos vetores
    ComprimidaLinhas(const ComprimidaLinhas&) = delete;

    // n deixa copiar os vetores de outro objeto usando o sinal de igual
    ComprimidaLinhas& operator=(const ComprimidaLinhas&) = delete;

    ~ComprimidaLinhas(); // libera os vetores e destrói a estrutura

    void limpar(); // apaga os dados das linhas
    void garantir_linhas(int total); // aumenta o numero de linhas para o necessario
    int inicio_linha(int item) const; // retorna onde começa os modificadores do item no vetor
    int fim_linha(int item) const; // retorna a posicao 
    int total_linhas() const; // qtd total de linhas
    int total_celulas() const; // qntd total de modificadores guardados
    int atributo_em(int p) const; // retorna o id do atributo na posição p
    int valor_em(int p) const; // retorna o valor do modificador na posição p

    // busca a posição do atributo no item ou onde ele deve ser inserido
    int localizar(int item, int atributo, EstatisticasCatalogo* e = nullptr) const; 

    // retorna o modificador do item naquele atributo
    int consultar(int item, int atributo, EstatisticasCatalogo* e = nullptr) const;

    // insere ou atualiza um modificador
    void atribuir(int item, int atributo, int valor, EstatisticasCatalogo* e = nullptr);

    // prepara os vetores para receber várias células durante uma migração
    void iniciar_construcao(int total_linhas, int total_celulas);

    // adiciona ao final uma célula recebida na ordem correta de item e atributo.
    void anexar_ordenado(int item, int atributo, int valor);

    // calcula onde começam os dados de cada item após adicionar as células.
    void concluir_construcao();

    // recebe os vetores de outro objeto 
    void tomar_de(ComprimidaLinhas& outra);

    // retorna uma estimativa da memoria reservada para os três vetores.
    unsigned long long memoria_dinamica() const;
};

#endif