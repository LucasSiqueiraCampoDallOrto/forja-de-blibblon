#include "metricas.h"
#include <cmath> 

// calcula poder, equilíbrio e sinergia, ordena todos os itens em cada métrica e transforma suas posições nos rankings em pontos para a recomendação

using namespace std;

// retorna o modificador do item naquele atributo ou zero se não existir
double ler_modificador(
    Matriz_modificadores& matriz,
    int id_item,
    int id_atributo
) {
    return matriz.consultar(id_item, id_atributo);
}

// calcula o poder somando todos os modificadores do item
double calcular_poder(
    Matriz_modificadores& matriz,
    int id_item,
    int qntd_atributo
) {
    if (qntd_atributo == 0) return 0.0; // sem atributos, o poder é zero

    Percurso p = matriz.percorrer_linha(id_item); // prepara a leitura dos modificadores do item
    Tripleta celula; // recebe um modificador por vez

    double soma = 0.0;

    while (p.proximo(celula)) {
        soma += celula.valor; // soma o modificador, incluindo valores negativos
    }

    return soma;
}

// calcula o equilíbrio: quanto menor a diferença entre os modificadores, maior o resultado
double calcular_equilibrio(
    Matriz_modificadores& matriz,
    int id_item,
    int qntd_atributo
) {
    if (qntd_atributo == 0) return 1.0; // sem atributos, o equilíbrio é definido como 1

    double media = calcular_poder(
        matriz, id_item, qntd_atributo
    ) / qntd_atributo; // divide a soma pelo total de atributos, incluindo os zeros

    Percurso p = matriz.percorrer_linha(id_item);
    Tripleta celula;

    double soma_desvios = 0.0; // soma das diferenças ao quadrado em relação à média
    int presentes = 0; // conta os atributos com modificador diferente de zero

    while (p.proximo(celula)) {
        double diferenca = celula.valor - media; // calcula a diferença entre o valor e a média

        soma_desvios += diferenca * diferenca;
        presentes++;
    }

    // inclui na conta os atributos sem célula, cujo modificador vale zero
    soma_desvios += (qntd_atributo - presentes) * media * media;

    return 1.0 / (
        1.0 + sqrt(soma_desvios / qntd_atributo) // calcula 1 dividido por 1 mais o desvio padrão
    );
}

// calcula o quanto os modificadores do item combinam com os totais da build
double calcular_sinergia(
    Matriz_modificadores& matriz,
    int id_item,
    int qntd_atributo,
    const double* build
) {
    if (qntd_atributo > 0 && build == nullptr) {
        throw "Build nao valida"; // faltou o vetor com os totais da build
    }

    double produto = 0.0; // soma dos produtos entre valores do mesmo atributo
    double quadrados_item = 0.0; // soma dos modificadores do item ao quadrado
    double quadrados_build = 0.0; // soma dos totais da build ao quadrado

    Percurso p = matriz.percorrer_linha(id_item);
    Tripleta celula;

    while (p.proximo(celula)) {
        double valor = celula.valor;

        produto += valor * build[celula.id_atributo]; // multiplica os valores do mesmo atributo
        quadrados_item += valor * valor;
    }

    for (int a = 0; a < qntd_atributo; a++) {
        quadrados_build += build[a] * build[a];
    }

    if (quadrados_item == 0.0 || quadrados_build == 0.0) {
        return 0.0; // evita dividir por zero quando o item ou a build só possui valores zero
    }

    double resultado = produto / (
        sqrt(quadrados_item) * sqrt(quadrados_build) // divide pelo produto dos comprimentos dos dois vetores
    );

    // corrige pequenos excessos de arredondamento para manter o resultado entre -1 e 1
    if (resultado > 1.0) return 1.0;
    if (resultado < -1.0) return -1.0;

    return resultado;
}

// coloca primeiro o maior valor; em caso de empate, coloca o menor ID
bool item_vem_antes(ItemOrdenado& a, ItemOrdenado& b) {
    if(a.valor_arredondado != b.valor_arredondado) {
        return a.valor_arredondado > b.valor_arredondado;
    }

    return a.id_item < b.id_item;
}

// ordena os itens com MergeSort: divide em metades, ordena cada metade e junta os resultados
void ordenar_itens(
    ItemOrdenado* itens,
    ItemOrdenado* auxiliar,
    int inicio,
    int fim
) {
    if (fim - inicio <= 1) {
        return; // zero ou um item já está ordenado
    }

    int meio = inicio + (fim - inicio) / 2;

    ordenar_itens(itens, auxiliar, inicio, meio); // ordena a metade esquerda
    ordenar_itens(itens, auxiliar, meio, fim); // ordena a metade direita

    int esquerda = inicio; // próximo item da metade esquerda
    int direita = meio; // próximo item da metade direita
    int destino = inicio; // próxima posição a preencher no vetor auxiliar

    while (esquerda < meio && direita < fim) { // compara os próximos itens das duas metades
        if (item_vem_antes(itens[esquerda], itens[direita])) {
            auxiliar[destino++] = itens[esquerda++]; // copia o item da esquerda e avança
        } else {
            auxiliar[destino++] = itens[direita++]; // copia o item da direita e avança
        }
    }

    while (esquerda < meio) {
        auxiliar[destino++] = itens[esquerda++]; // copia o que sobrou da metade esquerda
    }

    while (direita < fim) {
        auxiliar[destino++] = itens[direita++]; // copia o que sobrou da metade direita
    }

    for (int i = inicio; i < fim; i++) {
        itens[i] = auxiliar[i]; // devolve o trecho ordenado ao vetor original
    }
}

// cria um ranking para cada métrica e soma os pontos de cada item multiplicados pelos pesos
double* calcular_pontuacoes(Matriz_modificadores& matriz, int total_itens, int total_atributos, const double* build, const MetricaConsulta* metricas, int quantidade_metricas) {
    if (total_itens < 0 || total_atributos < 0 || quantidade_metricas < 0) {
        throw "Quantidade invalida"; // as quantidades não podem ser negativas
    }

    if (total_atributos > 0 && build == nullptr) {
        throw "Build invalida"; // faltou o vetor com os totais da build
    }

    if (quantidade_metricas > 0 && metricas == nullptr) {
        throw "Metricas invalidas"; // faltou o vetor com as métricas solicitadas
    }

    for (int m = 0; m < quantidade_metricas; m++) {
        TipoMetrica tipo = metricas[m].tipo;

        if (tipo != TipoMetrica::PODER &&tipo != TipoMetrica::EQUILIBRIO && tipo != TipoMetrica::SINERGIA) {
            throw "Metrica invalida"; // aceita somente as três métricas do sistema
        }
    }

    double* pontuacoes = nullptr; // guardará a pontuação final de cada item
    ItemOrdenado* ordem = nullptr; // guardará os itens para montar cada ranking
    ItemOrdenado* auxiliar = nullptr; // ajudará a ordenar os itens

    try {
        pontuacoes = new double[total_itens](); // começa com zero pontos para cada item
        ordem = new ItemOrdenado[total_itens];
        auxiliar = new ItemOrdenado[total_itens];

        for (int m = 0; m < quantidade_metricas; m++) { // processa uma métrica por vez
            for (int i = 0; i < total_itens; i++) { // calcula essa métrica para todos os itens
                double valor = 0.0;

                switch (metricas[m].tipo) {
                    case TipoMetrica::PODER:
                        valor = calcular_poder(matriz, i, total_atributos);
                        break;

                    case TipoMetrica::EQUILIBRIO:
                        valor = calcular_equilibrio(matriz, i, total_atributos);
                        break;

                    case TipoMetrica::SINERGIA:
                        valor = calcular_sinergia(matriz, i, total_atributos, build);
                        break;
                }

                ordem[i] = ItemOrdenado{i, round(valor * 1000000.0) / 1000000.0}; // guarda o ID e o valor arredondado para seis casas
            }

            ordenar_itens(ordem, auxiliar, 0, total_itens); // monta o ranking da métrica atual

            for (int posicao = 0; posicao < total_itens; posicao++) {
                int id_item = ordem[posicao].id_item; // identifica o item nessa posição
                int pontos = total_itens - posicao; // primeiro recebe N pontos, segundo N-1, até o último receber 1

                pontuacoes[id_item] += metricas[m].peso * pontos; // aplica o peso e soma à pontuação do item
            }
        }
    } catch (...) { // se ocorrer um erro, libera os vetores antes de repassá-lo
        delete[] pontuacoes;
        delete[] ordem;
        delete[] auxiliar;
        throw;
    }

    delete[] ordem; // libera os vetores usados apenas na ordenação
    delete[] auxiliar;

    return pontuacoes; // devolve as pontuações; quem receber deverá liberar o vetor com delete[]
}