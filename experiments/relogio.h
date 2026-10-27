#ifndef ANALISE_RELOGIO_H
#define ANALISE_RELOGIO_H
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>

// consulta o contador de tempo do Windows e sua frequência
struct Relogio {
    typedef long long time_point; // define o tipo usado para guardar uma leitura

    // retorna a leitura atual do contador
    static time_point now() {
        // consulta contador -> devolve leitura
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        return t.QuadPart;
    }

    // informa quantas unidades o contador marca por segundo
    static double frequencia() {
        // consulta frequência uma vez -> guarda -> devolve valor

        // inicializa f
        static const double f = []() {
            LARGE_INTEGER v;
            QueryPerformanceFrequency(&v);
            return static_cast<double>(v.QuadPart);
        }();

        return f; // reutiliza a frequência nas próximas chamadas
    }
};

// converte a diferença entre duas leituras em nanosegundos
inline double nanos(Relogio::time_point a, Relogio::time_point b) {
    // calcula diferença -> converte pela frequência -> devolve nanosegundos
    return static_cast<double>(b - a) * 1000000000.0 / Relogio::frequencia();
}

// informa o nome do relógio utilizado
inline const char* nome_relogio() {
    return "QueryPerformanceCounter";
}

// faz as primeiras consultas ao relógio antes das medições
inline void preparar_relogio() {
    // inicializa frequência -> consultar contador
    (void)Relogio::frequencia(); // chama a função e descarta o retorno
    (void)Relogio::now();
}

// usa o relógio da biblioteca C++ nos outros sistemas
#else

#include <chrono>

// dá o nome Relogio ao relógio que mede intervalos sem retroceder.
using Relogio = std::chrono::steady_clock;

// calcula o intervalo entre duas leituras em nanosegundos
inline double nanos(Relogio::time_point a, Relogio::time_point b) {
    // calcula diferença -> converte para nanosegundos -> devolver valor
    return std::chrono::duration<double, std::nano>(b - a).count();
}

// informa o nome do relógio utilizado
inline const char* nome_relogio() {
    return "std::chrono::steady_clock";
}

// faz uma primeira leitura antes de iniciar as medições
inline void preparar_relogio() {
    (void)Relogio::now(); // consulta o relógio e descarta o retorno
}

#endif
#endif