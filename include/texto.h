#ifndef TP_TEXTO_H
#define TP_TEXTO_H

#include <cstddef>
#include <iosfwd>

class Texto {
private:

    char* dados; // aponta para o vetor de caracteres
    std::size_t tamanho; // quantidade de caracteres 
    std::size_t capacidade; // quantos caracteres cabem no vetor

    void reservar(std::size_t minimo); // aumenta o vetor de caracteres se necessario
    void acrescentar(char caractere); // escreve um caractere 
  
public:

    Texto(const char* origem = ""); // pega os caracteres recebidos e copia para um vetor
    Texto(const Texto& outro); // cria um novo Texto com uma cópia dos caracteres de outro Texto
    Texto& operator=(const Texto& outro); // permite usar = para copiar o conteúdo de um Texto para outro que já existe
   
    ~Texto(); // libera o vetor de caracteres 
   
    bool operator==(const char* outro) const; // compara o texto com uma sequencia de caracteres
   
    const char* c_str() const; // permite acessar os caracteres do texto
    friend std::istream& operator>>(std::istream& entrada, Texto& texto); // lê um texto do fluxo de entrada e o armazena no objeto recebido
    friend std::ostream& operator<<(std::ostream& saida, const Texto& texto); // escreve os caracteres do objeto no fluxo de saída

};

#endif
