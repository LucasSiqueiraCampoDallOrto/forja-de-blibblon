#include "fila.h"

// organiza as encomendas por ordem de chegada: novos pedidos entram no final e os mais antigos saem pela frente

// cria uma fila de encomendas vazia
Fila::Fila() {
    frente = nullptr; // começa sem primeiro pedido
    tras = nullptr; // começa sem último pedido
    quantidade = 0;
}

// libera os pedidos restantes ao destruir a fila
Fila::~Fila() {
    int aventureiro, item; // recebem os IDs dos pedidos removidos

    while (desenfileirar(aventureiro, item)) {} // remove e libera os pedidos até esvaziar
}

// verifica se a fila está vazia
bool Fila::vazia() const {
    return frente == nullptr;
}

// retorna a quantidade de pedidos na fila
int Fila::get_quantidade() const {
    return quantidade;
}

// coloca uma nova encomenda no final da fila
void Fila::enfileirar(int id_aventureiro, int id_item) {
    Encomenda* nova = new Encomenda{
        id_aventureiro, // quem fez o pedido
        id_item, // qual item foi pedido
        nullptr // não existe pedido depois deste
    };

    if (vazia()) {
        frente = nova; // se a fila estava vazia, este também é o primeiro pedido
    } else {
        tras->proxima = nova; // conecta o último pedido anterior ao novo
    }

    tras = nova; // atualiza o último pedido
    quantidade++;
}

// remove o primeiro pedido e informa os IDs do aventureiro e do item
bool Fila::desenfileirar(int& id_aventureiro, int& id_item) {
    if (vazia()) return false; // não existe pedido para retirar

    Encomenda* antiga = frente; // aponta para o pedido que será removido

    id_aventureiro = antiga->id_aventureiro; // entrega o ID do aventureiro pela referência
    id_item = antiga->id_item; // entrega o ID do item pela referência

    frente = antiga->proxima; // o próximo pedido passa a ser o primeiro

    if (frente == nullptr) {
        tras = nullptr; // se a fila ficou vazia, também deixa de existir um último pedido
    }

    delete antiga; // libera o pedido removido
    quantidade--;

    return true; // informa que um pedido foi retirado
}