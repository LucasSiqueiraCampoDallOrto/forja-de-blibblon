#include "indice_faixas.h"

// mantém os modificadores organizados por atributo e valor para responder as consultas lf, incluindo os itens com valor zero quando a faixa permitir e ordenando a resposta pelo id dos itens

// aumenta o vetor de pares quando falta espaço
void ColunaFaixa::reservar_pares() {
    if (quantidade < capacidade) return; // ainda cabe mais um par

    int nova = capacidade ? capacidade * 2 : 4; // dobra a capacidade ou começa com quatro posições
    ResultadoFaixa* vetor = new ResultadoFaixa[nova];

    for (int i = 0; i < quantidade; i++) vetor[i] = pares[i]; // copia os pares existentes

    delete[] pares; pares = vetor; capacidade = nova; // substitui o vetor antigo
}

// aumenta o vetor que guarda os grupos de IDs com modificador zero
void ColunaFaixa::reservar_zeros() {
    if (intervalos < capacidade_zeros) return; // ainda cabe mais um grupo

    int nova = capacidade_zeros ? capacidade_zeros * 2 : 4;
    IntervaloZero* vetor = new IntervaloZero[nova];

    for (int i = 0; i < intervalos; i++) vetor[i] = zeros[i]; // copia os grupos existentes

    delete[] zeros; zeros = vetor; capacidade_zeros = nova;
}

// busca onde o par item/valor está ou deveria ser inserido
int ColunaFaixa::posicao_par(int item, int valor) const {
    int e = 0, d = quantidade; // limites esquerdo e direito da busca

    while (e < d) {
        int m = e + (d - e) / 2; // posição do meio

        // compara primeiro o valor e, em caso de empate, o ID do item
        if (pares[m].valor < valor ||
            (pares[m].valor == valor && pares[m].id_item < item)) e = m + 1;
        else d = m;
    }

    return e;
}

// remove um grupo de zeros do vetor
void ColunaFaixa::apagar_intervalo(int p) {
    for (int j = p; j + 1 < intervalos; j++) zeros[j] = zeros[j + 1]; // fecha o espaço movendo os seguintes
    intervalos--;
}

// retira o item dos grupos de zeros porque ele ganhou um modificador não zero
void ColunaFaixa::retirar_zero(int item) {
    int e = 0, d = intervalos;

    while (e < d) { // procura o grupo que contém o item
        int m = e + (d - e) / 2;
        if (zeros[m].ultimo < item) e = m + 1; else d = m;
    }

    // o item deveria estar em um grupo de zeros
    if (e == intervalos || zeros[e].primeiro > item) throw "Indice de zeros inconsistente";

    int primeiro = zeros[e].primeiro, ultimo = zeros[e].ultimo; // guarda os extremos do grupo

    if (primeiro == ultimo) apagar_intervalo(e); // o grupo tinha somente esse item
    else if (item == primeiro) zeros[e].primeiro++; // retira o primeiro ID do grupo
    else if (item == ultimo) zeros[e].ultimo--; // retira o último ID do grupo
    else { // o item está no meio, então divide o grupo em dois
        reservar_zeros();

        for (int j = intervalos; j > e + 1; j--) zeros[j] = zeros[j - 1]; // abre espaço para outro grupo

        zeros[e].ultimo = item - 1; // o grupo da esquerda termina antes do item
        zeros[e + 1] = IntervaloZero{item + 1, ultimo}; // o grupo da direita começa depois do item
        intervalos++;
    }
}

// recoloca o item nos grupos de zeros porque seu modificador foi removido
void ColunaFaixa::devolver_zero(int item) {
    int e = 0, d = intervalos;

    while (e < d) { // procura onde o item deve entrar
        int m = e + (d - e) / 2;
        if (zeros[m].primeiro < item) e = m + 1; else d = m;
    }

    bool junta_esquerda = e > 0 && zeros[e - 1].ultimo == item - 1; // verifica se pode unir ao grupo anterior
    bool junta_direita = e < intervalos && zeros[e].primeiro == item + 1; // verifica se pode unir ao próximo grupo

    if (junta_esquerda && junta_direita) {
        zeros[e - 1].ultimo = zeros[e].ultimo; apagar_intervalo(e); // une os dois grupos incluindo o item
    } else if (junta_esquerda) zeros[e - 1].ultimo = item; // aumenta o grupo anterior
    else if (junta_direita) zeros[e].primeiro = item; // aumenta o próximo grupo
    else {
        reservar_zeros();

        for (int j = intervalos; j > e; j--) zeros[j] = zeros[j - 1]; // abre espaço para um novo grupo

        zeros[e] = IntervaloZero{item, item}; intervalos++; // cria um grupo com somente esse item
    }
}

// busca o primeiro valor >= ao recebido ou o primeiro > quando superior é true
int ColunaFaixa::limite(int valor, bool superior, EstatisticasFaixa& est) const {
    int e = 0, d = quantidade;

    while (e < d) {
        int m = e + (d - e) / 2;
        registrar_contagem(est.comparacoes_limites); // conta a comparação se habilitado

        // no limite superior, também passa pelos valores iguais ao recebido
        if (pares[m].valor < valor || (superior && pares[m].valor == valor)) e = m + 1;
        else d = m;
    }

    return e; // posição que delimita o começo ou o fim da faixa
}

// verifica se o ID de a é maior que o de b
bool ColunaFaixa::maior_id(const ResultadoFaixa& a, const ResultadoFaixa& b, EstatisticasFaixa& est) {
    registrar_contagem(est.comparacoes_ordenacao); // conta a comparação se habilitado
    return a.id_item > b.id_item;
}

// ajusta uma parte do heap para manter o maior ID acima dos filhos
void ColunaFaixa::descer_heap(ResultadoFaixa* v, int n, int raiz, EstatisticasFaixa& est) {
    while (raiz < n / 2) {
        int filho = 2 * raiz + 1; // posição do filho esquerdo

        if (filho + 1 < n && maior_id(v[filho + 1], v[filho], est)) filho++; // escolhe o filho direito se ele tiver ID maior

        if (!maior_id(v[filho], v[raiz], est)) return; // o pai já tem ID maior ou igual ao dos filhos

        ResultadoFaixa aux = v[raiz]; v[raiz] = v[filho]; v[filho] = aux; // troca o pai com o maior filho
        raiz = filho; // continua ajustando na posição para onde o pai desceu
    }
}

// libera os vetores deste atributo
ColunaFaixa::~ColunaFaixa() {
    delete[] pares; delete[] zeros;
}

// registra os novos itens com modificador inicialmente zero neste atributo
void ColunaFaixa::garantir_itens(int total) {
    if (total <= total_itens) return; // os itens já são conhecidos

    if (intervalos > 0 && zeros[intervalos - 1].ultimo == total_itens - 1)
        zeros[intervalos - 1].ultimo = total - 1; // inclui os novos IDs no último grupo
    else {
        reservar_zeros();
        zeros[intervalos++] = IntervaloZero{total_itens, total - 1}; // cria um grupo com os novos IDs
    }

    total_itens = total;
}

// atualiza os pares e os grupos de zeros quando o modificador de um item muda
void ColunaFaixa::alterar(int item, int antigo, int novo) {
    if (antigo == novo) return; // o valor não mudou

    if (antigo != 0) { // retira o par com o valor antigo
        int p = posicao_par(item, antigo);

        if (p == quantidade || pares[p].id_item != item || pares[p].valor != antigo)
            throw "Indice de faixa inconsistente"; // o par esperado não foi encontrado

        for (int j = p; j + 1 < quantidade; j++) pares[j] = pares[j + 1]; // fecha o espaço do par removido
        quantidade--;
    }

    if (novo != 0) { // insere o par com o novo valor
        reservar_pares();
        int p = posicao_par(item, novo); // encontra a posição correta na ordem por valor e ID

        for (int j = quantidade; j > p; j--) pares[j] = pares[j - 1]; // abre espaço para o novo par

        pares[p] = ResultadoFaixa{item, novo}; quantidade++;
    }

    if (antigo == 0) retirar_zero(item); // o item deixou de ter modificador zero
    else if (novo == 0) devolver_zero(item); // o item passou a ter modificador zero
}

// retorna os itens da faixa em ordem crescente de ID, incluindo zeros quando necessário
ResultadoFaixa* ColunaFaixa::consultar(int minimo, int maximo, int& total, EstatisticasFaixa& est) const {
    total = 0;

    if (minimo > maximo) return nullptr; // a faixa está invertida

    int comeco = limite(minimo, false, est), fim = limite(maximo, true, est); // encontra os limites da faixa por busca binária
    bool inclui_zero = minimo <= 0 && maximo >= 0; // verifica se zero pertence à faixa

    total = fim - comeco + (inclui_zero ? total_itens - quantidade : 0); // conta os resultados, incluindo os zeros quando necessário

    if (total == 0) return nullptr; // nenhum item atende à consulta

    ResultadoFaixa* resposta = new ResultadoFaixa[total]; // cria o vetor da resposta
    int p = 0; // próxima posição livre na resposta

    for (int j = comeco; j < fim; j++) {
        resposta[p++] = pares[j]; registrar_contagem(est.resultados_copiados); // copia os pares encontrados e contabiliza
    }

    if (inclui_zero) {
        for (int j = 0; j < intervalos; j++) // percorre os grupos de zeros
            for (int item = zeros[j].primeiro; item <= zeros[j].ultimo; item++) {
                resposta[p++] = ResultadoFaixa{item, 0}; // inclui cada item do grupo com valor zero
                registrar_contagem(est.resultados_copiados);
            }
    }

    // prepara o heap para ordenar os resultados por ID
    for (int j = total / 2; j > 0; j--) descer_heap(resposta, total, j - 1, est);

    for (int j = total - 1; j > 0; j--) {
        ResultadoFaixa aux = resposta[0]; resposta[0] = resposta[j]; resposta[j] = aux; // coloca o maior ID no fim da parte ainda não ordenada
        descer_heap(resposta, j, 0, est); // ajusta o heap restante
    }

    return resposta; // quem receber o vetor deverá liberá-lo com delete[]
}

// estima os bytes reservados para os pares e os grupos de zeros
unsigned long long ColunaFaixa::memoria_dinamica() const {
    return static_cast<unsigned long long>(capacidade) * sizeof(ResultadoFaixa)
         + static_cast<unsigned long long>(capacidade_zeros) * sizeof(IntervaloZero);
}

// libera as estruturas dos atributos e o vetor que aponta para elas
IndiceFaixas::~IndiceFaixas() {
    for (int a = 0; a < atributos; a++) delete colunas[a];
    delete[] colunas;
}

// retorna a quantidade de itens conhecidos pelo índice
int IndiceFaixas::total_itens() const {
    return itens;
}

// informa a todos os atributos que novos itens foram cadastrados
void IndiceFaixas::garantir_itens(int total) {
    if (total <= itens) return;

    for (int a = 0; a < atributos; a++) colunas[a]->garantir_itens(total); // registra os novos itens em cada atributo
    itens = total;
}

// cria as estruturas dos atributos que ainda não estão registrados
void IndiceFaixas::garantir_atributos(int total) {
    if (total <= atributos) return;

    if (total > capacidade) { // aumenta o vetor de ponteiros se faltar espaço
        int nova = capacidade ? capacidade : 4;
        while (nova < total) nova *= 2;

        ColunaFaixa** vetor = new ColunaFaixa*[nova];

        for (int a = 0; a < atributos; a++) vetor[a] = colunas[a]; // copia os endereços das estruturas existentes

        delete[] colunas; colunas = vetor; capacidade = nova; // substitui apenas o vetor de ponteiros
    }

    while (atributos < total) {
        colunas[atributos] = new ColunaFaixa; // cria a estrutura do novo atributo
        colunas[atributos]->garantir_itens(itens); // registra os itens existentes, inicialmente com valor zero
        atributos++;
    }
}

// encaminha a mudança de um modificador para a estrutura do atributo correspondente
void IndiceFaixas::alterar(int item, int atributo, int antigo, int novo) {
    garantir_itens(item + 1); garantir_atributos(atributo + 1); // garante que o item e o atributo sejam conhecidos
    colunas[atributo]->alterar(item, antigo, novo);
}

// consulta a faixa no atributo escolhido e informa quantos resultados foram encontrados
ResultadoFaixa* IndiceFaixas::consultar(int atributo, int minimo, int maximo, int& total) {
    if (atributo < 0 || atributo >= atributos) { total = 0; return nullptr; } // atributo não registrado

    return colunas[atributo]->consultar(minimo, maximo, total, estatisticas);
}

// retorna uma cópia dos contadores das consultas LF
EstatisticasFaixa IndiceFaixas::get_estatisticas() const {
    return estatisticas;
}

// zera os contadores sem apagar os dados do índice
void IndiceFaixas::zerar_estatisticas() {
    estatisticas = EstatisticasFaixa{};
}

// estima os bytes alocados para o vetor de colunas e as estruturas dos atributos
unsigned long long IndiceFaixas::memoria_dinamica() const {
    unsigned long long bytes = static_cast<unsigned long long>(capacidade) * sizeof(ColunaFaixa*); // espaço do vetor de ponteiros

    for (int a = 0; a < atributos; a++) bytes += sizeof(ColunaFaixa) + colunas[a]->memoria_dinamica(); // soma cada objeto e seus vetores

    return bytes;
}