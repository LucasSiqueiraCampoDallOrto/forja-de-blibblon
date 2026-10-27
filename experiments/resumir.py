from pathlib import Path
from collections import defaultdict
import csv
import json
import math
import statistics as st
import sys
import zipfile

PASTA = Path(__file__).resolve().parent  # pasta deste script
R = PASTA / 'resultados'  # pasta que contém as coletas
MODOS = ('L', 'T', 'C')  # representações comparadas


# Seleciona as linhas que atendem a todos os filtros recebidos.
def selecionar(linhas, **filtros):
    # Fluxo: percorrer linhas -> conferir filtros -> guardar correspondências.

    selecionadas = []

    for r in linhas:
        corresponde = True

        for k, v in filtros.items():
            if r[k] != str(v):
                corresponde = False
                break

        if corresponde:
            selecionadas.append(r)

    return selecionadas


# Calcula a mediana de uma coluna, aplicando a escala indicada.
def med(linhas, coluna, escala=1):
    # Fluxo: conferir dados -> converter valores -> calcular mediana.

    if not linhas:
        raise ValueError('Faltam resultados para ' + coluna)

    valores = []

    for r in linhas:
        valores.append(float(r[coluna]) * escala)

    return st.median(valores)


# Escreve um número com três casas decimais.
def fmt(valor):
    return f'{valor:.3f}'


# Monta uma tabela em Markdown com cabeçalho e linhas de dados.
def tabela(cabecalho, linhas):
    # Fluxo: escrever cabeçalho -> escrever separadores -> acrescentar linhas.

    texto = '| ' + ' | '.join(cabecalho) + ' |\n'
    separadores = []

    for coluna in cabecalho:
        separadores.append('---')

    texto = texto + '| ' + ' | '.join(separadores) + ' |\n'

    for r in linhas:
        campos = []

        for valor in r:
            campos.append(str(valor))

        texto = texto + '| ' + ' | '.join(campos) + ' |\n'

    return texto + '\n'


# Confere as coletas e gera o resumo, o relatório, o gráfico e o ZIP.
def main():
    # Fluxo: validar coletas -> ler CSVs -> resumir medidas -> gerar arquivos.

    metadados = {}

    # Confere se os três tipos de coleta terminaram corretamente.
    for coleta in ('tempos', 'contadores', 'extras'):
        p = R / ('coleta_' + coleta + '.json')

        if not p.is_file():
            raise ValueError(
                'Falta a coleta ' + coleta + '. Execute medir.py.'
            )

        m = json.loads(p.read_text(encoding='utf-8'))

        if m['status'] != 'concluida':
            raise ValueError(
                'A coleta ' + coleta + ' nao terminou corretamente.'
            )

        metadados[coleta] = m

    fonte = metadados['tempos']  # referência para comparar as versões

    # Confere se as coletas usaram as mesmas versões e opções.
    for m in metadados.values():
        for chave in (
            'fontes_tp_sha256',
            'bancada_sha256',
            'compilador',
            'flags'
        ):
            if m[chave] != fonte[chave]:
                raise ValueError(
                    'As coletas usam versoes diferentes: ' + chave
                )

    # Quantidade esperada de linhas por repetição de cada experimento.
    multiplicadores = {
        'perfis': 1440,
        'migracoes': 90,
        'ordenacoes': 108,
        'produtos': 72,
        'limiar': 54,
        'faixas': 48
    }

    dados = {}

    # Lê os CSVs e confere a quantidade de registros e os tempos.
    for coleta in ('tempos', 'contadores', 'extras'):
        if coleta != 'extras':
            tamanhos = multiplicadores
        else:
            tamanhos = {
                'operacoes': 15,
                'migracoes': 30
            }

        for sufixo, por_rep in tamanhos.items():
            caminho = R / (coleta + '_' + sufixo + '.csv')

            with caminho.open(encoding='utf-8-sig', newline='') as f:
                linhas = list(csv.DictReader(f))

            esperado = por_rep * metadados[coleta]['repeticoes']

            if len(linhas) != esperado:
                raise ValueError(
                    f'{coleta}_{sufixo}: esperava {esperado} linhas, '
                    f'encontrou {len(linhas)}.'
                )

            for r in linhas:
                for k, v in r.items():
                    # Identifica as colunas que armazenam tempos.
                    if (
                        'tempo' in k
                        or k.endswith('_us')
                        or k in ('q_medio_ns', 'percurso_us_linha')
                    ):
                        if not math.isfinite(float(v)) or float(v) < 0:
                            raise ValueError(
                                'Medicao numerica invalida em ' + sufixo
                            )

            dados[coleta, sufixo] = linhas

    perfis = dados['tempos', 'perfis']
    base = selecionar(perfis, cenario='base')
    grupos = defaultdict(list)  # uma lista para cada grupo de medidas

    # Agrupa repetições da mesma operação e configuração.
    for r in perfis:
        campos = []

        for k in ('cenario', 'perfil', 'modo', 'estrategia', 'comando'):
            campos.append(r[k])

        chave = tuple(campos)
        grupos[chave].append(r)

    resumo = []

    for chave, linhas in sorted(grupos.items()):
        v = []

        for r in linhas:
            v.append(float(r['tempo_medio_ns']) / 1000)

        # Calcula os quartis quando há mais de uma repetição.
        if len(v) > 1:
            q = st.quantiles(v, n=4, method='inclusive')
        else:
            q = [v[0], v[0], v[0]]

        linha_resumo = list(chave)
        linha_resumo.append(len(v))
        linha_resumo.append(st.median(v))
        linha_resumo.append(q[0])
        linha_resumo.append(q[2])
        resumo.append(linha_resumo)

    # Salva mediana e percentis 25 e 75 em um CSV.
    with (R / 'resumo_operacoes.csv').open(
        'w', newline='', encoding='utf-8'
    ) as f:
        w = csv.writer(f)

        w.writerow([
            'cenario',
            'perfil',
            'modo',
            'estrategia',
            'comando',
            'repeticoes',
            'mediana_us',
            'p25_us',
            'p75_us'
        ])

        w.writerows(resumo)

    # Começa o relatório com as informações do ambiente e da metodologia.
    texto = ['# Resultados da bancada modular do TP\n']

    texto.append(
        'Este é um relatório de apoio para escrever a documentação. '
        'As tabelas usam apenas as novas coletas concluídas. '
        'Os resultados antigos não foram misturados.\n'
    )

    texto.append('## Ambiente e método\n')

    texto.append(
        f'- Sistema: {fonte["sistema"]}.\n'
        f'- Compilador: {fonte["compilador"]}.\n'
        f'- Opções: `{" ".join(fonte["flags"])}`.\n'
        f'- Repetições: tempos={fonte["repeticoes"]}; '
        f'contadores={metadados["contadores"]["repeticoes"]}; '
        f'extras={metadados["extras"]["repeticoes"]}.\n'
    )

    texto.append(
        '```text\n' + fonte['relogio'].strip() + '\n```\n'
    )

    texto.append(
        'Os intervalos vazios acima caracterizam esta execução, não uma '
        'resolução universal. Seu custo não foi subtraído. Diferenças próximas '
        'desse custo exigem cautela. Medianas resumem os tempos médios das '
        'repetições; P25–P75 está no resumo CSV. As sementes dos perfis são '
        '773 + 97 × repetição: incluem variação de entrada, além do ambiente. '
        'Com uma repetição, não se estima dispersão.\n'
    )

    texto.append(
        'Os arquivos `.cpp` de TP são compilados separadamente, sem modificar '
        'o projeto. Os TADs e as métricas são os mesmos. `MotorExperimental` '
        'executa operações sem entrada/saída textual e reutiliza buffers de '
        'build e Q nas duas estratégias. Os algoritmos alternativos pertencem '
        'à bancada; os tempos não representam o programa completo lendo e '
        'imprimindo comandos. A memória estimada exclui custos do alocador, '
        'fragmentação, nomes dinâmicos de Texto e picos transitórios.\n'
    )

    texto.append('## Configurações dos perfis\n')

    config = []
    nomes_cenarios = {}

    # Reúne os nomes dos cenários, sem repetir e mantendo a ordem.
    for r in perfis:
        nomes_cenarios[r['cenario']] = None

    for nome in nomes_cenarios:
        r = selecionar(perfis, cenario=nome)[0]

        config.append([
            nome,
            r['itens'],
            r['atributos'],
            fmt(100 * float(r['densidade_inicial'])),
            r['build_inicial'],
            r['fila_inicial'],
            r['n'],
            r['m']
        ])

    texto.append(tabela(
        [
            'Cenário',
            'Itens',
            'Atributos',
            'Densidade efetiva inicial (%)¹',
            'Build inicial',
            'Fila inicial',
            'n',
            'm'
        ],
        config
    ))

    texto.append(
        '¹ Primeira semente; os CSVs registram também densidade final e '
        'médias observadas de build/fila. São 8 aventureiros, 1.200 comandos '
        'por carga e k=1 em Z. Os fatores variam um por vez, sem formar um '
        'projeto fatorial completo.\n'
    )

    texto.append(tabela(
        ['Perfil', 'M', 'F', 'Z', 'D', 'LI', 'LA', 'LH', 'Q'],
        [
            [
                'Atualizações',
                '30%', '20%', '20%', '20%',
                '2%', '2%', '3%', '3%'
            ],
            [
                'Consultas',
                '4%', '2%', '2%', '2%',
                '20%', '20%', '25%', '25%'
            ]
        ]
    ))

    texto.append(
        '## Operações no cenário base\n'
        'Build sob demanda; medianas em µs por operação.\n'
    )

    # Compara os tempos de cada comando nos três modos.
    linhas = []

    for p in ('atualizacoes', 'consultas'):
        for c in ('M', 'F', 'Z', 'D', 'LI', 'LA', 'LH', 'Q'):
            linha = [p, c]

            for m in MODOS:
                selecionadas = selecionar(
                    base,
                    perfil=p,
                    comando=c,
                    modo=m,
                    estrategia='demanda'
                )

                linha.append(fmt(med(
                    selecionadas, 'tempo_medio_ns', 0.001
                )))

            linhas.append(linha)

    texto.append(tabela(
        ['Perfil', 'Comando', 'L', 'T', 'C'],
        linhas
    ))

    texto.append(
        'O índice de LF está ativo nos três modos: seu custo de manutenção '
        'está incluído em M. L percorre colunas pelos encadeamentos; T filtra '
        'o vetor de tripletas; C procura o atributo em cada linha. Essa '
        'diferença deve ser considerada ao discutir LA.\n'
    )

    texto.append(
        '## Estratégias de build\n'
        'Cenário base; medianas em µs. As linhas M mostram também o custo de '
        'manter os totais após alterar um item equipado.\n'
    )

    # Compara as estratégias de build nos comandos selecionados.
    linhas = []

    for p in ('atualizacoes', 'consultas'):
        for m in MODOS:
            for c in ('M', 'Z', 'D', 'LH', 'Q'):
                linha = [p, m, c]

                for e in ('demanda', 'imediata'):
                    selecionadas = selecionar(
                        base,
                        perfil=p,
                        modo=m,
                        comando=c,
                        estrategia=e
                    )

                    linha.append(fmt(med(
                        selecionadas, 'tempo_medio_ns', 0.001
                    )))

                linhas.append(linha)

    texto.append(tabela(
        ['Perfil', 'Modo', 'Comando', 'Demanda', 'Imediata'],
        linhas
    ))

    texto.append(
        '## Totais e memória\n'
        'Soma dos intervalos das 1.200 operações, em ms. Não inclui setup '
        'nem entrada/saída; `tempo_loop_ns` também inclui acompanhamento '
        'experimental.\n'
    )

    # Resume o tempo total das operações de cada variante.
    linhas = []

    for p in ('atualizacoes', 'consultas'):
        for m in MODOS:
            for e in ('demanda', 'imediata'):
                selecionadas = selecionar(
                    base,
                    perfil=p,
                    modo=m,
                    estrategia=e,
                    comando='M'
                )

                total = med(
                    selecionadas,
                    'tempo_total_operacoes_ns',
                    0.000001
                )

                linhas.append([p, m, e, fmt(total)])

    texto.append(tabela(
        ['Perfil', 'Modo', 'Build', 'Total (ms)'],
        linhas
    ))

    # Reúne as estimativas de memória do catálogo e do motor experimental.
    linhas = []

    for p in ('atualizacoes', 'consultas'):
        for m in MODOS:
            linha = [p, m]

            for k in (
                'memoria_matriz_bytes',
                'memoria_representacao_bytes',
                'memoria_indice_bytes',
                'memoria_motor_pico_bytes'
            ):
                selecionadas = selecionar(
                    base,
                    perfil=p,
                    modo=m,
                    estrategia='demanda',
                    comando='M'
                )

                linha.append(int(med(selecionadas, k)))

            linhas.append(linha)

    texto.append(tabela(
        [
            'Perfil',
            'Modo',
            'Catálogo total (bytes)',
            'Representação dinâmica',
            'Índice LF dinâmico',
            'Pico estável estimado do motor'
        ],
        linhas
    ))

    texto.append(
        'O total do catálogo inclui o objeto; as colunas de representação '
        'e índice mostram apenas alocações dinâmicas. Capacidades retidas '
        'após remoção também são contadas. O pico do motor é acompanhado '
        'nos estados após operações, com a estimativa do catálogo atualizada '
        'após M; não é RSS.\n'
    )

    texto.append(
        '## Influência dos parâmetros em Q\n'
        'Perfil de consultas, build sob demanda; medianas em µs.\n'
    )

    # Mostra como o tempo de Q varia entre os cenários.
    linhas = []

    for c in config:
        linha = [c[0]]

        for m in MODOS:
            selecionadas = selecionar(
                perfis,
                cenario=c[0],
                perfil='consultas',
                modo=m,
                estrategia='demanda',
                comando='Q'
            )

            linha.append(fmt(med(
                selecionadas, 'tempo_medio_ns', 0.001
            )))

        linhas.append(linha)

    texto.append(tabela(['Cenário', 'L', 'T', 'C'], linhas))

    texto.append(
        '## Migrações\n'
        'Cenário base; cada direção parte de uma matriz nova, com entradas '
        'pareadas entre representações. Tempo inclui criar o destino e '
        'liberar a representação antiga, preservando o índice de LF.\n'
    )

    # Calcula a mediana do tempo de cada direção de migração.
    mig = selecionar(dados['tempos', 'migracoes'], cenario='base')
    linhas = []

    for d in ('L_T', 'L_C', 'T_L', 'T_C', 'C_L', 'C_T'):
        selecionadas = selecionar(mig, direcao=d)
        tempo = med(selecionadas, 'tempo_ns', 0.001)
        linhas.append([d, fmt(tempo)])

    texto.append(tabela(['Direção', 'Mediana (µs)'], linhas))

    texto.append(
        '## Ordenações em Q\n'
        'Recorte N=512, uma alteração M entre consultas, 40 consultas medidas. '
        'Cada célula mostra tempo de Q (µs) / comparações nas ordenações das '
        'métricas. Tempos usam medianas; contagens usam a coleta separada. '
        'O CSV também separa os tempos de ordenação das métricas e de todas '
        'as ordenações.\n'
    )

    # Compara os algoritmos com e sem reutilização da ordem anterior.
    so = dados['tempos', 'ordenacoes']
    co = dados['contadores', 'ordenacoes']
    linhas = []

    for m in MODOS:
        for reutiliza in ('0', '1'):
            if reutiliza == '1':
                resposta = 'sim'
            else:
                resposta = 'não'

            row = [m, resposta]

            for alg in ('merge', 'insercao', 'heap'):
                filtro = {
                    'itens': 512,
                    'modo': m,
                    'alteracoes_por_q': 1,
                    'reutiliza': reutiliza,
                    'algoritmo': alg
                }

                tempo = med(
                    selecionar(so, **filtro),
                    'q_medio_ns',
                    0.001
                )

                comparacoes = med(
                    selecionar(co, **filtro),
                    'comparacoes_metricas_por_q'
                )

                row.append(fmt(tempo) + ' / ' + fmt(comparacoes))

            linhas.append(row)

    texto.append(tabela(
        ['Modo', 'Reutiliza ordem', 'Merge', 'Inserção', 'Heap'],
        linhas
    ))

    texto.append(
        'Poucas mudanças podem favorecer a ordenação por inserção somente '
        'se a ordem anterior chegar ao algoritmo. A variante que reinicia '
        'por ID evita confundir poucas atualizações com entrada já quase '
        'ordenada. Os CSVs incluem 0, 1 e 16 alterações e N=128/512.\n'
    )

    texto.append(
        '## Produto interno\n'
        'Recorte T=4096; medianas em µs por par. O buffer denso é alocado '
        'antes do lote, mas sua limpeza faz parte do custo. Ambos os '
        'produtos são funções compiladas em unidades separadas, sem LTO.\n'
    )

    # Compara os dois produtos internos em diferentes densidades.
    pr = dados['tempos', 'produtos']
    linhas = []

    for m in MODOS:
        for d in ('0.002', '0.02', '0.2', '0.8'):
            linha = [m, d]

            for a in ('denso', 'intercalado'):
                selecionadas = selecionar(
                    pr,
                    modo=m,
                    atributos=4096,
                    densidade_solicitada=d,
                    algoritmo=a
                )

                linha.append(fmt(med(
                    selecionadas, 'tempo_medio_ns', 0.001
                )))

            linhas.append(linha)

    texto.append(tabela(
        ['Modo', 'Densidade solicitada', 'Denso', 'Intercalado'],
        linhas
    ))

    texto.append(
        '## Frequência de consultas da build\n'
        'Ensaio isolado com 50.000 operações e catálogo fixo; totais em ms. '
        'Consulta é apenas leitura do vetor, sem Q, fila ou M. O primeiro '
        'ponto favorável em uma amostragem não determina um limiar universal '
        'para os perfis completos.\n'
    )

    # Compara as estratégias conforme aumenta a frequência de consultas.
    li = dados['tempos', 'limiar']
    linhas = []

    for m in MODOS:
        for p in (0, 1, 2, 5, 10, 25, 50, 75, 90):
            linha = [m, p]

            for e in ('demanda', 'imediata'):
                selecionadas = selecionar(
                    li,
                    modo=m,
                    consultas_porcento=p,
                    estrategia=e
                )

                linha.append(fmt(med(
                    selecionadas, 'tempo_total_ns', 0.000001
                )))

            linhas.append(linha)

    texto.append(tabela(
        ['Modo', 'Consultas (%)', 'Demanda', 'Imediata'],
        linhas
    ))

    texto.append(
        '## Percurso e inserção: comparação específica dos extras\n'
        'Medianas em µs. Percurso é por linha, com leitura/consumo das células; '
        'os demais tempos são por célula alterada. O índice LF está ativo '
        'igualmente em L, T e C.\n'
    )

    # Resume os testes específicos de percurso e alteração de células.
    ex = dados['extras', 'operacoes']
    linhas = []

    for c in range(5):
        for m in MODOS:
            selecionadas = selecionar(ex, config=c, modo=m)

            linha = [
                c,
                m,
                selecionadas[0]['itens'],
                selecionadas[0]['atributos'],
                fmt(med(selecionadas, 'densidade_real'))
            ]

            for k in (
                'percurso_us_linha',
                'insercao_us',
                'substituicao_us',
                'remocao_us'
            ):
                linha.append(fmt(med(selecionadas, k)))

            linhas.append(linha)

    texto.append(tabela(
        [
            'Config.',
            'Modo',
            'N',
            'T',
            'Densidade efetiva',
            'Percurso/linha',
            'Inserção',
            'Substituição',
            'Remoção'
        ],
        linhas
    ))

    texto.append(
        'C percorre valores/atributos contíguos e obtém os limites da linha '
        'diretamente. T também armazena células contíguas, mas localiza linhas '
        'por busca binária. L segue ponteiros; a disposição efetiva dos nós '
        'depende do alocador. Inserir em T/C pode deslocar vetores; C também '
        'ajusta índices das linhas seguintes. As diferenças observadas podem '
        'ser discutidas à luz desses padrões, mas não comprovam taxas de '
        'falhas de cache.\n'
    )

    texto.append(
        '## Consulta LF\n'
        'Recorte N=4096; tempo médio por consulta em µs, resumido pela mediana '
        'das repetições. Inclui criar/ordenar/liberar a resposta e consumi-la, '
        'sem impressão. Contagens vêm da execução instrumentada.\n'
    )

    # Compara consultas por faixa com diferentes quantidades de resultados.
    lf = dados['tempos', 'faixas']
    lc = dados['contadores', 'faixas']
    linhas = []

    for m in MODOS:
        for d in ('0.02', '0.5'):
            for minimo, maximo in ((0, 0), (-1, 1), (2, 4), (100, 200)):
                filtro = {
                    'itens': 4096,
                    'modo': m,
                    'densidade_solicitada': d,
                    'minimo': minimo,
                    'maximo': maximo
                }

                rr = selecionar(lf, **filtro)
                cc = selecionar(lc, **filtro)

                linhas.append([
                    m,
                    d,
                    f'[{minimo},{maximo}]',
                    fmt(med(rr, 'resultados_por_consulta')),
                    fmt(med(rr, 'tempo_medio_ns', 0.001)),
                    fmt(med(cc, 'comparacoes_limites_por_consulta')),
                    fmt(med(cc, 'resultados_copiados_por_consulta'))
                ])

    texto.append(tabela(
        [
            'Modo',
            'Densidade',
            'Faixa',
            'K (mediana)',
            'Tempo µs',
            'Comparações limites¹',
            'Resultados copiados¹'
        ],
        linhas
    ))

    texto.append(
        '¹ Coleta de contadores: a semente e os resultados podem diferir da '
        'mediana de K de cinco sementes. LF localiza os limites por busca '
        'binária; os zeros são enumerados pelos intervalos de IDs ausentes. '
        'O custo de produzir uma resposta grande não deve ser confundido '
        'com uma busca linear pelo catálogo.\n'
    )

    texto.append(
        '## Contadores e limites de interpretação\n'
        '`celulas_percorridas` e `comparacoes_busca` são os contadores '
        'existentes no TP. Não são contadores de cache. Em C, buscas binárias '
        'contam comparações; percursos contam células devolvidas. Em T, '
        'o filtro de coluna conta as tripletas examinadas. Em L, buscas '
        'por ponteiros não têm a mesma instrumentação de comparações. '
        'Não trate um único contador isolado como uma medida uniforme de '
        'todos os acessos físicos entre modos.\n'
    )

    texto.append(
        'As médias muito pequenas incluem overhead do relógio e do mecanismo '
        'experimental. Geradores, buffers e materialização inicial ficam '
        'fora dos intervalos; alocações inerentes às operações permanecem '
        'incluídas. Resultados valem para esta implementação e estas cargas.\n'
    )

    # Salva o relatório completo em Markdown.
    (R / 'RELATORIO_TP_ATUAL.md').write_text(
        '\n'.join(texto),
        encoding='utf-8'
    )

    try:
        import matplotlib

        matplotlib.use('Agg')  # gera a imagem sem abrir uma janela

        import matplotlib.pyplot as plt

        fig, ax = plt.subplots(figsize=(7, 4))
        xs = list(range(3))  # posições dos modos no gráfico
        larg = 0.35  # largura das barras

        # Desenha uma barra por estratégia de build em cada modo.
        for j, e in enumerate(('demanda', 'imediata')):
            valores = []

            for m in MODOS:
                selecionadas = selecionar(
                    base,
                    perfil='consultas',
                    modo=m,
                    estrategia=e,
                    comando='Q'
                )

                valores.append(med(
                    selecionadas, 'tempo_medio_ns', 0.001
                ))

            posicoes = []

            for x in xs:
                posicoes.append(x + (j - 0.5) * larg)

            ax.bar(
                posicoes,
                valores,
                width=larg,
                label=e
            )

        ax.set_xticks(xs)
        ax.set_xticklabels(MODOS)
        ax.set_ylabel('Q: mediana (µs)')
        ax.legend()
        fig.tight_layout()
        fig.savefig(R / 'grafico_q.png', dpi=180)
        plt.close(fig)  # libera os recursos da figura

    except ImportError:
        # O relatório e o ZIP continuam sendo gerados sem a biblioteca.
        print('Relatorio gerado sem grafico; matplotlib e opcional.')

    destino = PASTA / 'resultados_TP_atual.zip'

    # Inclui os arquivos diretamente presentes na pasta resultados.
    with zipfile.ZipFile(
        destino, 'w', zipfile.ZIP_DEFLATED
    ) as z:
        for p in sorted(R.iterdir()):
            if p.is_file():
                z.write(p, 'resultados/' + p.name)

    print(
        'Gerados RELATORIO_TP_ATUAL.md, resumo_operacoes.csv '
        'e resultados_TP_atual.zip.'
    )

    print(
        'Envie resultados_TP_atual.zip para revisarmos a discussao '
        'e a documentacao.'
    )


# Inicia o resumo quando este arquivo é executado diretamente.
if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError) as erro:
        sys.exit('Nao foi possivel resumir: ' + str(erro))