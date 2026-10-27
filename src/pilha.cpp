#include "pilha.h"

// organiza os equipamentos em uma pilha onde o último item equipado é o primeiro a ser removido

// cria uma pilha de equipamentos vazia
Pilha::Pilha() {
    topo = nullptr; // começa sem nenhum equipamento
}

// libera os equipamentos restantes ao destruir a pilha
Pilha::~Pilha() {
    int item; // recebe o ID de cada item retirado

    while (desempilhar(item)) {} // remove e libera os nós até esvaziar
}

// verifica se a pilha está vazia
bool Pilha::vazia() const {
    return topo == nullptr;
}

// coloca um item no topo, ligado ao equipamento que estava antes dele
void Pilha::empilhar(int id_item) {
    topo = new ItemEquipado{id_item, topo};
}

// remove o último equipamento colocado e informa seu ID
bool Pilha::desempilhar(int& id_item) {
    if (vazia()) return false; // não existe equipamento para retirar

    ItemEquipado* antigo = topo; // guarda o endereço do nó que será removido
    id_item = antigo->id_item; // entrega o ID do item pela referência

    topo = antigo->anterior; // o equipamento anterior passa a ser o topo
    delete antigo; // libera o nó da pilha; o item continua cadastrado no catálogo

    return true; // informa que um equipamento foi retirado
}

// retorna um ponteiro para consultar o topo sem retirar o equipamento
const ItemEquipado* Pilha::get_topo() const {
    return topo;
}