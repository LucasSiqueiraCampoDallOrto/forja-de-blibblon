#ifndef TP_ITEM_H
#define TP_ITEM_H

#include "texto.h"

class Item {
    private:
        // mantém um contador compartilhado para gerar o próximo id de item.
        static int contador;
    public:

        int id_item;
        Texto nome_item;
        Item(Texto nome);
        int get_id();
};

#endif
