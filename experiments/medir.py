from pathlib import Path
from datetime import datetime
import hashlib
import json
import os
import shutil
import subprocess
import sys

PASTA = Path(__file__).resolve().parent  # encontra a pasta deste script
RESULTADOS = PASTA / 'resultados'  # indica onde ficam os resultados

# Associa cada tipo de coleta ao executável e ao número padrão de repetições.
MODOS = {
    'tempos': ('bancada_tempo', 5),
    'contadores': ('bancada_contadores', 1),
    'extras': ('bancada_extras', 5)
}


# Confere os arquivos, executa a bancada e registra os resultados da coleta.
def main():
    # Fluxo: ler opções -> conferir versões -> guardar coleta anterior -> executar -> registrar resultado.

    if len(sys.argv) > 1:
        modo = sys.argv[1]  # usa o tipo de coleta informado no terminal
    else:
        modo = 'tempos'

    if modo not in MODOS:
        sys.exit('Use: python medir.py tempos 5 | contadores 1 | extras 5')

    nome = MODOS[modo][0]  # nome do executável
    padrao = MODOS[modo][1]  # quantidade padrão de repetições

    if len(sys.argv) > 2:
        repeticoes = int(sys.argv[2])
    else:
        repeticoes = padrao

    if repeticoes < 1:
        sys.exit('O numero de repeticoes deve ser positivo.')

    # Localiza o registro criado durante a preparação das bancadas.
    ambiente = RESULTADOS / 'ambiente_e_versao.json'

    if not ambiente.is_file():
        sys.exit('Execute primeiro python preparar.py.')

    dados = json.loads(ambiente.read_text(encoding='utf-8'))
    tp = Path(dados['tp'])  # recupera o caminho do projeto TP

    # Confere se os arquivos do TP continuam iguais aos usados na preparação.
    for relativo, esperado in dados['fontes_tp_sha256'].items():
        arq = tp / relativo

        if not arq.is_file() or hashlib.sha256(arq.read_bytes()).hexdigest() != esperado:
            sys.exit('O projeto TP mudou. Execute preparar.py novamente antes de medir.')

    # Confere se os arquivos da bancada também permanecem iguais.
    for relativo, esperado in dados['bancada_sha256'].items():
        if hashlib.sha256((PASTA / relativo).read_bytes()).hexdigest() != esperado:
            sys.exit('A bancada mudou. Execute preparar.py novamente.')

    # Acrescenta a extensão do executável quando o sistema é Windows.
    if os.name == 'nt':
        extensao = '.exe'
    else:
        extensao = ''

    exe = PASTA / (nome + extensao)

    if not exe.is_file():
        sys.exit('Executavel ausente. Execute preparar.py novamente.')

    # Reúne os arquivos de uma coleta anterior do mesmo tipo.
    anteriores = list(RESULTADOS.glob(modo + '_*.csv'))

    arquivos_adicionais = [
        RESULTADOS / ('coleta_' + modo + '.json'),
        RESULTADOS / ('log_' + modo + '.txt')
    ]

    for p in arquivos_adicionais:
        if p.exists():
            anteriores.append(p)

    # Preserva os resultados anteriores antes de iniciar a nova coleta.
    if anteriores:
        backup = RESULTADOS / 'anteriores' / (
            datetime.now().strftime('%Y%m%d_%H%M%S_%f') + '_' + modo
        )

        backup.mkdir(parents=True)

        for p in anteriores:
            shutil.move(str(p), str(backup / p.name))

        print(
            'A coleta anterior foi preservada em resultados/anteriores.',
            flush=True
        )

    registro = dict(dados)  # copia as informações do ambiente e da versão do TP

    # Acrescenta as informações desta execução ao registro.
    registro.update({
        'coleta': modo,
        'repeticoes': repeticoes,
        'inicio': datetime.now().astimezone().isoformat(),
        'status': 'em_execucao',
        'executavel_sha256': hashlib.sha256(exe.read_bytes()).hexdigest()
    })

    destino = RESULTADOS / ('coleta_' + modo + '.json')

    # Grava o estado atual da coleta no arquivo JSON.
    def salvar():
        # Fluxo: transformar registro em texto JSON -> escrever no arquivo.
        destino.write_text(
            json.dumps(registro, indent=2, ensure_ascii=False),
            encoding='utf-8'
        )

    salvar()

    # Monta o comando com executável, prefixo dos resultados e repetições.
    comando = [
        str(exe),
        str(RESULTADOS / modo),
        str(repeticoes)
    ]

    print(
        'Iniciando coleta ' + modo +
        '. Evite executar outras bancadas ao mesmo tempo.',
        flush=True
    )

    # Abre o arquivo que guardará as mensagens produzidas pela bancada.
    with (RESULTADOS / ('log_' + modo + '.txt')).open(
        'w', encoding='utf-8'
    ) as log:

        # Inicia a bancada e reúne suas mensagens de saída e de erro.
        processo = subprocess.Popen(
            comando,
            cwd=PASTA,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            encoding='utf-8',
            errors='replace'
        )

        try:
            # Mostra cada mensagem no terminal e também a salva no arquivo.
            for linha in processo.stdout:
                print(linha, end='', flush=True)
                log.write(linha)
                log.flush()

            codigo = processo.wait()  # aguarda o encerramento e recebe o código de saída

        except KeyboardInterrupt:
            # Encerra a bancada se o usuário interromper com Ctrl+C.
            processo.terminate()
            processo.wait()

            registro['status'] = 'interrompida'
            salvar()

            sys.exit('Coleta interrompida. Execute o mesmo comando para repetir.')

    registro['fim'] = datetime.now().astimezone().isoformat()

    # Código zero indica que a bancada terminou com sucesso.
    if codigo == 0:
        registro['status'] = 'concluida'
    else:
        registro['status'] = 'falhou'

    registro['codigo_saida'] = codigo
    salvar()

    if codigo != 0:
        sys.exit('A coleta falhou. Envie a mensagem acima.')

    print(
        'COLETA ' + modo.upper() +
        ' CONCLUIDA. Arquivos guardados em resultados.'
    )


# Executa a função principal quando este arquivo é iniciado diretamente.
if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError) as erro:
        # Mostra uma mensagem caso ocorra erro de sistema ou valor inválido.
        sys.exit('Erro: ' + str(erro))