from pathlib import Path
import hashlib
import json
import os
import platform
import shutil
import subprocess
import sys

PASTA = Path(__file__).resolve().parent  # encontra a pasta deste script

# Define a extensão dos executáveis conforme o sistema operacional.
if os.name == 'nt':
    EXT = '.exe'
else:
    EXT = ''


# Executa um comando e informa erro se ele não terminar com sucesso.
def executar(comando, **kwargs):
    # Fluxo: converter argumentos para texto -> executar comando -> conferir sucesso.

    argumentos = []

    for a in comando:
        argumentos.append(str(a))

    # kwargs permite receber opções adicionais para a execução.
    subprocess.run(
        argumentos,
        cwd=PASTA,
        check=True,
        **kwargs
    )


# Calcula uma assinatura de cada arquivo para identificar alterações no conteúdo.
def hashes(caminhos, raiz):
    # Fluxo: percorrer arquivos -> calcular assinaturas -> devolver resultados.

    assinaturas = {}

    for p in caminhos:
        relativo = p.relative_to(raiz).as_posix()  # caminho a partir da pasta raiz
        assinatura = hashlib.sha256(p.read_bytes()).hexdigest()
        assinaturas[relativo] = assinatura

    return assinaturas


# Compila as bancadas, executa as validações e registra o ambiente utilizado.
def main():
    # Fluxo: localizar TP -> compilar -> validar -> conferir fontes -> salvar registro.

    if len(sys.argv) > 1:
        tp = Path(sys.argv[1]).expanduser().resolve()
    else:
        tp = PASTA.parent  # codigo principal na raiz do repositorio

    # Confere dois arquivos necessários para reconhecer o projeto.
    if not (tp / 'src/main.cpp').is_file() or not (tp / 'include/catalogo.h').is_file():
        sys.exit(
            'Nao encontrei o projeto. Execute: python experiments/preparar.py na raiz do repositorio ou passe o caminho:\n'
            'python preparar.py "caminho completo da pasta TP"'
        )

    compilador = shutil.which('g++')  # procura o compilador disponível no terminal

    if compilador is None:
        sys.exit('g++ nao foi encontrado. Use o mesmo terminal em que compilou o TP.')

    fontes = sorted((tp / 'src').glob('*.cpp'))  # reúne os arquivos de código
    cabecalhos = sorted((tp / 'include').glob('*.h'))

    arquivos_tp = fontes + cabecalhos + [tp / 'Makefile']
    antes = hashes(arquivos_tp, tp)  # registra o conteúdo do TP antes da preparação

    registros = []  # guarda os comandos usados na compilação
    flags = ['-std=c++11', '-O2', '-Wall', '-Wextra', '-pedantic']
    objetos_por_modo = {}  # separa os arquivos objeto de cada versão

    # Compila uma versão sem contadores e outra com contadores.
    for modo, contadores in [('tempo', 0), ('contadores', 1)]:
        pasta_obj = PASTA / 'compilados' / modo
        pasta_obj.mkdir(parents=True, exist_ok=True)

        objetos = []

        print('Compilando os modulos do TP para ' + modo + '...', flush=True)

        for fonte in fontes:
            # O main do TP será usado separadamente no executável de validação.
            if fonte.name == 'main.cpp':
                continue

            objeto = pasta_obj / (fonte.stem + '.o')

            # Monta o comando que transforma este arquivo de código em objeto.
            cmd = [compilador]
            cmd.extend(flags)
            cmd.append('-DFORJA_CONTADORES=' + str(contadores))
            cmd.extend([
                '-I',
                str(tp / 'include'),
                '-c',
                str(fonte),
                '-o',
                str(objeto)
            ])

            executar(cmd)
            registros.append(cmd)
            objetos.append(objeto)

        objetos_por_modo[modo] = objetos

    # Compila o arquivo principal indicado e reúne os objetos em um executável.
    def linkar(nome, fonte, modo='tempo', usa_tp=True):
        # Fluxo: escolher contadores -> montar comando -> juntar objetos -> gerar executável.

        if modo == 'contadores':
            contadores = 1
        else:
            contadores = 0

        cmd = [compilador]
        cmd.extend(flags)
        cmd.append('-DFORJA_CONTADORES=' + str(contadores))

        # Informa as pastas dos cabeçalhos e o arquivo principal desta bancada.
        cmd.extend([
            '-I',
            str(tp / 'include'),
            '-I',
            str(PASTA),
            str(fonte)
        ])

        if fonte.name == 'experimentos.cpp':
            cmd.append(str(PASTA / 'produtos_auxiliares.cpp'))

        # Acrescenta os módulos do TP quando o executável precisa deles.
        if usa_tp:
            for o in objetos_por_modo[modo]:
                cmd.append(str(o))

        cmd.extend(['-o', str(PASTA / (nome + EXT))])

        executar(cmd)
        registros.append(cmd)

    print('Gerando bancadas e verificadores...', flush=True)

    # Gera as bancadas de tempo, contadores e funcionalidades extras.
    linkar('bancada_tempo', PASTA / 'experimentos.cpp')
    linkar('bancada_contadores', PASTA / 'experimentos.cpp', 'contadores')
    linkar('bancada_extras', PASTA / 'experimentos_extras.cpp')

    # Gera os programas usados para validar os TADs e a saída do TP.
    linkar('validar_tads', PASTA / 'validar_tads.cpp', 'contadores')
    linkar('tp_validacao', tp / 'src/main.cpp')

    # O teste do relógio não precisa dos módulos do TP.
    linkar('calibrar_relogio', PASTA / 'calibrar_relogio.cpp', usa_tp=False)

    print('Verificando os TADs e a saida do programa...', flush=True)

    executar([PASTA / ('validar_tads' + EXT)])

    executar([
        sys.executable,
        PASTA / 'verificar_programa.py',
        PASTA / ('tp_validacao' + EXT)
    ])

    # Executa a calibração e guarda o texto produzido pelo programa.
    relogio = subprocess.check_output(
        [str(PASTA / ('calibrar_relogio' + EXT))],
        text=True
    )

    print(relogio, end='')

    # Confere se os arquivos do TP permaneceram iguais durante a preparação.
    if hashes(arquivos_tp, tp) != antes:
        sys.exit('Os fontes do TP mudaram durante a preparacao. Execute novamente.')

    resultado = PASTA / 'resultados'
    resultado.mkdir(exist_ok=True)

    # Reúne as informações necessárias para identificar o ambiente e os arquivos usados.
    dados = {
        'tp': str(tp),
        'sistema': platform.platform(),
        'python': platform.python_version()
    }

    processador = platform.processor()

    if not processador:
        processador = os.environ.get('PROCESSOR_IDENTIFIER', '')

    dados['processador'] = processador

    versao_compilador = subprocess.check_output(
        [compilador, '--version'],
        text=True
    )

    dados['compilador'] = versao_compilador.splitlines()[0]
    dados['relogio'] = relogio
    dados['flags'] = flags
    dados['fontes_tp_sha256'] = antes

    # Registra também as assinaturas dos códigos e cabeçalhos da bancada.
    arquivos_bancada = (
        sorted(PASTA.glob('*.cpp')) +
        sorted(PASTA.glob('*.h'))
    )

    dados['bancada_sha256'] = hashes(arquivos_bancada, PASTA)
    dados['comandos_compilacao'] = registros
    dados['observacao'] = (
        'As estrategias alternativas ficam na bancada. Nenhum fonte de TP foi alterado.'
    )

    # Salva as informações do ambiente, das versões e dos comandos de compilação.
    (resultado / 'ambiente_e_versao.json').write_text(
        json.dumps(dados, indent=2, ensure_ascii=False),
        encoding='utf-8'
    )

    # Salva separadamente o resultado da calibração do relógio.
    (resultado / 'relogio.txt').write_text(relogio, encoding='utf-8')

    print('PREPARACAO CONCLUIDA. O projeto TP permaneceu intacto.')
    print('As bancadas e seus arquivos objeto estao somente em analise_TP.')


# Inicia a preparação quando este arquivo é executado diretamente.
if __name__ == '__main__':
    try:
        main()
    except subprocess.CalledProcessError as erro:
        # Informa qual código de erro foi devolvido pelo comando que falhou.
        sys.exit(
            'A preparacao parou: um comando falhou (codigo ' +
            str(erro.returncode) +
            '). Envie a mensagem acima.'
        )