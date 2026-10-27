# Bancadas experimentais

Esta pasta reúne o código-fonte das bancadas e automações em Python usadas na análise do trabalho acadêmico. O diretório `results/` contém os CSVs das medições originais e o gráfico exibido no README principal. **Logs de coleta e registros do ambiente foram omitidos do pacote público** por conterem caminhos locais da máquina; eles são regenerados quando a bancada é executada.

## Reproduzir as verificações

A partir da **raiz do repositório**, com Python 3 e `g++` instalados:

```bash
python experiments/preparar.py
```

O comando compila bancadas de tempo e contadores, testes dos TADs, verificador do programa e calibração do relógio; também registra localmente hashes e o ambiente de execução. Se o código for modificado, execute novamente `preparar.py` antes de coletar novos dados.

## Fazer novas medições

```bash
python experiments/medir.py tempos 5
python experiments/medir.py contadores 1
python experiments/medir.py extras 5
python experiments/resumir.py
```

A instalação de `matplotlib` é **opcional**: o gerador utiliza essa biblioteca para produzir gráficos. O relatório pode ser produzido sem ela. Os números resultantes dependem do hardware, SO, compilador e demais condições de execução; não interprete as medições do pacote como garantia em outras máquinas. A coleta local pode atualizar CSVs de `results/`.
