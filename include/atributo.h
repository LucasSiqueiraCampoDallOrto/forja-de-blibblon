#ifndef TP_ATRIBUTO_H
#define TP_ATRIBUTO_H
#include "texto.h"

class Atributo {
    private:
        // mantém um único contador compartilhado por todos os atributos para gerar id diferentes
        static int contador;
    public:
        int id_atributo;
        Texto nome_atributo;
        Atributo(Texto nome); // recebe nome -> reserva prox id -> guarda nome
    };

#endif
