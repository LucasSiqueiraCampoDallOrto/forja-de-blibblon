#include "percurso.h"

// lê uma célula por chamada, seguindo os ponteiros no modo L, o vetor de tripletas no modo T ou os vetores comprimidos no modo C

// entrega a próxima célula em resultado e retorna false quando o percurso termina
bool Percurso::proximo(Tripleta& resultado) {
    if (modo == 'C') {
        if (!por_coluna) { // percorre os modificadores de um item
            if (posicao == fim) return false; // chegou ao fim do trecho do item

            if (estatisticas) registrar_contagem(estatisticas->celulas_percorridas); // conta a célula se habilitado

            resultado = Tripleta{id, comprimida->atributo_em(posicao), comprimida->valor_em(posicao)}; // reúne o item, o atributo e o valor
            posicao++; // prepara a leitura da próxima posição
            return true;
        }

        while (posicao < fim) { // procura o atributo em cada item
            int item = posicao++; // guarda o item atual e avança para o próximo
            int valor = comprimida->consultar(item, id, estatisticas);

            if (valor != 0) { // encontrou um item que modifica o atributo
                if (estatisticas) registrar_contagem(estatisticas->celulas_percorridas);

                resultado = Tripleta{item, id, valor};
                return true;
            }
        }

        return false; // terminou de examinar os itens
    }

    if (modo == 'L') {
        if (atual == nullptr) return false; // não existe outra célula para ler

        if (estatisticas != nullptr) registrar_contagem(estatisticas->celulas_percorridas);

        resultado = Tripleta{ // copia os dados da célula atual
            atual->id_item,
            atual->id_atributo,
            atual->valor
        };

        if (por_coluna) {
            atual = atual->celula_abaixo; // segue para o próximo item que modifica esse atributo
        } else {
            atual = atual->celula_direita; // segue para o próximo atributo desse item
        }

        return true;
    }

    while (posicao < fim) { // percorre o vetor do modo T
        if (estatisticas != nullptr) registrar_contagem(estatisticas->celulas_percorridas);

        Tripleta celula = vetor[posicao++]; // lê a tripleta atual e avança

        if (!por_coluna && celula.id_item != id) {
            return false; // encontrou outro item, então terminou a linha procurada
        }

        if (!por_coluna || celula.id_atributo == id) { // aceita a célula da linha ou do atributo procurado
            resultado = celula;
            return true;
        }
    }

    return false; // não encontrou outra célula para entregar
}