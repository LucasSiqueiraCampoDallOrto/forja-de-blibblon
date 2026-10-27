#include "relogio.h"
#include <iostream>
#include <iomanip>

// mede o intervalo entre duas leituras seguidas do relógio e mostra informações do ambiente
int main() {
    preparar_relogio(); // prepara o relógio antes das medições

    double soma = 0; // soma dos intervalos medidos
    double menor_positivo = 0; // menor intervalo maior que zero encontrado
    int zeros = 0; // quantidade de medições que retornaram zero

    const int amostras = 100000; // quantidade de medições realizadas

    for (int i = 0; i < amostras; i = i + 1) {
        Relogio::time_point a = Relogio::now(); // faz a primeira leitura
        Relogio::time_point b = Relogio::now(); // faz a segunda leitura logo depois

        double dt = nanos(a, b); // calcula o intervalo em nanossegundos
        soma = soma + dt; // acrescenta o intervalo à soma

        if (dt == 0) {
            zeros = zeros + 1; // conta uma medição sem diferença detectada
        } else if (menor_positivo == 0 || dt < menor_positivo) {
            menor_positivo = dt; // guarda o primeiro intervalo positivo ou um menor
        }
    }

    // mostra os resultados e informações sobre a compilação
    std::cout << std::setprecision(12) // usa até 12 algarismos significativos
        << "Relogio: " << nome_relogio() << '\n'
        << "Intervalo vazio medio (ns): " << soma / amostras << '\n'
        << "Menor intervalo positivo observado (ns): " << menor_positivo << '\n'
        << "Intervalos zero: " << zeros << '/' << amostras << '\n'
        << "Compilador: " << __VERSION__ << '\n'
        << "Tamanho de ponteiro (bytes): " << sizeof(void*) << '\n';
}