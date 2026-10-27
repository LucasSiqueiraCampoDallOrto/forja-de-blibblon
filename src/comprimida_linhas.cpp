#include "comprimida_linhas.h"

// aumenta os vetores de valores e atributos quando falta espaço
void ComprimidaLinhas::reservar_celulas(int minimo) {
    if (minimo <= capacidade) return; // já existe espaço suficiente

    int nova; // usa a capacidade atual ou começa com quatro posições
    if (capacidade) {
        nova = capacidade;
    } else {
        nova = 4;
    }

    while (nova < minimo) nova *= 2; // dobra até atingir o espaço necessário

    int* novos_valores = new int[nova]; // cria o novo vetor de modificadores
    int* novos_atributos = new int[nova]; // cria o novo vetor de IDs dos atributos

    for (int p = 0; p < quantidade; p++) {
        novos_valores[p] = valores[p]; // copia cada modificador
        novos_atributos[p] = atributos[p]; // copia o atributo correspondente
    }

    delete[] valores; // libera os vetores antigos
    delete[] atributos;

    valores = novos_valores; // passa a usar os novos vetores
    atributos = novos_atributos;
    capacidade = nova;
}

// libera os vetores quando o objeto é destruído
ComprimidaLinhas::~ComprimidaLinhas() {
    limpar();
}

// apaga os dados e deixa a estrutura vazia
void ComprimidaLinhas::limpar() {
    delete[] valores; delete[] atributos; delete[] inicio; // libera os três vetores
    valores = atributos = inicio = nullptr; // deixa os ponteiros sem endereço
    linhas = capacidade_linhas = quantidade = capacidade = 0; // zera quantidades e capacidades
}

// garante que existam linhas para todos os itens informados
void ComprimidaLinhas::garantir_linhas(int total) {
    if (total <= linhas) return; // as linhas necessárias já existem

    if (total > capacidade_linhas) { // precisa aumentar o vetor de inícios
        int nova;
        if (capacidade_linhas) {
            nova = capacidade_linhas;
        } else {
            nova = 4;
        }

        while (nova < total) nova *= 2;

        int* novos = new int[nova + 1]; // reserva uma posição extra para indicar o fim da última linha

        for (int i = 0; i <= linhas; i++) {
            // preserva os índices antigos ou começa com zero
            if (inicio) {
                novos[i] = inicio[i];
            } else {
                novos[i] = 0;
            }
        }

        delete[] inicio;
        inicio = novos;
        capacidade_linhas = nova;
    }

    // as novas linhas ficam vazias, começando e terminando no fim dos dados
    for (int i = linhas + 1; i <= total; i++) inicio[i] = quantidade;

    linhas = total;
}

// retorna onde começam os modificadores do item ou o fim dos dados se a linha não existir
int ComprimidaLinhas::inicio_linha(int item) const {
    if (item < linhas) {
        return inicio[item];
    } else {
        return quantidade;
    }
}

// retorna a posição logo após os modificadores do item ou o fim dos dados se a linha não existir
int ComprimidaLinhas::fim_linha(int item) const {
    if (item < linhas) {
        return inicio[item + 1];
    } else {
        return quantidade;
    }
}

// retorna a quantidade de linhas
int ComprimidaLinhas::total_linhas() const {
    return linhas;
}

// retorna a quantidade de modificadores armazenados
int ComprimidaLinhas::total_celulas() const {
    return quantidade;
}

// retorna o ID do atributo guardado na posição p
int ComprimidaLinhas::atributo_em(int p) const {
    return atributos[p];
}

// retorna o modificador guardado na posição p
int ComprimidaLinhas::valor_em(int p) const {
    return valores[p];
}

// busca a posição do atributo no item ou onde ele deveria ser inserido
int ComprimidaLinhas::localizar(int item, int atributo, EstatisticasCatalogo* e) const {
    int esquerda = inicio_linha(item), direita = fim_linha(item); // limita a busca ao trecho desse item

    while (esquerda < direita) {
        int meio = esquerda + (direita - esquerda) / 2; // encontra o meio do intervalo

        if (e) registrar_contagem(e->comparacoes_busca); // registra a comparação se a contagem estiver habilitada

        if (atributos[meio] < atributo) esquerda = meio + 1; // continua na metade direita
        else direita = meio; // continua procurando à esquerda
    }

    return esquerda; // posição encontrada ou posição para inserir
}

// retorna o modificador do item naquele atributo ou zero se não existir
int ComprimidaLinhas::consultar(int item, int atributo, EstatisticasCatalogo* e) const {
    int p = localizar(item, atributo, e); // procura o atributo no trecho do item

    if (p < fim_linha(item) && atributos[p] == atributo) {
        return valores[p];
    } else {
        return 0;
    }
}

// insere ou atualiza um modificador; se o valor for zero, remove
void ComprimidaLinhas::atribuir(int item, int atributo, int valor, EstatisticasCatalogo* e) {
    garantir_linhas(item + 1); // garante que a linha do item exista

    int p = localizar(item, atributo, e);
    bool existe = p < fim_linha(item) && atributos[p] == atributo; // verifica se encontrou a célula

    if (existe && valor != 0) { valores[p] = valor; return; } // substitui o valor existente
    if (!existe && valor == 0) return; // não existe célula para remover

    int diferenca; // será -1 para remoção ou +1 para inserção

    if (existe) { // a célula existe e recebeu zero, então deve ser removida
        for (int j = p; j + 1 < quantidade; j++) {
            valores[j] = valores[j + 1]; atributos[j] = atributos[j + 1]; // fecha o espaço movendo os dados para a esquerda
        }

        quantidade--; diferenca = -1;
    } else {
        reservar_celulas(quantidade + 1); // garante espaço para o novo modificador

        for (int j = quantidade; j > p; j--) {
            valores[j] = valores[j - 1]; atributos[j] = atributos[j - 1]; // abre espaço movendo os dados para a direita
        }

        valores[p] = valor; atributos[p] = atributo; // guarda o novo modificador e seu atributo
        quantidade++; diferenca = 1;
    }

    // ajusta o fim desta linha e os limites das seguintes após o deslocamento
    for (int i = item + 1; i <= linhas; i++) inicio[i] += diferenca;
}

// esvazia a estrutura e prepara espaço para receber as células de uma migração
void ComprimidaLinhas::iniciar_construcao(int total_linhas, int total_celulas) {
    limpar(); garantir_linhas(total_linhas); reservar_celulas(total_celulas);
}

// adiciona ao final uma célula recebida na ordem correta de item e atributo
void ComprimidaLinhas::anexar_ordenado(int item, int atributo, int valor) {
    valores[quantidade] = valor; atributos[quantidade] = atributo; // escreve na próxima posição livre
    quantidade++; inicio[item + 1]++; // conta a nova célula e quantas pertencem a esse item
}

// transforma as contagens de células em posições que delimitam cada linha
void ComprimidaLinhas::concluir_construcao() {
    for (int i = 1; i <= linhas; i++) inicio[i] += inicio[i - 1]; // soma as quantidades das linhas anteriores
}

// recebe os vetores de outro objeto e deixa o objeto de origem vazio
void ComprimidaLinhas::tomar_de(ComprimidaLinhas& outra) {
    limpar(); // libera os dados atuais antes de receber os novos

    valores = outra.valores; atributos = outra.atributos; inicio = outra.inicio; // transfere os endereços dos vetores
    linhas = outra.linhas; capacidade_linhas = outra.capacidade_linhas; // recebe o total e a capacidade de linhas
    quantidade = outra.quantidade; capacidade = outra.capacidade; // recebe o total e a capacidade de células

    outra.valores = outra.atributos = outra.inicio = nullptr; // impede que a origem libere os vetores transferidos
    outra.linhas = outra.capacidade_linhas = outra.quantidade = outra.capacidade = 0;
}

// estima os bytes reservados para os três vetores
unsigned long long ComprimidaLinhas::memoria_dinamica() const {
    unsigned long long bytes = 2ULL * capacidade * sizeof(int); // espaço dos vetores de valores e atributos

    if (inicio != nullptr) { // soma o espaço do vetor de inícios se ele estiver alocado
        bytes += static_cast<unsigned long long>(capacidade_linhas + 1) * sizeof(int);
    }

    return bytes;
}