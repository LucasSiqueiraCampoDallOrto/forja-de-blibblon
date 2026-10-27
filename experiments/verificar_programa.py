import subprocess
import random
import math
import sys
from pathlib import Path
from collections import deque
from decimal import Decimal

# Usa o executável informado no terminal ou o nome padrão.
if len(sys.argv) > 1:
    caminho_executavel = sys.argv[1]
else:
    caminho_executavel = "forja_extras.exe"

EXECUTAVEL = str(Path(caminho_executavel).resolve())


# Gera comandos e compara a saída do programa com uma simulação em Python.
def testar(seed, modo):
    # Fluxo: criar referência -> gerar comandos -> executar programa -> comparar saídas.

    r = random.Random(seed)  # permite repetir a mesma sequência de testes

    entrada = []  # comandos enviados ao programa
    esperado = []  # linhas que o programa deve imprimir

    attrs = []  # nomes dos atributos cadastrados
    items = []  # nomes dos itens cadastrados
    heroes = []  # nomes dos aventureiros cadastrados
    equipped = []  # equipamentos de cada aventureiro, na ordem em que chegaram
    matrix = {}  # valores dos modificadores, identificados por item e atributo
    queue = deque()  # fila de encomendas da simulação

    # Guarda um comando e calcula sua saída esperada.
    def cmd(s):
        # Fluxo: guardar comando -> identificar operação -> atualizar referência -> guardar resposta.

        entrada.append(s)
        t = s.split()
        op = t[0]

        if op == 'E':
            # A representação não muda os valores da simulação.
            esperado.append(s)

        elif op in ('A', 'I', 'H'):
            # Escolhe a lista correspondente ao cadastro.
            if op == 'A':
                target = attrs
            elif op == 'I':
                target = items
            else:
                target = heroes

            esperado.append(f'{op} {len(target)}')
            target.append(t[1])

            if op == 'H':
                equipped.append([])  # o novo aventureiro começa sem equipamentos

        elif op == 'M':
            i = int(t[1])
            a = int(t[2])
            v = int(t[3])

            matrix[i, a] = v  # guarda o valor esperado daquele modificador
            esperado.append(f'M {items[i]} {attrs[a]} {v}')

        elif op == 'F':
            h = int(t[1])
            i = int(t[2])

            queue.append((h, i))  # coloca a encomenda no final da fila
            esperado.append(f'F {len(queue)}')

        elif op == 'Z':
            count = min(int(t[1]), len(queue))
            esperado.append(f'Z {count}')
            parts = []

            for j in range(count):
                h, i = queue.popleft()  # retira a encomenda mais antiga
                equipped[h].append(i)  # equipa o item no aventureiro
                parts.append(f'encomenda_{j + 1} {h} {i}')

            if parts:
                esperado.append(' '.join(parts))

        elif op == 'D':
            h = int(t[1])

            if equipped[h]:
                item_removido = equipped[h].pop()
                name = items[item_removido]
            else:
                name = 'VAZIO'

            esperado.append(f'D {heroes[h]} {name}')

        elif op == 'LF':
            a = int(t[1])
            minimo = int(t[2])
            maximo = int(t[3])
            parts = []

            # Confere todos os itens, considerando zero quando não há modificador.
            for i in range(len(items)):
                v = matrix.get((i, a), 0)

                if minimo <= v <= maximo:
                    parts.append(f'item_{len(parts) + 1} {i} {v}')

            if parts:
                resposta = ' '.join(parts)
            else:
                resposta = 'VAZIO'

            esperado.append(f'LF {attrs[a]} ' + resposta)

        elif op in ('LI', 'LA', 'LH'):
            idx = int(t[1])
            parts = []
            values = []

            if op == 'LI':
                name = items[idx]
                label = 'atributo'

                # Reúne os modificadores do item em ordem de atributo.
                for a in range(len(attrs)):
                    values.append(matrix.get((idx, a), 0))

            elif op == 'LA':
                name = attrs[idx]
                label = 'item'

                # Reúne os modificadores do atributo em ordem de item.
                for i in range(len(items)):
                    values.append(matrix.get((i, idx), 0))

            else:
                name = heroes[idx]
                label = 'atributo'

                # Soma os modificadores dos equipamentos para cada atributo.
                for a in range(len(attrs)):
                    modificadores = []

                    for i in equipped[idx]:
                        modificadores.append(matrix.get((i, a), 0))

                    values.append(sum(modificadores))

            # Inclui somente os valores diferentes de zero.
            for i, v in enumerate(values):
                if v != 0:
                    parts.append(f'{label}_{len(parts) + 1} {i} {v}')

            if parts:
                resposta = ' '.join(parts)
            else:
                resposta = 'VAZIO'

            esperado.append(f'{op} {name} ' + resposta)

        elif op == 'Q':
            h = int(t[1])
            n = int(t[2])
            m = int(t[3])

            scores = []

            # Usa números decimais para acumular as pontuações ponderadas.
            for i in range(len(items)):
                scores.append(Decimal(0))

            build = []

            # Calcula a build atual do aventureiro consultado.
            for a in range(len(attrs)):
                modificadores = []

                for i in equipped[h]:
                    modificadores.append(matrix.get((i, a), 0))

                build.append(sum(modificadores))

            for j in range(m):
                metric = t[4 + 2 * j]
                weight = Decimal(t[5 + 2 * j])
                values = []

                # Calcula a métrica para todos os itens do catálogo.
                for i in range(len(items)):
                    row = []

                    for a in range(len(attrs)):
                        row.append(matrix.get((i, a), 0))

                    if metric == 'PODER':
                        v = sum(row)

                    elif metric == 'EQUILIBRIO':
                        if row:
                            mean = sum(row) / len(row)
                        else:
                            mean = 0

                        if row:
                            desvios_quadrados = []

                            for x in row:
                                desvios_quadrados.append((x - mean) ** 2)

                            v = 1 / (
                                1 + math.sqrt(
                                    sum(desvios_quadrados) / len(row)
                                )
                            )
                        else:
                            v = 1

                    else:
                        quadrados_item = []
                        quadrados_build = []

                        for x in row:
                            quadrados_item.append(x * x)

                        for x in build:
                            quadrados_build.append(x * x)

                        # Calcula o produto das normas do item e da build.
                        norm = (
                            math.sqrt(sum(quadrados_item))
                            * math.sqrt(sum(quadrados_build))
                        )

                        if norm != 0:
                            produtos = []

                            for x, y in zip(row, build):
                                produtos.append(x * y)

                            v = sum(produtos) / norm
                        else:
                            v = 0

                        v = max(-1, min(1, v))  # mantém o resultado entre -1 e 1

                    # Arredonda na escala de seis casas decimais para comparar.
                    rounded = math.copysign(
                        math.floor(abs(v) * 1000000.0 + 0.5),
                        v
                    )

                    values.append(rounded)

                # Define a ordem da métrica: maior valor primeiro, menor ID no empate.
                def chave_metrica(i):
                    return (-values[i], i)

                order = sorted(
                    range(len(items)),
                    key=chave_metrica
                )

                # Converte a posição de cada item em pontos ponderados.
                for rank, i in enumerate(order):
                    scores[i] = scores[i] + weight * (len(items) - rank)

            candidates = []

            # Só retira os itens equipados depois de calcular todos os pontos.
            for i in range(len(items)):
                if i not in equipped[h]:
                    candidates.append(i)

            # Ordena pela pontuação final e usa o ID para desempatar.
            def chave_pontuacao(i):
                return (-scores[i], i)

            candidates = sorted(candidates, key=chave_pontuacao)
            candidates = candidates[:n]  # mantém apenas os primeiros n candidatos

            for pos, i in enumerate(candidates, 1):
                esperado.append(f'Q {pos} {i} {scores[i]:.2f}')

    # Testa fila vazia, aventureiro sem equipamentos e catálogo sem itens.
    cmd(f'E {modo}')
    cmd('Z 3')
    cmd('H Hero')
    cmd('D 0')
    cmd('LH 0')
    cmd('Q 0 3 1 PODER 1.00')

    for j in range(9):
        cmd('I Item' + chr(65 + j))

    # Testa recomendações quando ainda não existem atributos.
    cmd('Q 0 20 3 PODER 1.00 EQUILIBRIO 2.00 SINERGIA 0.10')

    for j in range(7):
        cmd('A Attr' + chr(65 + j))

    for j in range(5):
        cmd('H Hero' + chr(65 + j))

    # Testa alterações de modificadores, itens repetidos, remoção e migração.
    comandos_iniciais = [
        'M 0 0 5',
        'M 0 0 7',
        'F 0 0',
        'F 1 2',
        'F 0 0',
        'Z 8',
        'LH 0',
        'D 0',
        'LH 0',
        'M 0 0 -3',
        'LH 0',
        'M 0 0 0',
        'LI 0',
        'E T',
        'E L'
    ]

    for s in comandos_iniciais:
        cmd(s)

    # Gera uma sequência variada de comandos usando a semente do teste.
    for turn in range(1300):
        op = r.choice([
            'E', 'M', 'M', 'M', 'F', 'Z', 'D', 'LI',
            'LA', 'LH', 'Q', 'LF', 'LF', 'LF', 'A', 'I'
        ])

        i = r.randrange(len(items))
        a = r.randrange(len(attrs))
        h = r.randrange(len(heroes))

        if op == 'E':
            s = 'E ' + r.choice(['L', 'T', 'C'])

        elif op == 'A':
            s = 'A AttrExtra' + chr(65 + turn % 26)

        elif op == 'I':
            s = 'I ItemExtra' + chr(65 + turn % 26)

        elif op == 'LF':
            x, y = r.choice([
                (-9, -1),
                (0, 0),
                (-3, 3),
                (8, 20),
                (-2147483648, 2147483647),
                (-7, -7),
                (7, 7)
            ])

            s = f'LF {a} {x} {y}'

        elif op == 'M':
            s = f'M {i} {a} {r.randrange(-7, 8)}'

        elif op == 'F':
            s = f'F {h} {i}'

        elif op == 'Z':
            s = f'Z {r.randrange(1, 10)}'

        elif op in ('D', 'LH'):
            s = f'{op} {h}'

        elif op == 'LI':
            s = f'LI {i}'

        elif op == 'LA':
            s = f'LA {a}'

        else:
            s = (
                f'Q {h} {r.randrange(1, 12)} '
                '3 PODER 0.10 EQUILIBRIO 0.20 SINERGIA 0.30'
            )

        cmd(s)

    # Testa valores extremos e zeros após várias migrações.
    for modo_final in 'LCTLTC':
        cmd('E ' + modo_final)

        for valor in [-2147483648, 2147483647, 0, -1, 1, 0]:
            cmd(f'M 0 0 {valor}')
            cmd(f'LF 0 {valor} {valor}')
            cmd('LF 0 -2147483648 2147483647')

    # Equipa todos os itens para testar uma recomendação sem candidatos.
    for i in range(len(items)):
        cmd(f'F 0 {i}')

    cmd('Z 999')
    cmd('Q 0 10 1 PODER 1.00')

    # Executa o programa e envia toda a entrada gerada.
    actual = subprocess.run(
        [EXECUTAVEL],
        input='\n'.join(entrada) + '\n',
        text=True,
        capture_output=True
    )

    assert actual.returncode == 0, actual.stderr
    assert not actual.stderr, actual.stderr

    lines = actual.stdout.splitlines()

    # Procura a primeira diferença para informar onde a comparação falhou.
    if lines != esperado:
        diferenca = (len(lines), len(esperado))

        for i, (x, y) in enumerate(zip(lines, esperado)):
            if x != y:
                diferenca = (i, x, y)
                break

        assert lines == esperado, diferenca

    print(
        f'OK: modo inicial {modo}, {len(entrada)} comandos, '
        f'{len(lines)} linhas verificadas; saida e stderr corretos.'
    )


# Repete os testes com três sementes e cada modo inicial.
for seed in [42, 91, 2026]:
    for modo in 'LTC':
        testar(seed, modo)