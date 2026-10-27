#include "sistema_forja.h"
#include <iostream> 
#include <iomanip> 
#include <cmath> 

using namespace std;

// lê os comandos, processa encomendas, gerencia equipamentos e imprime as consultas e recomendações

// inicia a forja sem atributos, itens ou aventureiros cadastrados
SistemaForja::SistemaForja() {
    atributos = nullptr;
    itens = nullptr;
    aventureiros = nullptr;

    total_atributos = 0;
    total_itens = 0;
    total_aventureiros = 0;

    capacidade_atributos = 0;
    capacidade_itens = 0;
    capacidade_aventureiros = 0;
}

// libera os cadastros e seus vetores ao destruir a forja
SistemaForja::~SistemaForja() {
    for (int i = 0; i < total_aventureiros; i++) {
        delete aventureiros[i]; // libera o aventureiro, incluindo sua pilha
    }

    for (int i = 0; i < total_atributos; i++) {
        delete atributos[i]; // libera cada atributo
    }

    for (int i = 0; i < total_itens; i++) {
        delete itens[i]; // libera cada item
    }

    delete[] aventureiros; // libera os vetores de ponteiros
    delete[] atributos;
    delete[] itens;

    // a fila e a matriz são liberadas pelos próprios destrutores
}

// cadastra um atributo e imprime seu ID
void SistemaForja::cadastrar_atributo(Texto nome) {
    if (total_atributos == capacidade_atributos) { // verifica se falta espaço
        int nova;

        if (capacidade_atributos == 0) {
            nova = 4; // começa com quatro posições
        } else {
            nova = capacidade_atributos * 2; // dobra a capacidade existente
        }

        Atributo** novos = new Atributo*[nova]; // cria um vetor maior de ponteiros

        for (int i = 0; i < total_atributos; i++) {
            novos[i] = atributos[i]; // preserva os endereços dos atributos existentes
        }

        delete[] atributos; // libera apenas o vetor antigo
        atributos = novos;
        capacidade_atributos = nova;
    }

    atributos[total_atributos] = new Atributo(nome); // cria o atributo na próxima posição livre
    cout << "A " << atributos[total_atributos]->id_atributo << '\n';

    total_atributos++;
    matriz.registrar_atributos(total_atributos); // informa o novo total à matriz
}

// cadastra um item e imprime seu ID
void SistemaForja::cadastrar_item(Texto nome) {
    if (total_itens == capacidade_itens) { // verifica se falta espaço
        int nova;

        if (capacidade_itens == 0) {
            nova = 4;
        } else {
            nova = capacidade_itens * 2;
        }

        Item** novos = new Item*[nova]; // cria um vetor maior de ponteiros

        for (int i = 0; i < total_itens; i++) {
            novos[i] = itens[i]; // preserva os itens existentes
        }

        delete[] itens;
        itens = novos;
        capacidade_itens = nova;
    }

    itens[total_itens] = new Item(nome); // cria o novo item
    cout << "I " << itens[total_itens]->id_item<< '\n';

    total_itens++;
    matriz.registrar_itens(total_itens); // registra o item nas estruturas da matriz, incluindo o controle de zeros de LF
}

// cadastra um aventureiro com sua própria pilha de equipamentos e imprime seu ID
void SistemaForja::cadastrar_aventureiro(Texto nome) {
    if (total_aventureiros == capacidade_aventureiros) {
        int nova;

        if (capacidade_aventureiros == 0) {
            nova = 4;
        } else {
            nova = capacidade_aventureiros * 2;
        }

        Aventureiro** novos = new Aventureiro*[nova];

        for (int i = 0; i < total_aventureiros; i++) {
            novos[i] = aventureiros[i]; // preserva cada aventureiro e seus equipamentos
        }

        delete[] aventureiros;
        aventureiros = novos;
        capacidade_aventureiros = nova;
    }

    aventureiros[total_aventureiros] = new Aventureiro(nome); // cria o aventureiro

    cout << "H "
        << aventureiros[total_aventureiros]->id_aventureiro
        << '\n';

    total_aventureiros++;
}

// define o modificador de um item em um atributo e imprime a confirmação
void SistemaForja::modificar(int id_item, int id_atributo, int valor) {
    matriz.atribuir(id_item, id_atributo, valor); // insere, atualiza ou remove se o valor for zero

    cout << "M "
        << itens[id_item]->nome_item << ' '
        << atributos[id_atributo]->nome_atributo << ' '
        << valor << '\n';
}

// coloca o pedido no final da fila e imprime sua posição
void SistemaForja::encomendar(int id_aventureiro, int id_item) {
    fila.enfileirar(id_aventureiro, id_item);

    cout << "F " << fila.get_quantidade() << '\n'; // o novo pedido ocupa a última posição
}

// atende até k pedidos em ordem de chegada e equipa os itens nos aventureiros
void SistemaForja::trabalhar(int k) {
    int forjadas = k;

    if (forjadas > fila.get_quantidade()) {
        forjadas = fila.get_quantidade(); // não pode atender mais pedidos do que existem
    }

    cout << "Z " << forjadas << '\n';

    for (int i = 0; i < forjadas; i++) {
        int aventureiro, item; // recebem os dados do pedido retirado

        if (!fila.desenfileirar(aventureiro, item)) throw "Fila inconsistente"; // retira o primeiro pedido

        aventureiros[aventureiro]->equipamentos.empilhar(item); // coloca o item no topo da pilha do aventureiro

        if (i > 0) {
            cout << ' '; // separa os pedidos impressos na mesma linha
        }

        cout << "encomenda_" << i + 1 << ' '
            << aventureiro << ' '
            << item;
    }

    if (forjadas > 0) {
        cout << '\n'; // só existe uma segunda linha se algum pedido foi atendido
    }
}

// retira o último item equipado pelo aventureiro e imprime seu nome
void SistemaForja::desequipar(int id_aventureiro) {
    cout << "D "
        << aventureiros[id_aventureiro]->nome_aventureiro
        << ' ';

    int item; // recebe o ID do equipamento removido

    if (aventureiros[id_aventureiro]->equipamentos.desempilhar(item)) {
        cout << itens[item]->nome_item << '\n';
    } else {
        cout << "VAZIO\n"; // o aventureiro não possui equipamentos
    }
}

// lista os atributos que o item modifica e seus valores
void SistemaForja::listar_item(int id_item) {
    cout << "LI " << itens[id_item]->nome_item;

    Percurso p = matriz.percorrer_linha(id_item); // prepara a leitura dos modificadores do item
    Tripleta celula;
    int exibidos = 0; // conta os resultados impressos

    while (p.proximo(celula)) {
        exibidos++;

        cout << " atributo_" << exibidos << ' '
            << celula.id_atributo << ' '
            << celula.valor;
    }

    if (exibidos == 0) {
        cout << " VAZIO"; // o item não modifica nenhum atributo
    }

    cout << '\n';
}

// lista os itens que modificam o atributo e seus valores
void SistemaForja::listar_atributo(int id_atributo) {
    cout << "LA " << atributos[id_atributo]->nome_atributo;

    Percurso p = matriz.percorrer_coluna(id_atributo); // prepara a leitura dos itens desse atributo
    Tripleta celula;
    int exibidos = 0;

    while (p.proximo(celula)) {
        exibidos++;

        cout << " item_" << exibidos << ' '
            << celula.id_item << ' '
            << celula.valor;
    }

    if (exibidos == 0) {
        cout << " VAZIO"; // nenhum item modifica esse atributo
    }

    cout << '\n';
}

// calcula a build do aventureiro e imprime apenas os totais diferentes de zero
void SistemaForja::listar_build(int id_aventureiro) {
    cout << "LH "
        << aventureiros[id_aventureiro]->nome_aventureiro;

    long long* totais = aventureiros[id_aventureiro]->calcular_build(
        matriz,
        total_atributos
    ); // soma os modificadores de todos os itens equipados

    int exibidos = 0;

    for (int a = 0; a < total_atributos; a++) { // percorre os atributos em ordem de ID
        if (totais[a] != 0) {
            exibidos++;

            cout << " atributo_" << exibidos << ' '
                << a << ' '
                << totais[a];
        }
    }

    if (exibidos == 0) {
        cout << " VAZIO"; // todos os totais são zero
    }

    cout << '\n';
    delete[] totais; // libera o vetor usado para mostrar a build
}

// recomenda até n itens ainda não equipados, usando as métricas e seus pesos
void SistemaForja::recomendar(int id_aventureiro, int n, int m, const MetricaConsulta* metricas) {
    long long* totais = aventureiros[id_aventureiro]->calcular_build(
        matriz,
        total_atributos
    ); // obtém os totais atuais da build

    double* build = new double[total_atributos]();

    for (int a = 0; a < total_atributos; a++) {
        build[a] = static_cast<double>(totais[a]); // converte os totais para os cálculos das métricas
    }

    delete[] totais; // os valores já foram copiados

    double* pontos = calcular_pontuacoes(matriz, total_itens, total_atributos, build, metricas, m); // calcula os pontos de todos os itens

    bool* equipado = new bool[total_itens](); // começa marcando todos os itens como não equipados

    const ItemEquipado* atual =
        aventureiros[id_aventureiro]->equipamentos.get_topo();

    while (atual != nullptr) {
        equipado[atual->id_item] = true; // marca cada item que o aventureiro já usa
        atual = atual->anterior;
    }

    ItemOrdenado* candidatos = new ItemOrdenado[total_itens]; // guarda os itens que podem ser recomendados
    ItemOrdenado* auxiliar = new ItemOrdenado[total_itens]; // ajuda na ordenação

    int quantidade_candidatos = 0;

    for (int i = 0; i < total_itens; i++) {
        if (!equipado[i]) { // aceita apenas itens que não estão equipados
            double pontuacao = round(pontos[i] * 100.0) / 100.0; // arredonda para duas casas decimais
            candidatos[quantidade_candidatos] = ItemOrdenado{i, pontuacao};
            quantidade_candidatos++;
        }
    }

    ordenar_itens(candidatos, auxiliar, 0, quantidade_candidatos); // ordena por maior pontuação e desempata pelo menor ID

    if (n > quantidade_candidatos) n = quantidade_candidatos; // limita a resposta aos candidatos existentes

    for (int i = 0; i < n; i++) {
        // imprime a posição, o ID e a pontuação com duas casas decimais
        cout << "Q " << i + 1 << ' ' << candidatos[i].id_item << ' ' << fixed << setprecision(2) << candidatos[i].valor_arredondado << '\n';
    }

    delete[] build; // libera os vetores usados na consulta
    delete[] pontos;
    delete[] equipado;
    delete[] candidatos;
    delete[] auxiliar;
}

// imprime os itens cujo modificador está entre o mínimo e o máximo, incluindo os limites
void SistemaForja::listar_faixa(int atributo, int minimo, int maximo) {
    int total; // recebe a quantidade de resultados
    ResultadoFaixa* resultados = matriz.consultar_faixa(atributo, minimo, maximo, total);

    cout << "LF " << atributos[atributo]->nome_atributo;

    if (total == 0) cout << " VAZIO";

    for (int j = 0; j < total; j++) { // os resultados já estão em ordem de ID
        cout << " item_" << j + 1 << ' ' << resultados[j].id_item
             << ' ' << resultados[j].valor;
    }

    cout << '\n';
    delete[] resultados;
}

// lê cada comando e chama a função correspondente até terminar a entrada
void SistemaForja::executar() {
    Texto comando;

    while (cin >> comando) {
        if (comando == "E") { // escolhe ou muda o modo de armazenamento
            char modo;
            cin >> modo;

            matriz.mudar_modo(modo);
            cout << "E " << modo << '\n';

        } else if (comando == "A") { // cadastra um atributo
            Texto nome;
            cin >> nome;

            cadastrar_atributo(nome);

        } else if (comando == "I") { // cadastra um item
            Texto nome;
            cin >> nome;

            cadastrar_item(nome);

        } else if (comando == "M") { // define o modificador de um item em um atributo
            int item, atributo, valor;
            cin >> item >> atributo >> valor;

            modificar(item, atributo, valor);

        } else if (comando == "H") { // cadastra um aventureiro
            Texto nome;
            cin >> nome;

            cadastrar_aventureiro(nome);

        } else if (comando == "F") { // coloca uma encomenda na fila
            int aventureiro, item;
            cin >> aventureiro >> item;

            encomendar(aventureiro, item);

        } else if (comando == "Z") { // atende até k encomendas
            int k;
            cin >> k;

            trabalhar(k);

        } else if (comando == "D") { // remove o último equipamento do aventureiro
            int aventureiro;
            cin >> aventureiro;

            desequipar(aventureiro);

        } else if (comando == "LI") { // consulta os modificadores de um item
            int item;
            cin >> item;

            listar_item(item);

        } else if (comando == "LF") { // consulta os itens dentro de uma faixa de modificadores
            int atributo, minimo, maximo;
            cin >> atributo >> minimo >> maximo;

            listar_faixa(atributo, minimo, maximo);

        } else if (comando == "LA") { // consulta os itens que modificam um atributo
            int atributo;
            cin >> atributo;

            listar_atributo(atributo);

        } else if (comando == "LH") { // consulta os totais da build de um aventureiro
            int aventureiro;
            cin >> aventureiro;

            listar_build(aventureiro);

        } else if (comando == "Q") { // recomenda itens para um aventureiro
            int aventureiro, n, m;
            cin >> aventureiro >> n >> m; // lê quem consulta, quantos itens mostrar e quantas métricas usar

            MetricaConsulta* metricas = new MetricaConsulta[m];

            for (int j = 0; j < m; j++) {
                Texto nome;
                cin >> nome >> metricas[j].peso; // lê o nome e o peso da métrica

                if (nome == "PODER") {
                    metricas[j].tipo = TipoMetrica::PODER;
                } else if (nome == "EQUILIBRIO") {
                    metricas[j].tipo = TipoMetrica::EQUILIBRIO;
                } else if (nome == "SINERGIA") {
                    metricas[j].tipo = TipoMetrica::SINERGIA;
                }
            }

            recomendar(aventureiro, n, m, metricas);
            delete[] metricas;
        }
    }
}