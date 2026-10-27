#ifndef TP_INDICE_FAIXAS_H
#define TP_INDICE_FAIXAS_H

#include "instrumentacao.h"

struct ResultadoFaixa { 
    int id_item; 
    int valor; 
};

struct IntervaloZero { // guarda o primeiro e o último id de um grupo de itens com modificador zero
    int primeiro; 
    int ultimo; 
};

struct EstatisticasFaixa { // guarda as contagens usadas para avaliar o desempenho das consultas lf
    
    unsigned long long comparacoes_limites = 0; // qntd de comparações para encontrar os limites da faixa
    unsigned long long resultados_copiados = 0; // qntd de resultados copiados para as respostas
    unsigned long long comparacoes_ordenacao = 0; // qntd de comparações para ordenar os resultados por id
};

// Organiza os itens pelos valores que possuem em um único atributo.
class ColunaFaixa {

    ResultadoFaixa* pares = nullptr; // vetor que guarda os itens com modificador n nulo ordenados por valor e id
    int quantidade = 0, capacidade = 0; // qntd de pares e capacidade max
    IntervaloZero* zeros = nullptr; // vetor que guarda grupos de id dos itens que tem modificador zero em um atributo
    int intervalos = 0, capacidade_zeros = 0; // qntd de grupos de zeros e capacidade max
    int total_itens = 0; // total de itens

    void reservar_pares(); // aumenta o vetor de pares quando falta espaço
    void reservar_zeros(); // aumenta o vetor de grupos de zeros quando falta espaço
    int posicao_par(int item, int valor) const; // busca onde o par item/valor está ou deve ser inserido
    void apagar_intervalo(int p); // remove o grupo de zeros da posição p
    void retirar_zero(int item); // retira o item dos grupos de zeros porque ele ganhou um modificador
    void devolver_zero(int item); // recoloca o item nos grupos de zeros porque seu modificador foi removido
    int limite(int valor, bool superior, EstatisticasFaixa& est) const; // busca a primeira posição com valor >= ao recebido ou > se superior for true
    static bool maior_id(const ResultadoFaixa& a, const ResultadoFaixa& b, EstatisticasFaixa& est); // verifica se o ID de a é maior que o de b e contabiliza a comparação
    static void descer_heap(ResultadoFaixa* v, int n, int raiz, EstatisticasFaixa& est); // ajusta uma parte do heap para manter os maiores id acima dos menores

    public:
        
        ColunaFaixa() = default; // cria a estrutura vazia para um atributo
        ColunaFaixa(const ColunaFaixa&) = delete; // impede criar uma cópia que compartilhe os mesmos vetores
        ColunaFaixa& operator=(const ColunaFaixa&) = delete; // impede copiar este objeto usando o sinal de igual
        ~ColunaFaixa(); // libera os vetores de pares e de grupos de zeros
        void garantir_itens(int total); // registra os novos itens inicialmente com modificador zero neste atributo
        void alterar(int item, int antigo, int novo); // atualiza os pares e os grupos de zeros quando o modificador de um item muda
        ResultadoFaixa* consultar(int minimo, int maximo, int& total, EstatisticasFaixa& est) const; // retorna os itens da faixa em ordem de id
        unsigned long long memoria_dinamica() const; // estima a memoria reservada para os pares e os grupos de zeros
};

// Reúne as estruturas de todos os atributos para responder às consultas LF.
class IndiceFaixas {

    ColunaFaixa** colunas = nullptr; // vetor dinamico que aponta para todas ColunasFaixas de todos atributos
    int atributos = 0, capacidade = 0, itens = 0; // qntd de atributos, capacidade max para colunas e qtnd de itens.
    EstatisticasFaixa estatisticas; // guarda as contagens acumuladas durante as consultas LF.

public:
    
    IndiceFaixas() = default; // cria o índice vazio

    // impede criar uma cópia que compartilhe as mesmas estruturas
    IndiceFaixas(const IndiceFaixas&) = delete;

    // impede copiar este objeto usando o sinal de igual
    IndiceFaixas& operator=(const IndiceFaixas&) = delete;

    // libera as estruturas de cada atributo e o vetor de ponteiros
    ~IndiceFaixas();

    // retorna a quantidade de itens conhecidos pelo índice
    int total_itens() const;

    // informa a todos os atributos que novos itens foram cadastrados
    void garantir_itens(int total);

    // cria as estruturas dos novos atributos, inicialmente com zero para todos os itens
    void garantir_atributos(int total);

    // atualiza o modificador do item na estrutura do atributo informado.
    void alterar(int item, int atributo, int antigo, int novo);

    // consulta a faixa no atributo informado e retorna os resultados e sua quantidade
    ResultadoFaixa* consultar(int atributo, int minimo, int maximo, int& total);

    // retorna uma cópia dos contadores das consultas lf
    EstatisticasFaixa get_estatisticas() const;

    // zera os contadores 
    void zerar_estatisticas();

    // estima a memoria alocada para o vetor de colunas e as estruturas dos atributos
    unsigned long long memoria_dinamica() const;
};

#endif