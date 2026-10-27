#ifndef TP_TIPOS_CATALOGO_H
#define TP_TIPOS_CATALOGO_H

struct Celula {
   
    int id_item;
    int id_atributo;
    int valor;

    Celula* celula_direita; // aponta para a próxima celula da linha (mesmo item)
    Celula* celula_abaixo; // aponta para a proxima celula da coluna (mesmo atributo)

    Celula(int item, int atributo, int val) {
        id_item = item;
        id_atributo = atributo;
        valor = val;
        celula_direita = nullptr;
        celula_abaixo = nullptr;
    };
};

struct Tripleta {
    int id_item;
    int id_atributo;
    int valor;
};

#endif
