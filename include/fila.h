#ifndef TP_FILA_H
#define TP_FILA_H

struct Encomenda {
    int id_aventureiro; // guarda o id do aventureiro que fez a encomenda
    int id_item; // guarda o id do item solicitado na encomenda
    Encomenda* proxima; // aponta para a encomenda seguinte
};

class Fila {
    private:

        Encomenda* frente; // aponta para o pedido mais antigo que vai ser atendido primeiro
        Encomenda* tras; // aponta para o último pedido, onde será ligada a próxima encomenda
        int quantidade; // qntd de encomendas

    public:

        Fila(); // cria uma fila de encomendas vazia.

        // proíbe a cópia da fila para evitar dois donos dos mesmos ponteiros
        Fila(const Fila&) = delete; 
        Fila& operator=(const Fila&) = delete;
    
        ~Fila();  // libera as encomendas que ainda estão esperando na fila.

        bool vazia() const; // verifica se não há encomenda na frente da fila
        int get_quantidade() const; // mostra qnts pedidas esperam pra ser atendido
        void enfileirar(int id_aventureiro, int id_item); // acrescenta uma encomenda ao fim da fila global
        bool desenfileirar(int& id_aventureiro, int& id_item); // retira o pedido mais antigo e informa quem pediu e qual item deve ser equipado
    
    };
#endif
