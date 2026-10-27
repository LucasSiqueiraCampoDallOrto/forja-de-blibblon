#ifndef TP_PILHA_H
#define TP_PILHA_H

struct ItemEquipado {
    int id_item;
    ItemEquipado* anterior; // aponta para o equipamento anterior na pilha
};

class Pilha {
    private:
        ItemEquipado* topo; // aponta para a peça equipada mais recentemente

    public:
        Pilha(); // Cria uma pilha sem equipamentos.
    
        // proíbe a cópia da pilha para evitar dois donos dos mesmos ponteiros
        Pilha(const Pilha&) = delete;
        Pilha& operator=(const Pilha&) = delete;
        
        ~Pilha(); // libera os nós de equipamentos e destrói a pilha

        bool vazia() const; // ve se n tem equipamento no topo
        void empilhar(int id_item); // coloca um item no topo dos equipamentos
        bool desempilhar(int& id_item); /// remove a ultima peça equipada

        const ItemEquipado* get_topo() const; // consulta o topo
    
};

#endif
