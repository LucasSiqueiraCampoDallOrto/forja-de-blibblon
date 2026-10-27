#include "sistema_forja.h"
#include <iostream>
using namespace std;
#include <cassert>
#include <climits>

// guarda o estado do gerador, começando sempre com a mesma semente
static unsigned int estado = 712367;

// gera o próximo número da sequência usada nos testes
static unsigned int aleatorio() {
    // atualiza estado -> devolve número.
    estado = 1664525U * estado + 1013904223U;
    return estado;
}

// compara o catálogo com uma matriz simples que guarda os valores esperados
static void conferir(Matriz_modificadores& m, int referencia[27][13]) {
    // confere linhas -> confere colunas e faixas -> confere produto interno

    int quantidade = 0; // conta o total esperado de células não nulas

    // confere as consultas e o percurso de cada linha
    for (int i = 0; i < 27; i = i + 1) {
        Percurso p = m.percorrer_linha(i);
        Tripleta t; // recebe uma célula por vez

        int anterior = -1; // último id de atributo encontrado
        int lidos = 0;
        int esperados = 0;

        for (int a = 0; a < 13; a = a + 1) {
            assert(m.consultar(i, a) == referencia[i][a]);

            if (referencia[i][a] != 0) {
                esperados = esperados + 1;
                quantidade = quantidade + 1;
            }
        }

        while (p.proximo(t)) {
            // confere o item e a ordem crescente dos atributos
            assert(t.id_item == i && t.id_atributo > anterior);

            // confere se a célula existe e tem o valor esperado
            assert(t.valor != 0 && t.valor == referencia[i][t.id_atributo]);

            anterior = t.id_atributo;
            lidos = lidos + 1;
        }

        assert(lidos == esperados);
    }

    assert(m.get_quantidade() == quantidade);

    // confere o percurso de cada coluna e as consultas por faixa
    for (int a = 0; a < 13; a = a + 1) {
        Percurso p = m.percorrer_coluna(a);
        Tripleta t;

        int anterior = -1; // último ID de item encontrado
        int lidos = 0;
        int esperados = 0;

        // conta quantos itens modificam este atributo
        for (int i = 0; i < 27; i = i + 1) {
            if (referencia[i][a] != 0) {
                esperados = esperados + 1;
            }
        }

        while (p.proximo(t)) {
            // confere o atributo e a ordem crescente dos itens
            assert(t.id_atributo == a && t.id_item > anterior);
            assert(t.valor != 0 && t.valor == referencia[t.id_item][a]);

            anterior = t.id_item;
            lidos = lidos + 1;
        }

        assert(lidos == esperados);

        // testa limites extremos, zeros, valores específicos e uma faixa invertida
        const int faixas[][2] = {
            {INT_MIN, INT_MAX},
            {0, 0},
            {-3, 3},
            {1, 1},
            {-100, -50},
            {6, 6},
            {5, -5}
        };

        for (int f = 0; f < 7; f = f + 1) {
            int total; // recebe a quantidade de resultados da consulta
            int j = 0; // posição esperada na resposta

            ResultadoFaixa* r = m.consultar_faixa(
                a, faixas[f][0], faixas[f][1], total
            );

            // confere a resposta usando todos os valores da referência, incluindo zeros
            for (int i = 0; i < 27; i = i + 1) {
                int v = referencia[i][a];

                if (faixas[f][0] <= v && v <= faixas[f][1]) {
                    assert(j < total && r[j].id_item == i && r[j].valor == v);
                    j = j + 1;
                }
            }

            assert(j == total); // confere se não faltaram nem sobraram resultados
            delete[] r;
        }
    }

    // escolhe dois itens para conferir o produto interno
    int i = aleatorio() % 27;
    int j = aleatorio() % 27;
    double dot = 0;

    for (int a = 0; a < 13; a = a + 1) {
        dot = dot + static_cast<double>(referencia[i][a]) * referencia[j][a];
    }

    assert(m.produto_interno(i, j) == dot);
}

// testa alterações, migrações e consultas, comparando os resultados esperados
int main() {
    // testa cada modo inicial -> aplica operações -> confere zeros e novas entidades

    // repete os testes começando em L, T e C
    for (int inicial = 0; inicial < 3; inicial = inicial + 1) {
        const char modos[] = "LTC";

        Matriz_modificadores m(modos[inicial]);
        int referencia[27][13] = {}; // começa com todos os valores iguais a zero

        m.registrar_itens(27);
        m.registrar_atributos(13);

        conferir(m, referencia);

        // aplica uma sequência de operações gerada a partir da semente
        for (int passo = 0; passo < 3500; passo = passo + 1) {
            int i = aleatorio() % 27;
            int a = aleatorio() % 13;
            int v = static_cast<int>(aleatorio() % 13) - 6;
            int op = aleatorio() % 6;

            if (op == 0) {
                // muda a representação sem alterar os valores esperados
                m.mudar_modo(modos[aleatorio() % 3]);

            } else if (op == 1) {
                m.remover(i, a);
                referencia[i][a] = 0;

            } else if (op == 2 && m.get_modo() != 'C') {
                // testa diretamente a inserção dos modos L e T
                if (m.get_modo() == 'L') {
                    m.inserir_L(i, a, v);
                } else {
                    m.inserir_T(i, a, v);
                }

                // atualiza a referência se recebeu zero ou se a célula não existia
                if (v == 0 || referencia[i][a] == 0) {
                    referencia[i][a] = v;
                }

            } else if (op == 3 && m.get_modo() != 'C') {
                // testa diretamente a atualização dos modos L e T
                if (m.get_modo() == 'L') {
                    m.atualizar_L(i, a, v);
                } else {
                    m.atualizar_T(i, a, v);
                }

                // a atualização só muda a referência quando a célula já existe
                if (referencia[i][a] != 0) {
                    referencia[i][a] = v;
                }

            } else {
                // testa a operação geral de inserir, substituir ou remover
                m.atribuir(i, a, v);
                referencia[i][a] = v;
            }

            // confere o estado periodicamente durante a sequência
            if (passo % 31 == 0) {
                conferir(m, referencia);
            }
        }

        conferir(m, referencia); // confere o estado final deste teste
    }

    // cria um catálogo com milhares de modificadores e apenas um zero
    Matriz_modificadores m('C');
    m.registrar_itens(4096);
    m.registrar_atributos(1);

    for (int i = 0; i < 4096; i = i + 1) {
        if (i != 2000) {
            m.atribuir(i, 0, 1);
        }
    }

    m.zerar_estatisticas();
    int total;

    // a consulta deve encontrar somente o item que ficou sem célula
    ResultadoFaixa* r = m.consultar_faixa(0, 0, 0, total);

    assert(total == 1 && r[0].id_item == 2000 && r[0].valor == 0);
    delete[] r;

#if FORJA_CONTADORES
    // confere as contagens quando a instrumentação está habilitada
    EstatisticasFaixa e = m.get_estatisticas_faixa();

    assert(e.comparacoes_limites <= 26 && e.resultados_copiados == 1);
#endif

    // acrescenta itens e um atributo depois de migrar a representação
    m.mudar_modo('L');
    m.registrar_itens(4098);
    m.registrar_atributos(2);

    // o atributo recém-cadastrado vale zero para todos os itens
    r = m.consultar_faixa(1, 0, 0, total);
    assert(total == 4098);
    delete[] r;

    // no atributo antigo, há o zero anterior e os zeros dos dois novos itens
    r = m.consultar_faixa(0, 0, 0, total);
    assert(total == 3);
    delete[] r;

    cout << "OK: TADs, LF, zeros, produto interno e migracoes L/T/C.\n";
}