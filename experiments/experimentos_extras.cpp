#include "sistema_forja.h"
#include "relogio.h"
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

#include <fstream>
#include <cstdlib>
#include <cstdint>
#include <atomic>

// retorna os segundos decorridos desde a primeira chamada desta função
static double agora() {
    static const Relogio::time_point origem = Relogio::now();

    return nanos(origem, Relogio::now()) * 0.000000001;
}

static unsigned int estado; // guarda o estado atual do gerador de números

// gera o próximo número da sequência a partir do estado anterior
static unsigned int sorteio() {
    estado = 1664525U * estado + 1013904223U;

    return estado;
}

// recebe os resultados para que sejam utilizados durante o experimento
static volatile long long impedir_eliminacao = 0;

// Guarda o tamanho do catálogo e a densidade desejada para um cenário.
struct Configuracao {
    int itens; // quantidade de itens
    int atributos; // quantidade de atributos
    int densidade_por_mil; // proporção desejada de células a cada mil posições
};

// cadastra as dimensões da matriz e insere os modificadores não nulos
static void carregar(
    Matriz_modificadores& m,
    const int* dados,
    int n,
    int a
) {
    m.registrar_itens(n);
    m.registrar_atributos(a);

    for (int i = 0; i < n; i = i + 1) {
        for (int j = 0; j < a; j = j + 1) {
            if (dados[i * a + j] != 0) {
                m.atribuir(i, j, dados[i * a + j]);
            }
        }
    }
}

// calcula uma soma de verificação usando os IDs e os valores das células
static long long assinatura(Matriz_modificadores& m, int n) {
    long long soma = 0;
    Tripleta t; // recebe cada célula encontrada

    for (int i = 0; i < n; i = i + 1) {
        Percurso p = m.percorrer_linha(i);

        while (p.proximo(t)) {
            soma =
                soma
                + (t.id_item + 1LL) * (t.id_atributo + 1LL) * t.valor;
        }
    }

    return soma;
}

// monta os cenários, mede as operações em L/T/C e grava os resultados em CSV
int main(int argc, char** argv) {
    preparar_relogio(); // prepara o relógio antes das medições

    string prefixo; // início dos nomes dos arquivos de resultados

    if (argc > 1) {
        prefixo = argv[1]; // usa o nome informado no terminal
    } else {
        prefixo = "tempos_extras"; // usa o nome padrão
    }

    int repeticoes; // quantidade de vezes que cada cenário será executado

    if (argc > 2) {
        repeticoes = std::atoi(argv[2]);
    } else {
        repeticoes = 5;
    }

    if (repeticoes < 1) {
        cerr << "Repeticoes deve ser positivo.\n";
        return 1;
    }

#if FORJA_CONTADORES
    // impede medir tempos com os incrementos dos contadores habilitados
    cerr << "Compile com -DFORJA_CONTADORES=0 para medir tempos.\n";
    return 1;
#endif

    // cria os arquivos que receberão as medidas
    std::ofstream operacoes((prefixo + "_operacoes.csv").c_str());
    std::ofstream migracoes((prefixo + "_migracoes.csv").c_str());

    if (!operacoes || !migracoes) {
        cerr << "Nao foi possivel criar os CSVs.\n";
        return 1;
    }

    // escreve os nomes das colunas do arquivo de operações
    operacoes
        << "config,repeticao,semente,modo,itens,atributos,"
        << "celulas_iniciais,densidade_real,insercoes,leituras_linhas,"
        << "percurso_us_linha,insercao_us,substituicao_us,remocao_us,"
        << "memoria_representacao_bytes,memoria_indice_bytes\n";

    // escreve os nomes das colunas do arquivo de migrações
    migracoes
        << "config,repeticao,semente,origem,destino,itens,"
        << "atributos,celulas,tempo_us\n";

    // grava os números reais com seis casas decimais
    operacoes << std::fixed << std::setprecision(6);
    migracoes << std::fixed << std::setprecision(6);

    // define os cinco cenários: itens, atributos e densidade por mil
    const Configuracao configs[] = {
        {256, 128, 20},
        {256, 128, 200},
        {1024, 128, 20},
        {1024, 512, 20},
        {1024, 512, 100}
    };

    const char modos[] = "LTC"; // representações que serão comparadas

    for (int c = 0; c < 5; c = c + 1) {
        int n = configs[c].itens;
        int a = configs[c].atributos;
        int limite = n * a; // quantidade de pares possíveis de item e atributo

        for (
            int repeticao = 0;
            repeticao < repeticoes;
            repeticao = repeticao + 1
        ) {
            // define uma sequência de números para esta configuração e repetição
            unsigned int semente =
                20261022U + static_cast<unsigned int>(c * 100 + repeticao);

            estado = semente;

            int* dados = new int[limite](); // começa todos os modificadores em zero
            bool* escolhida = new bool[limite](); // marca posições já escolhidas

            int celulas = 0; // quantidade inicial de modificadores não nulos

            // preenche algumas posições conforme a densidade do cenário
            for (int p = 0; p < limite; p = p + 1) {
                if (
                    sorteio() % 1000
                    < static_cast<unsigned int>(configs[c].densidade_por_mil)
                ) {
                    dados[p] = static_cast<int>(sorteio() % 19) + 1;
                    celulas = celulas + 1;
                }
            }

            const int quantidade_insercoes = 256;

            // guarda as células que serão inseridas durante a medição
            Tripleta* novas = new Tripleta[quantidade_insercoes];

            for (int j = 0; j < quantidade_insercoes; j = j + 1) {
                int p;

                // procura uma posição vazia que ainda não foi escolhida
                do {
                    p = sorteio() % limite;
                } while (dados[p] != 0 || escolhida[p]);

                escolhida[p] = true;

                // transforma a posição do vetor em item e atributo
                novas[j] = Tripleta{p / a, p % a, 3};
            }

            delete[] escolhida; // as posições escolhidas já estão guardadas em novas

            // alterna qual modo é executado primeiro em cada repetição
            for (int ordem = 0; ordem < 3; ordem = ordem + 1) {
                char modo = modos[(ordem + repeticao) % 3];

                Matriz_modificadores m(modo);
                carregar(m, dados, n, a);

                // percorre o catálogo antes da medição e guarda sua assinatura
                long long original = assinatura(m, n);

                // registra a memória inicial da representação e do índice de faixas
                unsigned long long bytes_m = m.memoria_representacao_estimada();
                unsigned long long bytes_i = m.memoria_indice_estimada();

                const int varreduras = 80; // quantidade de percursos completos
                long long soma = 0;

                double inicio = agora();

                for (int k = 0; k < varreduras; k = k + 1) {
                    // coloca uma barreira de compilação entre as varreduras
                    std::atomic_signal_fence(std::memory_order_seq_cst);

                    soma = soma + assinatura(m, n);
                }

                // calcula o tempo médio de percurso por linha, em microssegundos
                double percurso =
                    (agora() - inicio) * 1000000.0 / (varreduras * n);

                impedir_eliminacao = soma; // utiliza o resultado dos percursos

                if (soma != original * varreduras) {
                    cerr << "Falha no percurso.\n";
                    return 1;
                }

                // mede a inserção das células escolhidas
                inicio = agora();

                for (int j = 0; j < quantidade_insercoes; j = j + 1) {
                    m.atribuir(
                        novas[j].id_item,
                        novas[j].id_atributo,
                        novas[j].valor
                    );
                }

                double inserir =
                    (agora() - inicio) * 1000000.0 / quantidade_insercoes;

                // mede a substituição dos valores dessas células por 5
                inicio = agora();

                for (int j = 0; j < quantidade_insercoes; j = j + 1) {
                    m.atribuir(
                        novas[j].id_item,
                        novas[j].id_atributo,
                        5
                    );
                }

                double substituir =
                    (agora() - inicio) * 1000000.0 / quantidade_insercoes;

                // mede a remoção das células inseridas durante o teste
                inicio = agora();

                for (int j = 0; j < quantidade_insercoes; j = j + 1) {
                    m.remover(
                        novas[j].id_item,
                        novas[j].id_atributo
                    );
                }

                double remover =
                    (agora() - inicio) * 1000000.0 / quantidade_insercoes;

                // verifica a quantidade e a assinatura após desfazer as inserções
                if (
                    m.get_quantidade() != celulas
                    || assinatura(m, n) != original
                ) {
                    cerr << "Falha apos atualizacoes.\n";
                    return 1;
                }

                // grava os parâmetros e as medidas das operações
                operacoes
                    << c << ','
                    << repeticao << ','
                    << semente << ','
                    << modo << ','
                    << n << ','
                    << a << ','
                    << celulas << ','
                    << static_cast<double>(celulas) / limite << ','
                    << quantidade_insercoes << ','
                    << varreduras * n << ','
                    << percurso << ','
                    << inserir << ','
                    << substituir << ','
                    << remover << ','
                    << bytes_m << ','
                    << bytes_i << '\n';

                // mede a migração do modo atual para cada um dos outros modos
                for (int destino = 0; destino < 3; destino = destino + 1) {
                    if (modos[destino] != modo) {
                        Matriz_modificadores copia(modo);

                        // prepara uma matriz com os dados iniciais antes de medir
                        carregar(copia, dados, n, a);

                        inicio = agora();

                        copia.mudar_modo(modos[destino]);

                        double tempo = (agora() - inicio) * 1000000.0;

                        // confere a quantidade e a assinatura após a migração
                        if (
                            copia.get_quantidade() != celulas
                            || assinatura(copia, n) != original
                        ) {
                            cerr << "Falha na migracao.\n";
                            return 1;
                        }

                        // grava os dados da migração e seu tempo em microssegundos
                        migracoes
                            << c << ','
                            << repeticao << ','
                            << semente << ','
                            << modo << ','
                            << modos[destino] << ','
                            << n << ','
                            << a << ','
                            << celulas << ','
                            << tempo << '\n';
                    }
                }
            }

            // libera os vetores usados nesta repetição
            delete[] novas;
            delete[] dados;
        }

        cout << "Configuracao " << c + 1 << "/5 concluida.\n";
    }

    // informa quais arquivos de resultados foram gerados
    cout
        << "OK: gerados "
        << prefixo << "_operacoes.csv e "
        << prefixo << "_migracoes.csv.\n";
}