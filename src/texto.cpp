#include "texto.h"
#include <iostream> 

// guarda textos em um vetor de caracteres e permite copiar, comparar, ler e imprimir seu conteúdo

// cria um texto copiando os caracteres recebidos
Texto::Texto(const char* origem) : dados(nullptr), tamanho(0), capacidade(0) {
    while (origem[tamanho] != '\0') tamanho++; // conta os caracteres até o fim do texto
    reservar(tamanho + 1); // reserva espaço para o texto e o '\0' que marca seu fim

    for (std::size_t i = 0; i <= tamanho; i++) dados[i] = origem[i]; // copia os caracteres e o '\0'
}

// cria uma cópia de outro Texto, com seu próprio vetor de caracteres
Texto::Texto(const Texto& outro) : Texto(outro.c_str()) {
}

// permite copiar o conteúdo de outro Texto usando o sinal de igual
Texto& Texto::operator=(const Texto& outro) {
    if (this == &outro) return *this; // não precisa copiar se for o mesmo objeto

    reservar(outro.tamanho + 1); // garante espaço para o texto recebido

    for (std::size_t i = 0; i <= outro.tamanho; i++) dados[i] = outro.dados[i]; // copia o texto e seu terminador

    tamanho = outro.tamanho;
    return *this; // devolve o próprio objeto, permitindo atribuições como a = b = c
}

// libera o vetor de caracteres ao destruir o texto
Texto::~Texto() {
    delete[] dados;
}

// aumenta o vetor de caracteres quando falta espaço
void Texto::reservar(std::size_t minimo) {
    if (minimo <= capacidade) return; // já existe espaço suficiente

    std::size_t nova = capacidade ? capacidade : 16; // usa a capacidade atual ou começa com 16 posições
    while (nova < minimo) nova *= 2; // dobra até alcançar o espaço necessário

    char* vetor = new char[nova];

    if (dados != nullptr) {
        for (std::size_t i = 0; i <= tamanho; i++) vetor[i] = dados[i]; // preserva o texto existente e seu terminador
    }

    delete[] dados;
    dados = vetor; // passa a usar o novo vetor
    capacidade = nova;
}

// adiciona um caractere ao final do texto
void Texto::acrescentar(char caractere) {
    reservar(tamanho + 2); // garante espaço para o caractere e o '\0'
    dados[tamanho++] = caractere; // escreve o caractere e aumenta o tamanho
    dados[tamanho] = '\0'; // marca o novo fim do texto
}

// verifica se o conteúdo é igual à sequência de caracteres recebida
bool Texto::operator==(const char* outro) const {
    std::size_t i = 0;

    while (dados[i] != '\0' && outro[i] != '\0') {
        if (dados[i] != outro[i]) return false; // encontrou caracteres diferentes
        i++;
    }

    return dados[i] == outro[i]; // só são iguais se os dois textos terminarem juntos
}

// retorna um ponteiro para ler os caracteres do texto
const char* Texto::c_str() const {
    return dados;
}

// permite ler um nome ou comando sem espaços usando >>
std::istream& operator>>(std::istream& entrada, Texto& texto) {
    entrada >> std::ws; // ignora espaços e quebras de linha antes do texto

    if (!entrada.good()) {
        entrada.setstate(std::ios::failbit); // informa que não foi possível iniciar a leitura
        return entrada;
    }

    texto.tamanho = 0; // apaga o conteúdo anterior para receber outro texto
    texto.dados[0] = '\0';

    int c; // recebe um caractere ou o sinal de fim da entrada

    while ((c = entrada.peek()) != std::istream::traits_type::eof()) { // olha o próximo caractere sem retirá-lo
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f' || c == '\v') break; // para ao encontrar um separador

        entrada.get(); // retira da entrada o caractere examinado
        texto.acrescentar(static_cast<char>(c)); // adiciona o caractere ao texto
    }

    if (texto.tamanho == 0) entrada.setstate(std::ios::failbit); // informa falha se nenhum caractere foi lido

    return entrada; // permite continuar a leitura, como em cin >> nome >> comando
}

// permite imprimir o texto usando <<
std::ostream& operator<<(std::ostream& saida, const Texto& texto) {
    return saida << texto.c_str(); // escreve os caracteres e permite continuar a impressão
}