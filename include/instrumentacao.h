#ifndef TP_INSTRUMENTACAO_H
#define TP_INSTRUMENTACAO_H
#ifndef FORJA_CONTADORES
#define FORJA_CONTADORES 0
#endif

struct EstatisticasCatalogo { // conta as células percorridas e comparações nas buscas para analisar o desempenho do catálogo
    unsigned long long celulas_percorridas = 0; // qntd de células contadas durante os percursos.
    unsigned long long comparacoes_busca = 0; // qntd de comparações contadas durante as buscas
};

inline void registrar_contagem(unsigned long long& contador) { // aumenta o contador recebido em 1 quando a contagem está ativada
#if FORJA_CONTADORES
    contador++;
#else
    (void)contador; // n altera o contador
#endif
}
#endif