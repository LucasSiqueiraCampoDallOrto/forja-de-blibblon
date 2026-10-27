#ifndef FORJA_APOIO_EXPERIMENTOS_HPP
#define FORJA_APOIO_EXPERIMENTOS_HPP

#include "relogio.h"
#include "sistema_forja.h"
#include <cmath>
#include <stdexcept>

using namespace std;

// gera uma sequência de números para montar os experimentos
class Gerador {
    unsigned int estado; // guarda o valor usado para gerar o próximo número

public:
    // inicia o gerador com o valor recebido
    explicit Gerador(unsigned int s) : estado(s) {
    }

    // calcula e retorna o próximo número da sequência
    unsigned int proximo() {
        estado = estado ^ (estado << 13);
        estado = estado ^ (estado >> 17);
        estado = estado ^ (estado << 5);

        return estado;
    }

    // retorna um número de zero até limite menos um
    int inteiro(int limite) {
        return static_cast<int>(proximo() % limite);
    }
};

// identifica os três algoritmos de ordenação disponíveis
enum Algoritmo {
    MERGE,
    INSERCAO,
    HEAP
};

// retorna o nome do algoritmo escolhido
inline const char* nome_alg(Algoritmo a) {
    if (a == MERGE) {
        return "merge";
    } else if (a == INSERCAO) {
        return "insercao";
    } else {
        return "heap";
    }
}

// ordena os itens e registra o tempo e as comparações realizadas
class OrdenadorExperimental {
    ItemOrdenado* auxiliar; // vetor usado para juntar as partes do MergeSort

    unsigned long long comparacoes = 0; // total de comparações registradas
    double tempo_ns = 0; // tempo acumulado das ordenações em nanossegundos

    // verifica qual item deve aparecer primeiro
    bool antes(const ItemOrdenado& a, const ItemOrdenado& b) {
        registrar_contagem(comparacoes); // conta a comparação quando habilitado

        if (a.valor_arredondado != b.valor_arredondado) {
            return a.valor_arredondado > b.valor_arredondado;
        }

        return a.id_item < b.id_item; // em caso de empate, usa o menor id
    }

    // divide o trecho, ordena suas partes e depois junta os resultados
    void merge(ItemOrdenado* a, int inicio, int fim) {
        if (fim - inicio <= 1) {
            return; // um trecho com até um elemento já está ordenado
        }

        int meio = inicio + (fim - inicio) / 2;

        merge(a, inicio, meio); // ordena a primeira metade
        merge(a, meio, fim); // ordena a segunda metade

        int i = inicio; // posição de leitura da primeira metade
        int j = meio; // posição de leitura da segunda metade
        int k = inicio; // posição de escrita no vetor auxiliar

        while (i < meio && j < fim) {
            if (antes(a[i], a[j])) {
                auxiliar[k] = a[i];
                i = i + 1;
            } else {
                auxiliar[k] = a[j];
                j = j + 1;
            }

            k = k + 1;
        }

        // copia os elementos que sobraram na primeira metade
        while (i < meio) {
            auxiliar[k] = a[i];
            k = k + 1;
            i = i + 1;
        }

        // copia os elementos que sobraram na segunda metade
        while (j < fim) {
            auxiliar[k] = a[j];
            k = k + 1;
            j = j + 1;
        }

        // devolve o trecho ordenado ao vetor original
        for (i = inicio; i < fim; i = i + 1) {
            a[i] = auxiliar[i];
        }
    }

    // coloca cada item na posição correta entre os itens anteriores
    void insercao(ItemOrdenado* a, int n) {
        for (int i = 1; i < n; i = i + 1) {
            ItemOrdenado valor = a[i]; // guarda o item que será inserido
            int j = i - 1;

            while (j >= 0 && antes(valor, a[j])) {
                a[j + 1] = a[j]; // abre espaço deslocando o item anterior
                j = j - 1;
            }

            a[j + 1] = valor; // coloca o item na posição encontrada
        }
    }

    // ajusta o heap, deixando acima o item que deve aparecer depois
    void descer(ItemOrdenado* a, int n, int raiz) {
        while (2 * raiz + 1 < n) {
            int filho = 2 * raiz + 1; // começa pelo filho da esquerda

            // escolhe o filho que deve aparecer por último na ordenação
            if (filho + 1 < n && antes(a[filho], a[filho + 1])) {
                filho = filho + 1;
            }

            if (!antes(a[raiz], a[filho])) {
                return; // a raiz já está na posição adequada
            }

            // troca a raiz com o filho escolhido
            ItemOrdenado tmp = a[raiz];
            a[raiz] = a[filho];
            a[filho] = tmp;

            raiz = filho; // continua o ajuste na posição seguinte
        }
    }

    // monta o heap e leva seus elementos para as posições finais
    void heap(ItemOrdenado* a, int n) {
        // ajusta os nós que possuem filhos
        for (int i = n / 2 - 1; i >= 0; i = i - 1) {
            descer(a, n, i);
        }

        for (int fim = n - 1; fim > 0; fim = fim - 1) {
            // coloca o elemento da raiz no final do trecho atual
            ItemOrdenado tmp = a[0];
            a[0] = a[fim];
            a[fim] = tmp;

            descer(a, fim, 0); // reorganiza os elementos restantes
        }
    }

public:
    // reserva o vetor auxiliar usado durante as ordenações
    explicit OrdenadorExperimental(int n)
        : auxiliar(new ItemOrdenado[n]) {
    }

    // libera o vetor auxiliar
    ~OrdenadorExperimental() {
        delete[] auxiliar;
    }

    // impede cópias que compartilhariam o mesmo vetor
    OrdenadorExperimental(const OrdenadorExperimental&) = delete;
    OrdenadorExperimental& operator=(const OrdenadorExperimental&) = delete;

    // executa o algoritmo escolhido e acumula o tempo gasto
    void ordenar(ItemOrdenado* a, int n, Algoritmo alg) {
        Relogio::time_point t = Relogio::now();

        if (alg == MERGE) {
            merge(a, 0, n);
        } else if (alg == INSERCAO) {
            insercao(a, n);
        } else {
            heap(a, n);
        }

        tempo_ns = tempo_ns + nanos(t, Relogio::now());
    }

    // reinicia as medidas de comparação e tempo
    void zerar() {
        comparacoes = 0;
        tempo_ns = 0;
    }

    // retorna o total de comparações registradas
    unsigned long long get_comparacoes() const {
        return comparacoes;
    }

    // retorna o tempo acumulado das ordenações
    double get_tempo_ns() const {
        return tempo_ns;
    }
};

// compara o cálculo imediato da build com o cálculo feito na consulta
class BuildExperimental {
    Aventureiro aventureiro; // usa a pilha de equipamentos do aventureiro
    long long* totais; // guarda o total de cada atributo
    int atributos; // quantidade de atributos cadastrados
    bool imediata; // indica se os totais são atualizados ao equipar e desequipar
    int pecas = 0; // quantidade de peças atualmente equipadas

    // soma ou subtrai os modificadores de um item nos totais da build
    void acumular(Matriz_modificadores& matriz, int item, int sinal) {
        Percurso p = matriz.percorrer_linha(item);
        Tripleta c; // recebe cada modificador encontrado

        while (p.proximo(c)) {
            totais[c.id_atributo] =
                totais[c.id_atributo]
                + static_cast<long long>(sinal) * c.valor;
        }
    }

public:
    // cria o aventureiro de teste e um vetor de totais inicialmente zerado
    BuildExperimental(int t, bool agora)
        : aventureiro("Teste"),
          totais(new long long[t]()),
          atributos(t),
          imediata(agora) {
    }

    // libera o vetor de totais
    ~BuildExperimental() {
        delete[] totais;
    }

    // impede que duas builds compartilhem o mesmo vetor por cópia
    BuildExperimental(const BuildExperimental&) = delete;
    BuildExperimental& operator=(const BuildExperimental&) = delete;

    // empilha o item e atualiza os totais se a estratégia for imediata
    void equipar(Matriz_modificadores& matriz, int item) {
        aventureiro.equipamentos.empilhar(item);
        pecas = pecas + 1;

        if (imediata) {
            acumular(matriz, item, 1);
        }
    }

    // retira a última peça e subtrai seus valores na estratégia imediata
    void desequipar(Matriz_modificadores& matriz) {
        int item; // recebe o ID do item retirado

        if (aventureiro.equipamentos.desempilhar(item)) {
            pecas = pecas - 1;

            if (imediata) {
                acumular(matriz, item, -1);
            }
        }
    }

    // retorna os totais, recalculando-os quando a estratégia é sob demanda
    const long long* consultar(Matriz_modificadores& matriz) {
        if (!imediata) {
            for (int a = 0; a < atributos; a = a + 1) {
                totais[a] = 0;
            }

            const ItemEquipado* p = aventureiro.equipamentos.get_topo();

            while (p != nullptr) {
                acumular(matriz, p->id_item, 1);
                p = p->anterior;
            }
        }

        return totais;
    }

    // atualiza o total para cada ocorrência equipada do item alterado
    void alterar_modificador(int item, int atributo, long long delta) {
        if (!imediata || delta == 0) {
            return; // não precisa atualizar o vetor nesse caso
        }

        const ItemEquipado* p = aventureiro.equipamentos.get_topo();

        while (p != nullptr) {
            if (p->id_item == item) {
                totais[atributo] = totais[atributo] + delta;
            }

            p = p->anterior;
        }
    }

    // marca quais itens estão equipados para excluí-los das recomendações
    void marcar(bool* equipado) const {
        const ItemEquipado* p = aventureiro.equipamentos.get_topo();

        while (p != nullptr) {
            equipado[p->id_item] = true;
            p = p->anterior;
        }
    }

    // retorna a quantidade de peças equipadas
    int get_pecas() const {
        return pecas;
    }
};

// identifica os comandos medidos nos experimentos
enum Comando {
    CMD_M,
    CMD_F,
    CMD_Z,
    CMD_D,
    CMD_LI,
    CMD_LA,
    CMD_LH,
    CMD_Q,
    NUM_COMANDOS
};

// guarda o nome correspondente a cada comando
const char* nomes_comandos[NUM_COMANDOS] = {
    "M", "F", "Z", "D", "LI", "LA", "LH", "Q"
};

// guarda os dados necessários para executar uma operação experimental
struct Operacao {
    Comando tipo; // comando que será executado
    int item; // ID do item envolvido
    int atributo; // ID do atributo envolvido
    int valor; // modificador usado na alteração
    int heroi; // ID do aventureiro envolvido
};

// reúne os parâmetros usados para montar um cenário de teste
struct Config {
    const char* nome; // nome do cenário
    int itens; // quantidade de itens
    int atributos; // quantidade de atributos
    double densidade; // proporção desejada de células não nulas
    int build; // quantidade inicial de peças por aventureiro
    int fila; // quantidade inicial de encomendas
    int n; // quantidade de recomendações solicitadas
    int m; // quantidade de métricas usadas na recomendação
    int operacoes; // quantidade de operações do experimento
};

// guarda as medidas acumuladas de um tipo de comando
struct Medida {
    unsigned long long chamadas = 0; // quantidade de execuções
    double tempo_ns = 0; // tempo total em nanossegundos
    unsigned long long celulas = 0; // células percorridas registradas
    unsigned long long buscas = 0; // comparações de busca registradas
    unsigned long long comparacoes = 0; // comparações de ordenação registradas
};

// monta o cenário, executa os comandos e reúne as medidas dos experimentos
class MotorExperimental {
    int n_itens; // quantidade de itens do cenário
    int n_atributos; // quantidade de atributos do cenário

    static const int herois = 8; // quantidade de aventureiros usada nos testes

    bool imediata; // estratégia usada para manter os totais das builds
    BuildExperimental** builds; // vetor de ponteiros para as builds
    Fila fila; // fila global de encomendas do experimento

    ItemOrdenado* ordens; // guarda uma ordenação anterior para cada métrica
    ItemOrdenado* candidatos; // guarda os itens que podem ser recomendados
    double* pontos; // pontuação final de cada item
    double* build_double; // totais da build convertidos para double
    bool* equipado; // indica quais itens estão equipados

    Medida medidas[NUM_COMANDOS]; // medidas separadas por comando

    unsigned long long checksum = 1469598103934665603ULL; // assinatura dos resultados

    long long fila_acumulada = 0; // soma dos tamanhos observados da fila
    long long pecas_acumuladas = 0; // soma das quantidades observadas de peças

    unsigned long long memoria_pico = 0; // maior estimativa de memória observada
    unsigned long long memoria_catalogo_atual = 0; // estimativa atual do catálogo

    unsigned long long comparacoes_metricas = 0; // comparações ao ordenar as métricas
    double tempo_metricas_ns = 0; // tempo gasto nas ordenações das métricas

    // incorpora um resultado à assinatura usada para comparar as variantes
    void consumir(long long x) {
        checksum = checksum ^ static_cast<unsigned long long>(x);
        checksum = checksum * 1099511628211ULL;
    }

public:
    Matriz_modificadores matriz; // catálogo utilizado pelos experimentos
    OrdenadorExperimental ordenador; // executa e mede as ordenações

    // cria o catálogo, as builds, a fila e os vetores auxiliares do cenário
    MotorExperimental(
        const Config& c,
        char modo,
        bool agora,
        unsigned int seed
    )
        : n_itens(c.itens),
          n_atributos(c.atributos),
          imediata(agora),
          builds(new BuildExperimental*[herois]),
          ordens(new ItemOrdenado[3 * c.itens]),
          candidatos(new ItemOrdenado[c.itens]),
          pontos(new double[c.itens]),
          build_double(new double[c.atributos]),
          equipado(new bool[c.itens]),
          matriz(modo),
          ordenador(c.itens) {

        Gerador g(seed); // inicia a sequência usada para montar o cenário

        // prepara o catálogo antes de começar as medições
        matriz.registrar_itens(c.itens);
        matriz.registrar_atributos(c.atributos);

        for (int i = 0; i < c.itens; i = i + 1) {
            for (int a = 0; a < c.atributos; a = a + 1) {
                // decide se esse par item e atributo terá uma célula
                if (
                    g.inteiro(1000000)
                    < static_cast<int>(c.densidade * 1000000)
                ) {
                    int v = g.inteiro(19) - 9; // gera um valor entre -9 e 9

                    if (v == 0) {
                        v = 1; // as células iniciais precisam ser não nulas
                    }

                    matriz.atribuir(i, a, v);
                }
            }
        }

        // cria os aventureiros e seus equipamentos iniciais
        for (int h = 0; h < herois; h = h + 1) {
            builds[h] = new BuildExperimental(c.atributos, agora);

            for (int p = 0; p < c.build; p = p + 1) {
                builds[h]->equipar(matriz, g.inteiro(c.itens));
            }
        }

        // coloca as encomendas iniciais na fila
        for (int p = 0; p < c.fila; p = p + 1) {
            fila.enfileirar(g.inteiro(herois), g.inteiro(c.itens));
        }

        // começa cada ordenação com os itens em ordem de id
        for (int m = 0; m < 3; m = m + 1) {
            for (int i = 0; i < c.itens; i = i + 1) {
                ordens[m * c.itens + i] = {i, 0};
            }
        }

        matriz.zerar_estatisticas(); // desconsidera as contagens da preparação

        memoria_catalogo_atual = matriz.memoria_estimada();
        memoria_pico = memoria_estimada();
    }

    // libera as builds e os vetores criados pelo motor
    ~MotorExperimental() {
        for (int h = 0; h < herois; h = h + 1) {
            delete builds[h];
        }

        delete[] builds;
        delete[] ordens;
        delete[] candidatos;
        delete[] pontos;
        delete[] build_double;
        delete[] equipado;
    }

    // impede cópias que compartilhariam os mesmos objetos e vetores
    MotorExperimental(const MotorExperimental&) = delete;
    MotorExperimental& operator=(const MotorExperimental&) = delete;

    // calcula os pontos, exclui os equipados e registra os melhores candidatos
    void recomendar(int h, int n, int m, Algoritmo alg, bool reaproveitar) {
        const long long* b = builds[h]->consultar(matriz);

        // converte os totais para o tipo utilizado pelas métricas
        for (int a = 0; a < n_atributos; a = a + 1) {
            build_double[a] = static_cast<double>(b[a]);
        }

        // prepara os pontos e as marcações para uma nova consulta
        for (int i = 0; i < n_itens; i = i + 1) {
            pontos[i] = 0;
            equipado[i] = false;
        }

        unsigned long long comparacoes_antes = ordenador.get_comparacoes();
        double tempo_antes = ordenador.get_tempo_ns();

        // usa poder, depois equilíbrio e, por último, sinergia
        for (int metrica = 0; metrica < m; metrica = metrica + 1) {
            ItemOrdenado* ordem = ordens + metrica * n_itens;

            for (int pos = 0; pos < n_itens; pos = pos + 1) {
                int i;

                if (reaproveitar) {
                    i = ordem[pos].id_item; // mantém a ordem da consulta anterior
                } else {
                    i = pos; // recomeça pela ordem dos id
                }

                double valor;

                if (metrica == 0) {
                    valor = calcular_poder(matriz, i, n_atributos);
                } else if (metrica == 1) {
                    valor = calcular_equilibrio(matriz, i, n_atributos);
                } else {
                    valor = calcular_sinergia(
                        matriz, i, n_atributos, build_double
                    );
                }

                // guarda a métrica arredondada para seis casas decimais
                ordem[pos] = {
                    i,
                    round(valor * 1000000.0) / 1000000.0
                };
            }

            ordenador.ordenar(ordem, n_itens, alg);

            // soma os pontos das posições com pesos fixos 1, 2 e 3
            for (int pos = 0; pos < n_itens; pos = pos + 1) {
                pontos[ordem[pos].id_item] =
                    pontos[ordem[pos].id_item]
                    + (metrica + 1) * (n_itens - pos);
            }
        }

        // registra somente as ordenações das métricas desta consulta
        comparacoes_metricas =
            comparacoes_metricas
            + ordenador.get_comparacoes() - comparacoes_antes;

        tempo_metricas_ns =
            tempo_metricas_ns
            + ordenador.get_tempo_ns() - tempo_antes;

        builds[h]->marcar(equipado);

        int qtd = 0; // quantidade de itens disponíveis para recomendar

        for (int i = 0; i < n_itens; i = i + 1) {
            if (!equipado[i]) {
                candidatos[qtd] = {i, pontos[i]};
                qtd = qtd + 1;
            }
        }

        ordenador.ordenar(candidatos, qtd, alg);

        if (n > qtd) {
            n = qtd; // limita a resposta aos candidatos existentes
        }

        consumir(n); // registra a quantidade de recomendações

        // registra os id e as pontuações dos melhores candidatos
        for (int i = 0; i < n; i = i + 1) {
            consumir(candidatos[i].id_item);
            consumir(
                static_cast<long long>(
                    candidatos[i].valor_arredondado * 100
                )
            );
        }
    }

    // executa um comando e acumula suas medidas de tempo e trabalho
    void executar(
        const Operacao& op,
        int n,
        int m,
        Algoritmo alg = MERGE,
        bool reaproveitar = false
    ) {
        EstatisticasCatalogo antes_stats = matriz.get_estatisticas();
        unsigned long long antes_comparacoes = ordenador.get_comparacoes();

        Relogio::time_point inicio = Relogio::now();

        if (op.tipo == CMD_M) {
            int anterior = matriz.consultar(op.item, op.atributo);

            matriz.atribuir(op.item, op.atributo, op.valor);

            // corrige as builds imediatas que possuem o item alterado
            if (imediata) {
                for (int h = 0; h < herois; h = h + 1) {
                    builds[h]->alterar_modificador(
                        op.item,
                        op.atributo,
                        static_cast<long long>(op.valor) - anterior
                    );
                }
            }
        } else if (op.tipo == CMD_F) {
            fila.enfileirar(op.heroi, op.item);

        } else if (op.tipo == CMD_Z) {
            int h;
            int item;

            if (fila.desenfileirar(h, item)) {
                builds[h]->equipar(matriz, item);
                consumir(h);
                consumir(item);
            }
        } else if (op.tipo == CMD_D) {
            builds[op.heroi]->desequipar(matriz);

        } else if (op.tipo == CMD_LI || op.tipo == CMD_LA) {
            Percurso p;

            if (op.tipo == CMD_LI) {
                p = matriz.percorrer_linha(op.item);
            } else {
                p = matriz.percorrer_coluna(op.atributo);
            }

            Tripleta c;

            // registra cada célula encontrada no percurso
            while (p.proximo(c)) {
                consumir(c.id_item);
                consumir(c.id_atributo);
                consumir(c.valor);
            }
        } else if (op.tipo == CMD_LH) {
            const long long* b = builds[op.heroi]->consultar(matriz);

            for (int a = 0; a < n_atributos; a = a + 1) {
                consumir(b[a]);
            }
        } else {
            recomendar(op.heroi, n, m, alg, reaproveitar);
        }

        double tempo = nanos(inicio, Relogio::now());

        EstatisticasCatalogo depois_stats = matriz.get_estatisticas();
        Medida& r = medidas[op.tipo]; // acessa as medidas desse comando

        r.chamadas = r.chamadas + 1;
        r.tempo_ns = r.tempo_ns + tempo;

        // acrescenta o trabalho registrado durante essa operação
        r.celulas =
            r.celulas
            + depois_stats.celulas_percorridas
            - antes_stats.celulas_percorridas;

        r.buscas =
            r.buscas
            + depois_stats.comparacoes_busca
            - antes_stats.comparacoes_busca;

        r.comparacoes =
            r.comparacoes
            + ordenador.get_comparacoes()
            - antes_comparacoes;

        fila_acumulada = fila_acumulada + fila.get_quantidade();

        // soma as quantidades de peças observadas nos aventureiros
        for (int h = 0; h < herois; h = h + 1) {
            pecas_acumuladas =
                pecas_acumuladas + builds[h]->get_pecas();
        }

        // atualiza a estimativa do catálogo apenas quando ocorre M
        if (op.tipo == CMD_M) {
            memoria_catalogo_atual = matriz.memoria_estimada();
        }

        unsigned long long atual = memoria_estimada();

        if (atual > memoria_pico) {
            memoria_pico = atual;
        }
    }

    // retorna as medidas acumuladas de um comando
    const Medida& get_medida(int comando) const {
        return medidas[comando];
    }

    // retorna a assinatura acumulada dos resultados
    unsigned long long get_checksum() const {
        return checksum;
    }

    // retorna as comparações feitas nas ordenações das métricas
    unsigned long long get_comparacoes_metricas() const {
        return comparacoes_metricas;
    }

    // retorna o tempo gasto nas ordenações das métricas
    double get_tempo_metricas_ns() const {
        return tempo_metricas_ns;
    }

    // calcula o tamanho médio da fila observado após as operações
    double get_fila_media(int ops) const {
        return static_cast<double>(fila_acumulada) / ops;
    }

    // calcula a quantidade média de peças por aventureiro após as operações
    double get_build_media(int ops) const {
        return static_cast<double>(pecas_acumuladas) / (ops * herois);
    }

    // estima os bytes ocupados pelo motor e pelas estruturas acompanhadas
    unsigned long long memoria_estimada() const {
        // evita contar o objeto da matriz duas vezes
        unsigned long long bytes =
            sizeof(*this) + memoria_catalogo_atual - sizeof(matriz);

        // inclui os ponteiros, as builds e seus vetores de totais
        bytes =
            bytes
            + herois * (
                sizeof(BuildExperimental*)
                + sizeof(BuildExperimental)
                + n_atributos * sizeof(long long)
            );

        // inclui os vetores usados nas recomendações
        bytes =
            bytes
            + 5ULL * n_itens * sizeof(ItemOrdenado)
            + n_itens * (sizeof(double) + sizeof(bool))
            + n_atributos * sizeof(double);

        // inclui os nós das encomendas que ainda estão na fila
        bytes =
            bytes
            + static_cast<unsigned long long>(fila.get_quantidade())
            * sizeof(Encomenda);

        // inclui os nós dos equipamentos de cada aventureiro
        for (int h = 0; h < herois; h = h + 1) {
            bytes =
                bytes
                + static_cast<unsigned long long>(builds[h]->get_pecas())
                * sizeof(ItemEquipado);
        }

        return bytes;
    }

    // retorna a maior estimativa de memória registrada
    unsigned long long get_memoria_pico() const {
        return memoria_pico;
    }

    // incorpora os totais das builds à assinatura para comparar estratégias
    void validar_builds() {
        for (int h = 0; h < herois; h = h + 1) {
            const long long* b = builds[h]->consultar(matriz);

            for (int a = 0; a < n_atributos; a = a + 1) {
                consumir(b[a]);
            }
        }
    }
};

#endif