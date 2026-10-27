#include "aventureiro.h"

Aventureiro::Aventureiro(Texto nome) { // cria um aventureiro com o nome recebido e um ID
    id_aventureiro = contador++;
    nome_aventureiro = nome;
}

// calcula o total de cada atributo somando os modificadores dos itens equipados
long long* Aventureiro::calcular_build(Matriz_modificadores& matriz, int total_atributos) {
    long long* totais = new long long[total_atributos](); // cria um vetor com todos os totais em zero
    const ItemEquipado* atual = equipamentos.get_topo(); // aponta para o último item equipado
    while (atual != nullptr) { // percorre todos os itens equipados
        Percurso p = matriz.percorrer_linha(atual->id_item); // prepara a leitura dos modificadores desse item
        Tripleta celula; // recebe um modificador por vez
        while (p.proximo(celula)) { // lê cada modificador do item
            totais[celula.id_atributo] += celula.valor; // soma o valor ao total do atributo correspondente
        }
        atual = atual->anterior; // passa para o item equipado antes deste
    }
    return totais; // devolve os totais
}

int Aventureiro::contador = 0; // cria o contador compartilhado começando em zero