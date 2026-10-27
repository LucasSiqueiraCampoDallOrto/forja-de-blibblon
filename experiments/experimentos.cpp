#include "apoio_experimentos.h"
#include "produtos_auxiliares.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

// guarda um resultado que o compilador não deve descartar
volatile unsigned long long resultado_observavel = 0;

// define os cenários usados nos experimentos
// ordem: nome, itens, atributos, densidade, build, fila, n, m e operações
const Config configuracoes[] = {
    {"base", 256, 128, 0.05, 16, 32, 5, 3, 1200},
    {"itens64", 64, 128, 0.05, 16, 32, 5, 3, 1200},
    {"itens1024", 1024, 128, 0.05, 16, 32, 5, 3, 1200},
    {"atributos32", 256, 32, 0.05, 16, 32, 5, 3, 1200},
    {"atributos512", 256, 512, 0.05, 16, 32, 5, 3, 1200},
    {"densidade005", 256, 128, 0.005, 16, 32, 5, 3, 1200},
    {"densidade25", 256, 128, 0.25, 16, 32, 5, 3, 1200},
    {"build1", 256, 128, 0.05, 1, 32, 5, 3, 1200},
    {"build64", 256, 128, 0.05, 64, 32, 5, 3, 1200},
    {"fila0", 256, 128, 0.05, 16, 0, 5, 3, 1200},
    {"fila256", 256, 128, 0.05, 16, 256, 5, 3, 1200},
    {"n1", 256, 128, 0.05, 16, 32, 1, 3, 1200},
    {"n50", 256, 128, 0.05, 16, 32, 50, 3, 1200},
    {"m1", 256, 128, 0.05, 16, 32, 5, 1, 1200},
    {"m2", 256, 128, 0.05, 16, 32, 5, 2, 1200}
};

// cria os comandos de um perfil, mantendo a proporção de cada tipo
Operacao* gerar_carga(const Config& c, int perfil, unsigned int seed) {
    // escolhe proporções -> cria comandos -> embaralha -> devolve vetor

    // quantidades de cada comando em um bloco de 100 operações
    const int atualizacoes[8] = {30, 20, 20, 20, 2, 2, 3, 3};
    const int consultas[8] = {4, 2, 2, 2, 20, 20, 25, 25};

    const int* pesos;

    if (perfil == 0) {
        pesos = atualizacoes;
    } else {
        pesos = consultas;
    }

    Operacao* ops = new Operacao[c.operacoes]; // cria o vetor de comandos
    Gerador g(seed); // inicia o gerador com a semente recebida

    // monta um bloco de 100 comandos por vez
    for (int inicio = 0; inicio < c.operacoes; inicio = inicio + 100) {
        int pos = inicio;

        for (int tipo = 0; tipo < 8; tipo = tipo + 1) {
            for (int j = 0; j < pesos[tipo]; j = j + 1) {
                int v = g.inteiro(19) - 9; // gera um valor entre -9 e 9

                if (g.inteiro(5) == 0) {
                    v = 0; // inclui atribuições de zero na carga
                }

                // guarda o comando e os dados usados durante sua execução
                ops[pos] = {
                    static_cast<Comando>(tipo),
                    g.inteiro(c.itens),
                    g.inteiro(c.atributos),
                    v,
                    g.inteiro(8)
                };

                pos = pos + 1;
            }
        }

        // embaralha o bloco sem mudar a quantidade de cada comando
        for (int j = 99; j > 0; j = j - 1) {
            int k = g.inteiro(j + 1);

            Operacao t = ops[inicio + j];
            ops[inicio + j] = ops[inicio + k];
            ops[inicio + k] = t;
        }
    }

    return ops;
}

// confere operações básicas das builds e dos algoritmos de ordenação
void validar_basico() {
    // compara builds -> altera e migra matriz -> confere ordenações

    const char modos[3] = {'L', 'T', 'C'};

    for (int indice = 0; indice < 3; indice = indice + 1) {
        char modo = modos[indice];

        Matriz_modificadores c(modo);
        c.atribuir(0, 0, 5);
        c.atribuir(1, 1, -3);

        BuildExperimental a(3, false); // calcula a build sob demanda
        BuildExperimental b(3, true); // mantém a build atualizada

        // equipa os mesmos itens nas duas estratégias, incluindo uma repetição
        a.equipar(c, 0);
        b.equipar(c, 0);
        a.equipar(c, 0);
        b.equipar(c, 0);
        a.equipar(c, 1);
        b.equipar(c, 1);

        const long long* x = a.consultar(c);
        const long long* y = b.consultar(c);

        for (int i = 0; i < 3; i = i + 1) {
            assert(x[i] == y[i]); // confere os totais de cada atributo
        }

        c.atribuir(0, 0, 0); // remove o modificador do catálogo
        b.alterar_modificador(0, 0, -5); // informa a diferença à build imediata

        x = a.consultar(c);
        y = b.consultar(c);

        for (int i = 0; i < 3; i = i + 1) {
            assert(x[i] == y[i]);
        }

        // troca a representação antes de testar a retirada de equipamento
        if (modo == 'L') {
            c.mudar_modo('C');
        } else {
            c.mudar_modo('L');
        }

        a.desequipar(c);
        b.desequipar(c);

        x = a.consultar(c);
        y = b.consultar(c);

        for (int i = 0; i < 3; i = i + 1) {
            assert(x[i] == y[i]);
        }
    }

    OrdenadorExperimental o(6);

    // confere se os três algoritmos produzem a ordem esperada
    for (int alg = 0; alg < 3; alg = alg + 1) {
        ItemOrdenado a[6] = {
            {4, 2}, {1, 3}, {3, 3}, {0, -1}, {2, 3}, {5, 2}
        };

        int esperado[6] = {1, 2, 3, 4, 5, 0};

        o.ordenar(a, 6, static_cast<Algoritmo>(alg));

        for (int i = 0; i < 6; i = i + 1) {
            assert(a[i].id_item == esperado[i]);
        }
    }
}

// compara os modos e as estratégias de build em diferentes perfis de comandos
void perfis(const string& prefixo, int repeticoes) {
    // gera carga -> executa variantes -> compara resultados -> salva medidas

    ofstream out(prefixo + "_perfis.csv");

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV");
    }

    // escreve os nomes das colunas do arquivo
    out << "cenario,perfil,rep,seed,itens,atributos,densidade_inicial,densidade_final,build_inicial,build_media,fila_inicial,fila_media,n,m,modo,estrategia,comando,chamadas,tempo_medio_ns,celulas_media,buscas_media,comparacoes_media,tempo_total_operacoes_ns,tempo_loop_ns,memoria_matriz_bytes,memoria_representacao_bytes,memoria_indice_bytes,memoria_motor_pico_bytes,checksum\n";

    for (const Config& c : configuracoes) {
        for (int perfil = 0; perfil < 2; perfil = perfil + 1) {
            for (int rep = 0; rep < repeticoes; rep = rep + 1) {
                unsigned int seed = 773 + rep * 97;

                const char* nome_perfil;

                if (perfil == 0) {
                    nome_perfil = "atualizacoes";
                } else {
                    nome_perfil = "consultas";
                }

                if (rep == 0) {
                    cerr << "  " << c.nome << ": " << nome_perfil << '\n';
                }

                Operacao* ops = gerar_carga(c, perfil, seed + 1);
                unsigned long long referencia = 0;

                // testa três modos, cada um com duas estratégias de build
                for (int variante = 0; variante < 6; variante = variante + 1) {
                    int v = (variante + rep) % 6; // alterna a ordem dos testes

                    const char modos[3] = {'L', 'T', 'C'};
                    char modo = modos[v / 2];
                    bool imediata = v % 2;

                    MotorExperimental motor(c, modo, imediata, seed);

                    double densidade_inicial =
                        static_cast<double>(motor.matriz.get_quantidade()) /
                        (c.itens * c.atributos);

                    Relogio::time_point inicio = Relogio::now();

                    // executa a mesma sequência de comandos em cada variante
                    for (int j = 0; j < c.operacoes; j = j + 1) {
                        motor.executar(ops[j], c.n, c.m);
                    }

                    double total_loop = nanos(inicio, Relogio::now());

                    motor.validar_builds();

                    // usa o primeiro resultado como referência para os demais
                    if (variante == 0) {
                        referencia = motor.get_checksum();
                    } else if (referencia != motor.get_checksum()) {
                        throw runtime_error(
                            "Divergencia de estado entre variantes nos perfis"
                        );
                    }

                    resultado_observavel = motor.get_checksum();

                    double total = 0;

                    // soma o tempo registrado para todos os comandos
                    for (int cmd = 0; cmd < NUM_COMANDOS; cmd = cmd + 1) {
                        total = total + motor.get_medida(cmd).tempo_ns;
                    }

                    const char* estrategia;

                    if (imediata) {
                        estrategia = "imediata";
                    } else {
                        estrategia = "demanda";
                    }

                    // grava uma linha de medidas para cada tipo de comando
                    for (int cmd = 0; cmd < NUM_COMANDOS; cmd = cmd + 1) {
                        const Medida& r = motor.get_medida(cmd);

                        out << setprecision(12)
                            << c.nome << ','
                            << nome_perfil << ','
                            << rep << ','
                            << seed << ','
                            << c.itens << ','
                            << c.atributos << ','
                            << densidade_inicial << ','
                            << static_cast<double>(motor.matriz.get_quantidade()) /
                               (c.itens * c.atributos) << ','
                            << c.build << ','
                            << motor.get_build_media(c.operacoes) << ','
                            << c.fila << ','
                            << motor.get_fila_media(c.operacoes) << ','
                            << c.n << ','
                            << c.m << ','
                            << modo << ','
                            << estrategia << ','
                            << nomes_comandos[cmd] << ','
                            << r.chamadas << ','
                            << r.tempo_ns / r.chamadas << ','
                            << static_cast<double>(r.celulas) / r.chamadas << ','
                            << static_cast<double>(r.buscas) / r.chamadas << ','
                            << static_cast<double>(r.comparacoes) / r.chamadas << ','
                            << total << ','
                            << total_loop << ','
                            << motor.matriz.memoria_estimada() << ','
                            << motor.matriz.memoria_representacao_estimada() << ','
                            << motor.matriz.memoria_indice_estimada() << ','
                            << motor.get_memoria_pico() << ','
                            << motor.get_checksum() << '\n';
                    }
                }

                delete[] ops; // libera a sequência de comandos
            }
        }
    }
}

// calcula um número de verificação a partir das células da matriz
unsigned long long assinatura(Matriz_modificadores& m, int itens) {
    // percorre itens -> ler células -> combina os dados -> devolve assinatura

    unsigned long long h = 1;

    for (int i = 0; i < itens; i = i + 1) {
        Percurso p = m.percorrer_linha(i);
        Tripleta c;

        while (p.proximo(c)) {
            h = h * 1099511628211ULL
                + c.id_item * 31
                + c.id_atributo * 17
                + c.valor;
        }
    }

    return h;
}

// mede o tempo e a memória das migrações entre L, T e C
void migracoes(const string& prefixo, int repeticoes) {
    // cria matriz -> registra estado -> migra -> confere e salva medidas

    ofstream out(prefixo + "_migracoes.csv");

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV");
    }

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV de migracoes");
    }

    out << "cenario,rep,seed,itens,atributos,celulas,direcao,tempo_ns,memoria_antes_bytes,memoria_depois_bytes\n";

    // cada posição representa uma direção de migração
    const char origens[6] = {'L', 'L', 'T', 'T', 'C', 'C'};
    const char destinos[6] = {'T', 'C', 'L', 'C', 'L', 'T'};

    for (const Config& c : configuracoes) {
        for (int rep = 0; rep < repeticoes; rep = rep + 1) {
            for (int ordem = 0; ordem < 6; ordem = ordem + 1) {
                int d = (ordem + rep) % 6;
                unsigned int seed = 773 + rep * 97;

                // cada direção começa com uma nova matriz contendo os mesmos dados
                MotorExperimental motor(c, origens[d], false, seed);

                unsigned long long esperado = assinatura(motor.matriz, c.itens);
                unsigned long long memoria = motor.matriz.memoria_estimada();

                Relogio::time_point t = Relogio::now();
                motor.matriz.mudar_modo(destinos[d]);
                double tempo = nanos(t, Relogio::now());

                if (esperado != assinatura(motor.matriz, c.itens)) {
                    throw runtime_error("Migracao alterou celulas");
                }

                out << c.nome << ','
                    << rep << ','
                    << seed << ','
                    << c.itens << ','
                    << c.atributos << ','
                    << motor.matriz.get_quantidade() << ','
                    << origens[d] << '_' << destinos[d] << ','
                    << tempo << ','
                    << memoria << ','
                    << motor.matriz.memoria_estimada() << '\n';
            }
        }
    }
}

// compara os algoritmos de ordenação usados nas recomendações
void ordenacoes(const string& prefixo, int repeticoes) {
    // prepara catálogo -> executa consultas e alterações -> compara e salva

    ofstream out(prefixo + "_ordenacoes.csv");

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV");
    }

    out << "itens,atributos,densidade,rep,modo,alteracoes_por_q,reutiliza,algoritmo,consultas,q_medio_ns,ordenacoes_medio_ns,metricas_ordenacao_medio_ns,comparacoes_por_q,comparacoes_metricas_por_q,checksum\n";

    for (int tamanho : {128, 512}) {
        for (char modo : {'L', 'T', 'C'}) {
            for (int alteracoes : {0, 1, 16}) {
                for (int rep = 0; rep < repeticoes; rep = rep + 1) {
                    Config c = {
                        "sort", tamanho, 128, 0.05, 16, 32, 5, 3, 1200
                    };

                    unsigned long long referencia = 0;

                    // compara três algoritmos, reutilizando ou não a ordem anterior
                    for (int variante = 0; variante < 6; variante = variante + 1) {
                        int v = (variante + rep) % 6;
                        Algoritmo alg = static_cast<Algoritmo>(v % 3);
                        bool reutiliza = v / 3;

                        MotorExperimental motor(c, modo, false, 773 + rep * 97);
                        Gerador g(991 + rep * 97);

                        // faz a primeira consulta para preparar as estruturas e a ordem
                        motor.executar({CMD_Q, 0, 0, 0, 0}, 5, 3, alg, reutiliza);

                        Medida anterior = motor.get_medida(CMD_Q);
                        double tempo_antes = motor.ordenador.get_tempo_ns();
                        double metricas_antes = motor.get_tempo_metricas_ns();
                        unsigned long long comp_antes =
                            motor.get_comparacoes_metricas();

                        // mede 40 consultas, com alterações entre elas
                        for (int q = 0; q < 40; q = q + 1) {
                            for (int j = 0; j < alteracoes; j = j + 1) {
                                motor.executar(
                                    {
                                        CMD_M,
                                        g.inteiro(tamanho),
                                        g.inteiro(128),
                                        g.inteiro(19) - 9,
                                        0
                                    },
                                    5,
                                    3,
                                    alg,
                                    reutiliza
                                );
                            }

                            motor.executar(
                                {CMD_Q, 0, 0, 0, 0}, 5, 3, alg, reutiliza
                            );
                        }

                        if (variante == 0) {
                            referencia = motor.get_checksum();
                        } else if (referencia != motor.get_checksum()) {
                            throw runtime_error(
                                "Ordenadores produziram recomendacoes diferentes"
                            );
                        }

                        const Medida& r = motor.get_medida(CMD_Q);

                        // desconta a consulta inicial antes de calcular as médias
                        out << tamanho << ",128,0.05,"
                            << rep << ','
                            << modo << ','
                            << alteracoes << ','
                            << reutiliza << ','
                            << nome_alg(alg) << ",40,"
                            << (r.tempo_ns - anterior.tempo_ns) / 40 << ','
                            << (motor.ordenador.get_tempo_ns() - tempo_antes) / 40 << ','
                            << (motor.get_tempo_metricas_ns() - metricas_antes) / 40 << ','
                            << static_cast<double>(
                                   r.comparacoes - anterior.comparacoes
                               ) / 40 << ','
                            << static_cast<double>(
                                   motor.get_comparacoes_metricas() - comp_antes
                               ) / 40 << ','
                            << motor.get_checksum() << '\n';
                    }
                }
            }
        }
    }
}

// compara o produto interno com vetor auxiliar e com intercalação de linhas
void produtos(const string& prefixo, int repeticoes) {
    // escolhe pares -> confere produtos -> mede as estratégias -> salva

    ofstream out(prefixo + "_produtos.csv");

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV");
    }

    out << "atributos,densidade_solicitada,densidade_real,modo,rep,algoritmo,pares,tempo_medio_ns,celulas_por_par,memoria_auxiliar_bytes,checksum\n";

    for (int t : {64, 512, 4096}) {
        for (double densidade : {0.002, 0.02, 0.2, 0.8}) {
            for (char modo : {'L', 'T', 'C'}) {
                Config c = {"dot", 64, t, densidade, 0, 0, 1, 1, 1200};
                MotorExperimental motor(c, modo, false, 773);

                double* auxiliar = new double[t]; // vetor da estratégia densa

                int a[256];
                int b[256];
                Gerador g(991);

                // escolhe os pares de itens que serão comparados
                for (int k = 0; k < 256; k = k + 1) {
                    a[k] = g.inteiro(64);
                    b[k] = g.inteiro(64);
                }

                double esperado = 0;

                // confere se as duas estratégias calculam os mesmos produtos
                for (int k = 0; k < 256; k = k + 1) {
                    double x = produto_denso(
                        motor.matriz, a[k], b[k], auxiliar, t
                    );

                    double y = motor.matriz.produto_interno(a[k], b[k]);

                    if (x != y) {
                        throw runtime_error("Produtos internos diferentes");
                    }

                    esperado = esperado + x;
                }

                for (int rep = 0; rep < repeticoes; rep = rep + 1) {
                    for (int v = 0; v < 2; v = v + 1) {
                        int alg = (v + rep) % 2; // alterna a ordem das estratégias

                        motor.matriz.zerar_estatisticas();
                        double checksum = 0;

                        Relogio::time_point inicio = Relogio::now();

                        for (int k = 0; k < 256; k = k + 1) {
                            if (alg == 0) {
                                checksum = checksum + produto_denso(
                                    motor.matriz, a[k], b[k], auxiliar, t
                                );
                            } else {
                                checksum = checksum +
                                    motor.matriz.produto_interno(a[k], b[k]);
                            }
                        }

                        double tempo = nanos(inicio, Relogio::now());

                        if (checksum != esperado) {
                            throw runtime_error("Produto perdeu resultado");
                        }

                        resultado_observavel = static_cast<unsigned long long>(
                            static_cast<long long>(checksum)
                        );

                        const char* nome_algoritmo;
                        std::size_t memoria_auxiliar;

                        if (alg == 0) {
                            nome_algoritmo = "denso";
                            memoria_auxiliar = t * sizeof(double);
                        } else {
                            nome_algoritmo = "intercalado";
                            memoria_auxiliar = 0;
                        }

                        out << t << ','
                            << densidade << ','
                            << static_cast<double>(motor.matriz.get_quantidade()) /
                               (64 * t) << ','
                            << modo << ','
                            << rep << ','
                            << nome_algoritmo << ",256,"
                            << tempo / 256 << ','
                            << static_cast<double>(
                                   motor.matriz.get_estatisticas().celulas_percorridas
                               ) / 256 << ','
                            << memoria_auxiliar << ','
                            << checksum << '\n';
                    }
                }

                delete[] auxiliar;
            }
        }
    }
}

// compara as estratégias de build conforme a frequência de consultas aumenta
void limiar(const string& prefixo, int repeticoes) {
    // varia frequência -> equipa e consulta -> compara estratégias -> salva

    ofstream out(prefixo + "_limiar.csv");

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV");
    }

    out << "modo,consultas_porcento,rep,estrategia,operacoes,consultas,alteracoes_equipamento,tempo_total_ns,checksum\n";

    Config c = {"limiar", 256, 128, 0.05, 16, 0, 5, 3, 1200};

    for (char modo : {'L', 'T', 'C'}) {
        MotorExperimental motor(c, modo, false, 773);

        for (int porcento : {0, 1, 2, 5, 10, 25, 50, 75, 90}) {
            for (int rep = 0; rep < repeticoes; rep = rep + 1) {
                unsigned long long referencia = 0;

                for (int v = 0; v < 2; v = v + 1) {
                    bool imediata = (v + rep) % 2;
                    BuildExperimental build(128, imediata);

                    // começa com os mesmos 16 itens equipados
                    for (int p = 0; p < 16; p = p + 1) {
                        build.equipar(motor.matriz, p);
                    }

                    Gerador g(991 + rep * 97);
                    unsigned long long checksum = 0;

                    int atualizacoes = 0;
                    int consultas = 0;
                    bool colocar = true;

                    Relogio::time_point inicio = Relogio::now();

                    for (int op = 0; op < 50000; op = op + 1) {
                        if (g.inteiro(100) < porcento) {
                            const long long* b = build.consultar(motor.matriz);

                            // usa os totais da build para conferir o resultado
                            for (int a = 0; a < 128; a = a + 1) {
                                checksum = checksum +
                                    static_cast<unsigned long long>(b[a]);
                            }

                            consultas = consultas + 1;
                        } else {
                            // alterna entre colocar e retirar um equipamento
                            if (colocar) {
                                build.equipar(motor.matriz, g.inteiro(256));
                            } else {
                                build.desequipar(motor.matriz);
                            }

                            colocar = !colocar;
                            atualizacoes = atualizacoes + 1;
                        }
                    }

                    double tempo = nanos(inicio, Relogio::now());

                    if (v == 0) {
                        referencia = checksum;
                    } else if (referencia != checksum) {
                        throw runtime_error("Limiar: resultados diferentes");
                    }

                    resultado_observavel = checksum;

                    const char* estrategia;

                    if (imediata) {
                        estrategia = "imediata";
                    } else {
                        estrategia = "demanda";
                    }

                    out << modo << ','
                        << porcento << ','
                        << rep << ','
                        << estrategia << ",50000,"
                        << consultas << ','
                        << atualizacoes << ','
                        << tempo << ','
                        << checksum << '\n';
                }
            }
        }
    }
}

// mede a consulta LF real, incluindo a criação e a ordenação dos resultados
void faixas(const string& prefixo, int repeticoes) {
    // preenche catálogo -> confere faixas -> repete consultas -> salva medidas

    ofstream out(prefixo + "_faixas.csv");

    if (!out) {
        throw runtime_error("Nao foi possivel criar CSV de LF");
    }

    out << "itens,atributos,densidade_solicitada,densidade_real,modo,rep,seed,minimo,maximo,consultas,resultados_por_consulta,tempo_medio_ns,comparacoes_limites_por_consulta,comparacoes_ordenacao_por_consulta,resultados_copiados_por_consulta,memoria_indice_bytes\n";

    // testa faixas com zero, valores positivos e nenhum resultado
    const int limites[4][2] = {
        {0, 0},
        {-1, 1},
        {2, 4},
        {100, 200}
    };

    for (int n : {256, 4096}) {
        for (double densidade : {0.02, 0.5}) {
            for (int rep = 0; rep < repeticoes; rep = rep + 1) {
                const char modos[] = "LTC";

                for (int ordem = 0; ordem < 3; ordem = ordem + 1) {
                    char modo = modos[(ordem + rep) % 3];
                    unsigned int seed = 773 + rep * 97;

                    Gerador g(seed);
                    Matriz_modificadores matriz(modo);

                    matriz.registrar_itens(n);
                    matriz.registrar_atributos(32);

                    // guarda os valores do atributo zero para conferir as respostas
                    int* referencia = new int[n]();

                    for (int i = 0; i < n; i = i + 1) {
                        for (int a = 0; a < 32; a = a + 1) {
                            if (g.inteiro(1000000) <
                                static_cast<int>(densidade * 1000000)) {

                                int valor = g.inteiro(19) - 9;

                                if (valor == 0) {
                                    valor = 1; // garante que a célula gerada exista
                                }

                                matriz.atribuir(i, a, valor);

                                if (a == 0) {
                                    referencia[i] = valor;
                                }
                            }
                        }
                    }

                    for (int f = 0; f < 4; f = f + 1) {
                        int minimo = limites[f][0];
                        int maximo = limites[f][1];
                        int total = 0;
                        int esperado = 0;

                        ResultadoFaixa* r = matriz.consultar_faixa(
                            0, minimo, maximo, total
                        );

                        // confere os itens, os valores e a ordem da resposta
                        for (int i = 0; i < n; i = i + 1) {
                            if (minimo <= referencia[i] &&
                                referencia[i] <= maximo) {

                                if (esperado >= total ||
                                    r[esperado].id_item != i ||
                                    r[esperado].valor != referencia[i]) {

                                    throw runtime_error(
                                        "LF divergiu da referencia densa"
                                    );
                                }

                                esperado = esperado + 1;
                            }
                        }

                        if (esperado != total) {
                            throw runtime_error("Quantidade incorreta em LF");
                        }

                        delete[] r;
                        matriz.zerar_estatisticas();

                        unsigned long long checksum = 0;
                        const int consultas = 64;

                        Relogio::time_point inicio = Relogio::now();

                        // repete a mesma consulta para calcular seu tempo médio
                        for (int k = 0; k < consultas; k = k + 1) {
                            r = matriz.consultar_faixa(
                                0, minimo, maximo, total
                            );

                            checksum = checksum + total;

                            for (int j = 0; j < total; j = j + 1) {
                                checksum = checksum * 31 +
                                    static_cast<unsigned long long>(
                                        r[j].id_item + r[j].valor
                                    );
                            }

                            delete[] r; // libera a resposta de cada consulta
                        }

                        double tempo = nanos(inicio, Relogio::now());

                        resultado_observavel = checksum;

                        EstatisticasFaixa est = matriz.get_estatisticas_faixa();

                        out << n << ",32,"
                            << densidade << ','
                            << static_cast<double>(matriz.get_quantidade()) /
                               (n * 32) << ','
                            << modo << ','
                            << rep << ','
                            << seed << ','
                            << minimo << ','
                            << maximo << ','
                            << consultas << ','
                            << esperado << ','
                            << tempo / consultas << ','
                            << static_cast<double>(est.comparacoes_limites) /
                               consultas << ','
                            << static_cast<double>(est.comparacoes_ordenacao) /
                               consultas << ','
                            << static_cast<double>(est.resultados_copiados) /
                               consultas << ','
                            << matriz.memoria_indice_estimada() << '\n';
                    }

                    delete[] referencia;
                }
            }
        }
    }
}

// executa as validações e os experimentos, salvando as medidas em arquivos CSV
int main(int argc, char** argv) {
    // fluxo: ler opções -> preparar relógio -> validar -> medir -> salvar resultados.

    string prefixo;

    if (argc > 1) {
        prefixo = argv[1]; // usa o prefixo informado no terminal
    } else {
        prefixo = "resultados";
    }

    int repeticoes;

    if (argc > 2) {
        repeticoes = std::atoi(argv[2]); // converte o argumento para inteiro
    } else {
        repeticoes = 5;
    }

    if (repeticoes < 1) {
        cerr << "Numero de repeticoes invalido.\n";
        return 1;
    }

    preparar_relogio();

    cerr << "Relogio: " << nome_relogio()
         << "; contadores=" << FORJA_CONTADORES << "\n";

    validar_basico();

    cerr << "Validacao inicial OK. Perfis...\n";
    perfis(prefixo, repeticoes);

    cerr << "Migracoes...\n";
    migracoes(prefixo, repeticoes);

    cerr << "Ordenacoes...\n";
    ordenacoes(prefixo, repeticoes);

    cerr << "Produtos...\n";
    produtos(prefixo, repeticoes);

    cerr << "Limiar...\n";
    limiar(prefixo, repeticoes);

    cerr << "Consultas LF...\n";
    faixas(prefixo, repeticoes);

    cerr << "Concluido; todas as comparacoes de resultados passaram.\n";
}