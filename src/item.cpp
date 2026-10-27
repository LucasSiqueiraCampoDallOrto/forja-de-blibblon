#include "item.h"

Item::Item(Texto nome) { // cria um item com o nome recebido e um ID
    id_item = contador++; // atribui o próximo ID e aumenta o contador
    nome_item = nome; // guarda o nome recebido
}

// retorna o ID deste item
int Item::get_id() {
    return id_item;
}

int Item::contador = 0; // cria o contador compartilhado começando em zero