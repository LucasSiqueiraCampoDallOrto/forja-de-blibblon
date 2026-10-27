#include "atributo.h"

Atributo::Atributo(Texto nome) { // construtor cria um atributo com o nome recebido e um id 
    id_atributo = contador++;
    nome_atributo = nome;
}

int Atributo::contador = 0; // cria o contador compartilhado começando em zero
