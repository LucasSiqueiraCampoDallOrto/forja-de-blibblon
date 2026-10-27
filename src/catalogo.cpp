#include "catalogo.h"

// atualiza o índice de lf quando um modificador muda
void Matriz_modificadores::registrar_alteracao(int item, int atributo, int antigo, int novo) {
    if (manter_indice) indice_faixas.alterar(item, atributo, antigo, novo);
}

// impede usar id negativo
void Matriz_modificadores::posicao_valida(int id_item, int id_atributo) {
    if(id_item < 0 || id_atributo < 0) throw "Posicao invalida";
}

// aumenta os vetores de cabeças para acomodar a linha e a coluna necessárias
void Matriz_modificadores::expandir_cabecas(int id_item, int id_atributo) {
    if(id_item >= capacidade_linha) { // verifica se falta espaço para a linha
        int nova_capacidade_linha = id_item + 1;
        Celula** novas_linhas = new Celula*[nova_capacidade_linha](); // cria ponteiros inicialmente nulos

        for(int i = 0; i < capacidade_linha; i++) {
            novas_linhas[i] = cabeca_linha[i]; // copia os endereços das primeiras células
        }

        delete[] cabeca_linha; // libera o vetor antigo, sem apagar as células
        cabeca_linha = novas_linhas; // passa a usar o novo vetor
        capacidade_linha = nova_capacidade_linha;
    };

    if(id_atributo >= capacidade_coluna) { // verifica se falta espaço para a coluna
        int nova_capacidade_coluna = id_atributo + 1;
        Celula** novas_colunas = new Celula*[nova_capacidade_coluna]();

        for(int i = 0; i < capacidade_coluna; i++) {
            novas_colunas[i] = cabeca_coluna[i]; // preserva as cabeças das colunas existentes
        }

        delete[] cabeca_coluna;
        cabeca_coluna = novas_colunas;
        capacidade_coluna = nova_capacidade_coluna;
    };
}

// aumenta o vetor de tripletas quando ele fica cheio
void Matriz_modificadores::expandir_tripletas() {
    if(quantidade < capacidade_tripletas) return; // ainda existe espaço disponível

    int nova_capacidade;

    if(capacidade_tripletas == 0) {
        nova_capacidade = 4; // começa com espaço para quatro tripletas
    } else nova_capacidade = capacidade_tripletas * 2; // dobra o espaço existente

    Tripleta* novo_vetor = new Tripleta[nova_capacidade];

    for(int i = 0; i < quantidade; i++) {
        novo_vetor[i] = tripletas[i]; // copia as tripletas para o novo vetor
    }

    delete[] tripletas;
    tripletas = novo_vetor;
    capacidade_tripletas = nova_capacidade;
}

// busca a posição da tripleta ou onde ela deveria ser inserida
int Matriz_modificadores::busca_posicao_T(int id_item, int id_atributo) {
    int inicio = 0;
    int fim = quantidade; // limite da busca, sem incluir essa posição

    while(inicio < fim) {
        int meio_vetor = inicio + (fim - inicio)/2; // encontra o meio do intervalo
        registrar_contagem(estatisticas.comparacoes_busca); // conta a comparação se habilitado

        // verifica se a tripleta do meio vem antes do par procurado
        bool vem_antes = tripletas[meio_vetor].id_item < id_item || (tripletas[meio_vetor].id_item == id_item && tripletas[meio_vetor].id_atributo < id_atributo);

        if(vem_antes) {
            inicio = meio_vetor + 1; // continua na metade direita
        } else {
            fim = meio_vetor; // continua procurando à esquerda
        }
    }

    return inicio; // posição encontrada ou posição para inserir
}

// remove uma tripleta e fecha o espaço que ela ocupava
void Matriz_modificadores::remover_T(int id_item, int id_atributo) {
    if (modo != 'T') throw "A matriz nao esta no modo T";
    posicao_valida(id_item, id_atributo);

    int posicao = busca_posicao_T(id_item, id_atributo);

    // encerra se a tripleta procurada não existir
    if(posicao == quantidade || tripletas[posicao].id_item != id_item || tripletas[posicao].id_atributo != id_atributo) return;

    registrar_alteracao(id_item, id_atributo, tripletas[posicao].valor, 0); // informa que o modificador passará a zero

    for(int i = posicao; i < quantidade - 1; i++) {
        tripletas[i] = tripletas[i + 1]; // desloca as seguintes uma posição para a esquerda
    }

    quantidade--; // desconta a tripleta removida
}

// insere uma nova tripleta na posição correta e se o valor for zero, remove
void Matriz_modificadores::inserir_T(int id_item, int id_atributo, int valor) {
    if (modo != 'T') throw "A matriz nao esta no modo T";
    posicao_valida(id_item, id_atributo);

    if (valor == 0) {
        remover_T(id_item, id_atributo);
        return;
    }

    int posicao = busca_posicao_T(id_item, id_atributo);

    // não insere outra tripleta para o mesmo item e atributo
    if (posicao < quantidade &&
        tripletas[posicao].id_item == id_item &&
        tripletas[posicao].id_atributo == id_atributo) {
        return;
    }

    expandir_tripletas(); // garante espaço para mais uma tripleta

    for (int i = quantidade; i > posicao; i--) {
        tripletas[i] = tripletas[i - 1]; // abre espaço deslocando as seguintes para a direita
    }

    tripletas[posicao] = Tripleta{id_item, id_atributo, valor}; // guarda a nova tripleta
    quantidade++;
    registrar_alteracao(id_item, id_atributo, 0, valor); // informa o novo modificador ao índice
}

// altera uma tripleta existente ou a remove se o novo valor for zero
void Matriz_modificadores::atualizar_T(int id_item, int id_atributo, int valor) {
    if (modo != 'T') throw "A matriz nao esta no modo T";
    posicao_valida(id_item, id_atributo);

    if (valor == 0) {
        remover_T(id_item, id_atributo);
        return;
    }

    int posicao = busca_posicao_T(id_item, id_atributo);

    // só atualiza se a tripleta realmente existir
    if (posicao < quantidade && tripletas[posicao].id_item == id_item && tripletas[posicao].id_atributo == id_atributo) {
        registrar_alteracao(id_item, id_atributo, tripletas[posicao].valor, valor);
        tripletas[posicao].valor = valor;
    }
}

// retorna o modificador no modo T ou zero se não existir
int Matriz_modificadores::consultar_valor_T(int id_item, int id_atributo) {
    if (modo != 'T') throw "A matriz nao esta no modo T";
    posicao_valida(id_item, id_atributo);

    int posicao = busca_posicao_T(id_item, id_atributo);

    // confirma que a posição encontrada corresponde ao item e atributo procurados
    if (posicao < quantidade && tripletas[posicao].id_item == id_item && tripletas[posicao].id_atributo == id_atributo) {
        return tripletas[posicao].valor;
    }

    return 0;
}

// cria uma matriz vazia no modo escolhido
Matriz_modificadores::Matriz_modificadores(char modo_inicial) {
    cabeca_linha = nullptr; // ainda não existe vetor de cabeças de linha
    cabeca_coluna = nullptr; // ainda não existe vetor de cabeças de coluna
    capacidade_linha = 0;
    capacidade_coluna = 0;
    quantidade = 0; // começa sem modificadores
    modo = modo_inicial;
    tripletas = nullptr; // ainda não existe vetor de tripletas
    capacidade_tripletas = 0;

    if(modo != 'L' && modo != 'T' && modo != 'C') throw "Modo invalido";
}

// libera as células e os vetores ao destruir a matriz
Matriz_modificadores::~Matriz_modificadores() {
    for(int i = 0; i < capacidade_linha; i++) { // percorre as linhas do modo L
        Celula* celula_atual = cabeca_linha[i];

        while(celula_atual != nullptr) {
            Celula* proxima_celula = celula_atual->celula_direita; // guarda a próxima antes de apagar
            delete celula_atual;
            celula_atual = proxima_celula;
        }
    }

    delete[] cabeca_coluna;
    delete[] cabeca_linha;
    delete[] tripletas;
}

// remove uma célula da lista cruzada e reconecta seus vizinhos
void Matriz_modificadores::remover_L(int id_item, int id_atributo) {
    if (modo != 'L') throw "A matriz nao esta no modo L";
    posicao_valida(id_item, id_atributo);

    if(id_item >= capacidade_linha || id_atributo >= capacidade_coluna) return; // não existe célula nessa posição

    Celula* celula_anterior_linha = nullptr; // acompanha a célula anterior na linha
    Celula* celula_atual = cabeca_linha[id_item]; // começa pela primeira célula do item

    while(celula_atual !=nullptr && celula_atual->id_atributo < id_atributo) {
        celula_anterior_linha = celula_atual;
        celula_atual = celula_atual->celula_direita; // avança até o atributo procurado
    }

    if(celula_atual == nullptr || celula_atual->id_atributo != id_atributo) return; // não encontrou a célula

    Celula* anterior_na_coluna = nullptr;
    Celula* atual_coluna = cabeca_coluna[id_atributo];

    while (atual_coluna != celula_atual) { // procura a mesma célula pela coluna
        anterior_na_coluna = atual_coluna;
        atual_coluna = atual_coluna->celula_abaixo;
    }

    if(celula_anterior_linha == nullptr) { // a célula removida é a primeira da linha
        cabeca_linha[id_item] = celula_atual->celula_direita;
    } else {
        celula_anterior_linha->celula_direita = celula_atual->celula_direita; // liga a anterior à próxima
    }

    if(anterior_na_coluna == nullptr) { // a célula removida é a primeira da coluna
        cabeca_coluna[id_atributo] = celula_atual->celula_abaixo;
    } else {
        anterior_na_coluna->celula_abaixo = celula_atual->celula_abaixo; // liga a de cima à de baixo
    }

    registrar_alteracao(id_item, id_atributo, celula_atual->valor, 0);
    delete celula_atual; // libera a célula removida
    quantidade--;
}

// insere uma célula na posição correta da linha e da coluna
void Matriz_modificadores::inserir_L(int id_item, int id_atributo, int valor) {
    if (modo != 'L') throw "A matriz nao esta no modo L";
    posicao_valida(id_item, id_atributo);

    if(valor == 0) { // zero significa remover
        remover_L(id_item, id_atributo);
        return;
    }

    expandir_cabecas(id_item, id_atributo); // garante espaço para a linha e a coluna

    Celula* anterior_linha = nullptr;
    Celula* atual_linha = cabeca_linha[id_item];

    while(atual_linha != nullptr && atual_linha->id_atributo < id_atributo) {
        anterior_linha = atual_linha;
        atual_linha = atual_linha->celula_direita; // procura onde inserir na linha
    }

    if (atual_linha != nullptr &&
        atual_linha->id_atributo == id_atributo) {
        return; // a célula já existe
    }

    Celula* anterior_na_coluna = nullptr;
    Celula* atual_na_coluna = cabeca_coluna[id_atributo];

    while(atual_na_coluna != nullptr && atual_na_coluna->id_item < id_item) {
        anterior_na_coluna = atual_na_coluna;
        atual_na_coluna = atual_na_coluna->celula_abaixo; // procura onde inserir na coluna
    }

    Celula* nova = new Celula(id_item, id_atributo, valor); // cria a célula
    nova->celula_direita = atual_linha; // aponta para a próxima célula da linha
    nova->celula_abaixo = atual_na_coluna; // aponta para a próxima célula da coluna

    if(anterior_linha == nullptr) {
        cabeca_linha[id_item] = nova; // a nova célula será a primeira da linha
    } else {
        anterior_linha->celula_direita = nova; // conecta a célula anterior à nova
    }

    if(anterior_na_coluna == nullptr) {
        cabeca_coluna[id_atributo] = nova; // a nova célula será a primeira da coluna
    } else {
        anterior_na_coluna->celula_abaixo = nova; // conecta a célula de cima à nova
    }

    quantidade++;
    registrar_alteracao(id_item, id_atributo, 0, valor);
}

// altera uma célula existente no modo L ou a remove se o valor for zero
void Matriz_modificadores::atualizar_L(int id_item, int id_atributo, int valor) {
    if (modo != 'L') throw "A matriz nao esta no modo L";
    posicao_valida(id_item, id_atributo);

    if(valor == 0) {
        remover_L(id_item, id_atributo);
        return;
    }

    if (id_item >= capacidade_linha ||
        id_atributo >= capacidade_coluna) {
        return; // a célula não existe
    }

    Celula* atual_linha = cabeca_linha[id_item];

    while(atual_linha != nullptr && atual_linha->id_atributo < id_atributo) {
        atual_linha = atual_linha->celula_direita; // procura o atributo na linha
    }

    if(atual_linha != nullptr && atual_linha->id_atributo == id_atributo) {
        registrar_alteracao(id_item, id_atributo, atual_linha->valor, valor);
        atual_linha->valor = valor; // substitui o modificador encontrado
    }
}

// retorna o modificador no modo L ou zero se não existir
int Matriz_modificadores::consultar_valor_L(int id_item, int id_atributo) {
    if (modo != 'L') throw "A matriz nao esta no modo L";
    posicao_valida(id_item, id_atributo);

    if(id_item >= capacidade_linha || id_atributo >= capacidade_coluna) return 0;

    Celula* atual = cabeca_linha[id_item];

    while(atual != nullptr && atual->id_atributo < id_atributo) {
        atual = atual->celula_direita; // procura o atributo na linha do item
    }

    if(atual != nullptr && atual->id_atributo == id_atributo) {
        return atual->valor;
    }

    return 0;
}

// informa o total de itens ao índice e prepara as linhas do modo C
void Matriz_modificadores::registrar_itens(int total) {
    indice_faixas.garantir_itens(total);
    if (modo == 'C') comprimida.garantir_linhas(total);
}

// informa ao índice quantos atributos existem
void Matriz_modificadores::registrar_atributos(int total) {
    indice_faixas.garantir_atributos(total);
}

// retorna os itens da faixa e coloca sua quantidade em total
ResultadoFaixa* Matriz_modificadores::consultar_faixa(int atributo, int minimo, int maximo, int& total) {
    return indice_faixas.consultar(atributo, minimo, maximo, total);
}

// retorna os contadores das consultas LF
EstatisticasFaixa Matriz_modificadores::get_estatisticas_faixa() const {
    return indice_faixas.get_estatisticas();
}

// estima os bytes alocados para o índice de LF
unsigned long long Matriz_modificadores::memoria_indice_estimada() const {
    return indice_faixas.memoria_dinamica();
}

// estima os bytes da representação da matriz, sem incluir o objeto ou o índice de LF
unsigned long long Matriz_modificadores::memoria_representacao_estimada() const {
    unsigned long long bytes = comprimida.memoria_dinamica(); // espaço dos vetores do modo C
    bytes += static_cast<unsigned long long>(capacidade_linha + capacidade_coluna) * sizeof(Celula*); // espaço das cabeças do modo L
    bytes += static_cast<unsigned long long>(capacidade_tripletas) * sizeof(Tripleta); // espaço do vetor do modo T
    if (modo == 'L') bytes += static_cast<unsigned long long>(quantidade) * sizeof(Celula); // espaço das células encadeadas
    return bytes;
}

// retorna a quantidade de modificadores diferentes de zero
int Matriz_modificadores::get_quantidade() const {
    return quantidade;
}

// troca a representação da matriz sem perder os modificadores
void Matriz_modificadores::mudar_modo(char novo_modo) {
    if (novo_modo != 'L' && novo_modo != 'T' && novo_modo != 'C') throw "Modo invalido";
    if (novo_modo == modo) return; // já está no modo solicitado

    Matriz_modificadores temporaria(novo_modo); // prepara uma matriz no novo modo
    temporaria.manter_indice = false; // o índice original já está correto e será mantido

    int linhas = indice_faixas.total_itens();

    if (novo_modo == 'C') temporaria.comprimida.iniciar_construcao(linhas, quantidade); // reserva espaço no modo C

    for (int item = 0; item < linhas; item++) { // percorre todos os itens
        Percurso p = percorrer_linha(item);
        Tripleta celula; // recebe um modificador por vez

        while (p.proximo(celula)) {
            if (novo_modo == 'C') {
                temporaria.comprimida.anexar_ordenado(item, celula.id_atributo, celula.valor); // copia para os vetores comprimidos
            } else if (novo_modo == 'L') {
                temporaria.inserir_L(item, celula.id_atributo, celula.valor); // copia para a lista cruzada
            } else {
                temporaria.inserir_T(item, celula.id_atributo, celula.valor); // copia para o vetor de tripletas
            }
        }
    }

    if (novo_modo == 'C') {
        temporaria.comprimida.concluir_construcao(); // calcula os inícios das linhas
        temporaria.quantidade = quantidade;
    }

    // libera as células antigas depois de terminar a cópia
    for (int i = 0; i < capacidade_linha; i++) {
        Celula* atual = cabeca_linha[i];

        while (atual != nullptr) {
            Celula* proxima = atual->celula_direita; // guarda a próxima antes de apagar
            delete atual; atual = proxima;
        }
    }

    delete[] cabeca_linha; delete[] cabeca_coluna; delete[] tripletas; // libera os vetores antigos

    // passa a usar os vetores e as capacidades da nova representação
    cabeca_linha = temporaria.cabeca_linha;
    cabeca_coluna = temporaria.cabeca_coluna;
    tripletas = temporaria.tripletas;
    capacidade_linha = temporaria.capacidade_linha;
    capacidade_coluna = temporaria.capacidade_coluna;
    capacidade_tripletas = temporaria.capacidade_tripletas;
    quantidade = temporaria.quantidade;

    comprimida.tomar_de(temporaria.comprimida); // transfere os vetores do modo C
    modo = novo_modo;

    // esvazia a temporária para que ela não apague os dados transferidos
    temporaria.cabeca_linha = temporaria.cabeca_coluna = nullptr;
    temporaria.tripletas = nullptr;
    temporaria.capacidade_linha = temporaria.capacidade_coluna = 0;
    temporaria.capacidade_tripletas = temporaria.quantidade = 0;
}

// retorna o modo atual: L, T ou C
char Matriz_modificadores::get_modo() {
    return modo;
}

// retorna as contagens de células percorridas e comparações nas buscas
EstatisticasCatalogo Matriz_modificadores::get_estatisticas() const {
    return estatisticas;
}

// zera os contadores do catálogo e de LF sem apagar os modificadores
void Matriz_modificadores::zerar_estatisticas() {
    estatisticas = EstatisticasCatalogo{};
    indice_faixas.zerar_estatisticas();
}

// estima os bytes do objeto, da representação da matriz e do índice de LF
unsigned long long Matriz_modificadores::memoria_estimada() const {
    unsigned long long bytes = sizeof(*this); // tamanho do próprio objeto
    bytes += static_cast<unsigned long long>(capacidade_linha) * sizeof(Celula*); // vetor de cabeças de linha
    bytes += static_cast<unsigned long long>(capacidade_coluna) * sizeof(Celula*); // vetor de cabeças de coluna
    bytes += static_cast<unsigned long long>(capacidade_tripletas) * sizeof(Tripleta); // vetor de tripletas

    if (modo == 'L') bytes += static_cast<unsigned long long>(quantidade) * sizeof(Celula); // células encadeadas

    return bytes + comprimida.memoria_dinamica() + indice_faixas.memoria_dinamica(); // inclui os vetores de C e o índice
}

// consulta um modificador usando o modo atual da matriz
int Matriz_modificadores::consultar(int item, int atributo) {
    posicao_valida(item, atributo);

    if (modo == 'C') return comprimida.consultar(item, atributo, &estatisticas);

    if (modo == 'L') {
        return consultar_valor_L(item, atributo);
    }

    return consultar_valor_T(item, atributo);
}

// remove uma célula usando o modo atual da matriz
void Matriz_modificadores::remover(int item, int atributo) {
    posicao_valida(item, atributo);

    if (modo == 'C') { atribuir(item, atributo, 0); return; } // no modo C, atribuir zero remove a célula

    if (modo == 'L') {
        remover_L(item, atributo);
    } else {
        remover_T(item, atributo);
    }
}

// insere ou atualiza um modificador; se o valor for zero, remove a célula
void Matriz_modificadores::atribuir(int item, int atributo, int valor) {
    posicao_valida(item, atributo);
    registrar_itens(item + 1); // garante que o item seja conhecido pelas estruturas auxiliares
    registrar_atributos(atributo + 1); // garante que o atributo também seja conhecido

    if (modo == 'C') {
        int antigo = comprimida.consultar(item, atributo, &estatisticas); // guarda o valor anterior
        comprimida.atribuir(item, atributo, valor, &estatisticas); // aplica a mudança no modo C
        quantidade = comprimida.total_celulas(); // atualiza a quantidade de modificadores
        registrar_alteracao(item, atributo, antigo, valor); // atualiza o índice de LF
        return;
    }

    if (valor == 0) { remover(item, atributo); return; }

    bool existe = consultar(item, atributo) != 0; // verifica se a célula já existe

    if (modo == 'L') {
        if (existe) atualizar_L(item, atributo, valor);
        else inserir_L(item, atributo, valor);
    } else {
        if (existe) atualizar_T(item, atributo, valor);
        else inserir_T(item, atributo, valor);
    }
}

// prepara a leitura dos modificadores de um item
Percurso Matriz_modificadores::percorrer_linha(int item) {
    posicao_valida(item, 0);
    Percurso p;

    p.estatisticas = &estatisticas; p.modo = modo; p.id = item; // informa os contadores, o modo e o item

    if (modo == 'L') {
        if (item < capacidade_linha) p.atual = cabeca_linha[item]; // começa na primeira célula da linha
    } else if (modo == 'T') {
        p.vetor = tripletas;
        p.posicao = busca_posicao_T(item, 0); p.fim = quantidade; // começa na primeira posição possível desse item
    } else {
        p.comprimida = &comprimida;
        p.posicao = comprimida.inicio_linha(item); // início do trecho do item
        p.fim = comprimida.fim_linha(item); // posição logo após o trecho do item
    }

    return p; // devolve o percurso pronto para usar proximo()
}

// prepara a leitura dos itens que modificam um atributo
Percurso Matriz_modificadores::percorrer_coluna(int atributo) {
    posicao_valida(0, atributo);
    Percurso p;

    p.estatisticas = &estatisticas; p.modo = modo;
    p.por_coluna = true; p.id = atributo; // indica que a leitura será de uma coluna

    if (modo == 'L') {
        if (atributo < capacidade_coluna) p.atual = cabeca_coluna[atributo]; // começa na primeira célula da coluna
    } else if (modo == 'T') {
        p.vetor = tripletas; p.fim = quantidade; // prepara a procura do atributo entre as tripletas
    } else {
        p.comprimida = &comprimida; p.fim = comprimida.total_linhas(); // prepara a procura do atributo em cada linha
    }

    return p;
}

// multiplica os modificadores dos atributos em comum entre dois itens e soma os resultados
double Matriz_modificadores::produto_interno(int item1, int item2) {
    Percurso p = percorrer_linha(item1); // prepara a leitura do primeiro item
    Percurso q = percorrer_linha(item2); // prepara a leitura do segundo item

    Tripleta a, b; // recebem os modificadores que serão comparados

    bool tem_a = p.proximo(a); // tenta ler o primeiro modificador do item 1
    bool tem_b = q.proximo(b); // tenta ler o primeiro modificador do item 2

    double soma = 0.0;

    while (tem_a && tem_b) { // continua enquanto ambos os percursos tiverem células
        if (a.id_atributo == b.id_atributo) {
            soma += static_cast<double>(a.valor) * b.valor; // multiplica os valores do mesmo atributo e soma

            tem_a = p.proximo(a); // avança nos dois itens
            tem_b = q.proximo(b);
        } else if (a.id_atributo < b.id_atributo) {
            tem_a = p.proximo(a); // avança apenas no item com o menor ID de atributo
        } else {
            tem_b = q.proximo(b);
        }
    }

    return soma;
}