#ifndef TP_CATALOGO_H
#define TP_CATALOGO_H

#include "instrumentacao.h"
#include "tipos_catalogo.h"
#include "comprimida_linhas.h"
#include "indice_faixas.h"
#include "percurso.h"

class Matriz_modificadores {
    private:

        EstatisticasCatalogo estatisticas; // guarda as contagens de células percorridas e de comparações realizadas nas buscas
        ComprimidaLinhas comprimida; // armazena a matriz no modo C usando vetores de valores, atributos e inícios das linhas
        IndiceFaixas indice_faixas; // organiza os modificadores por atributo e valor para realizar consultas por faixa
        bool manter_indice = true; // indica se as alterações devem atualizar também o índice das consultas LF
        void registrar_alteracao(int item, int atributo, int antigo, int novo); // informa ao índice de faixas a mudança do valor de um modificador, se manter_indice estiver ativo
 
        char modo;
    
        // modo T
        Tripleta* tripletas; // aponta para o endereço do vetor de tripletas
        int capacidade_tripletas;

        // modo L
        Celula** cabeca_linha; // aponta para o vetor de ponteiros que guarda as primeiras células de cada linha
        Celula** cabeca_coluna; // aponta para o vetor de ponteiros que guarda as primeiras células de cada coluna
        int capacidade_linha;
        int capacidade_coluna;
        int quantidade; // total de celulas nao nulas

        void posicao_valida(int id_item, int id_atributo); 
        // se o id do item ou do atributo for negativo a posição é inválida
    
        void expandir_cabecas(int id_item, int id_atributo);
        // vê se as linhas e colunas são suficientes -> aloca vetores maiores 

        void expandir_tripletas();
        // vê se as tripletas são suficientes -> aumenta a capacidade

        int busca_posicao_T(int id_item, int id_atributo); 
        // procura, por busca binária, a posição da tripleta ou onde ele deve ser inserida
    
    public:

        void remover_T(int id_item, int id_atributo); // remove uma tripleta do vetor 
        void inserir_T(int id_item, int id_atributo, int valor); // insere uma tripleta no vetor com um valor, se não existir
        void atualizar_T(int id_item, int id_atributo, int valor); // atualiza o valor de uma tripleta existente, remove se for 0
        int consultar_valor_T(int id_item, int id_atributo); // consulta o valor de uma tripleta se ela existir

        Matriz_modificadores(char modo_inicial = 'L'); // cria um catálogo vazio com o modo desejado
        ~Matriz_modificadores(); // destrói o catálogo ao liberar as células/tripletas e os vetores
    
        void remover_L(int id_item, int id_atributo); // remove uma célula e faz a conexão da sua anterior com sua próxima
        void inserir_L(int id_item, int id_atributo, int valor); // insere uma celula e faz a conexão com a linha e coluna na posição informada
        void atualizar_L(int id_item, int id_atributo, int valor); // atualiza o valor de uma célula
        int consultar_valor_L(int id_item, int id_atributo); // consulta o valor de uma celula

        void registrar_itens(int total); // informa ao índice o total de itens e, no modo C, garante as linhas correspondentes
        void registrar_atributos(int total); // informa ao índice o total de atributos e prepara as colunas correspondentes
        ResultadoFaixa* consultar_faixa(int atributo, int minimo, int maximo, int& total); // retorna os itens com modificador dentro da faixa inclusiva, ordenados por ID, e informa sua quantidade em total 
        EstatisticasFaixa get_estatisticas_faixa() const; // retorna as contagens de busca dos limites, cópia de resultados e ordenação das consultas LF
        unsigned long long memoria_indice_estimada() const; // estima a memória alocada dinamicamente pelo índice de faixas
        unsigned long long memoria_representacao_estimada() const; // estima a memória alocada para representar a matriz
    
        int get_quantidade() const; // informa quantas celulas n nulas existem
        void mudar_modo(char novo_modo); // muda o modo de representação das celulas
        char get_modo(); // retorna o modo atual
    
        EstatisticasCatalogo get_estatisticas() const; // retorna as contagens de células percorridas e de comparações nas buscas do catálogo
        void zerar_estatisticas(); // zera os contadores do catálogo e do índice de faixas
        unsigned long long memoria_estimada() const; // estima a memória do próprio objeto, da representação da matriz e do índice de faixas
    
        // Proíbe a cópia da matriz para evitar dois donos dos mesmos ponteiros.
        Matriz_modificadores(const Matriz_modificadores&) = delete;
        Matriz_modificadores& operator=(const Matriz_modificadores&) = delete;
    
        int consultar(int item, int atributo); // consulta uma celula a partir da operação de remoção do modo atual
        void remover(int item, int atributo); // remove uma celula a partir da operação de remoção do modo atual
        void atribuir(int item, int atributo, int valor); // insere uma celula ou atualiza o valor dela 
        
        Percurso percorrer_linha(int item); // cria um percurso para percorrer as linhas (os atributos n nulos de um item)
        Percurso percorrer_coluna(int atributo); // cria um percurso para percorrer as colunas (os itens que modificam um atributo)
        double produto_interno(int item1, int item2); // soma o produto dos valores que aparecem no mesmo atributo de dois percursos de linhas
    };

#endif
