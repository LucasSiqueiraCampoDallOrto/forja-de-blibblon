# Escolhe g++ como compilador C++.
CXX = g++
# Indica a pasta include para localizar os cabeçalhos do projeto.
CPPFLAGS = -Iinclude
# Usa C++11, otimização O2 e avisos de compilação.
CXXFLAGS = -std=c++11 -O2 -Wall -Wextra -pedantic
# Deixa vazias as opções adicionais da etapa de ligação.
LDFLAGS =
# Deixa vazia a lista de bibliotecas extras para a ligação.
LDLIBS =

# Define o nome e o caminho exigidos para o executável final.
TARGET = bin/tp1.out
# Reúne todos os arquivos .cpp da pasta src.
SOURCES = $(wildcard src/*.cpp)
# Calcula o caminho correspondente de cada arquivo objeto dentro de obj.
OBJECTS = $(patsubst src/%.cpp,obj/%.o,$(SOURCES))
# Calcula os nomes dos arquivos .d, que registram dependências de cabeçalhos.
DEPFILES = $(OBJECTS:.o=.d)

# Permite usar mingw32-make no Windows e make no Linux.
# Seleciona os comandos específicos quando o sistema é Windows.
ifeq ($(OS),Windows_NT)
# Usa o interpretador de comandos do Windows para executar as receitas.
SHELL = cmd.exe
# Encerra a seleção de comandos por sistema operacional.
endif

# Informa que all e clean são ações, e não arquivos gerados.
.PHONY: all clean
# Faz make all depender da criação do executável final.
all: $(TARGET)

# Liga novamente o programa quando algum objeto mudar e exige que bin exista.
$(TARGET): $(OBJECTS) | bin
# Reúne os objetos em um executável; $@ representa o arquivo de destino.
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

# -MMD/-MP atualizam as dependencias quando um header e alterado.
# Define como transformar cada fonte src/nome.cpp em obj/nome.o, garantindo a pasta obj.
obj/%.o: src/%.cpp | obj
# Compila um fonte por vez e gera suas dependências; $< é o fonte e $@ é o objeto.
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Define a regra que cria as pastas bin e obj quando necessário.
bin obj:
# Seleciona os comandos específicos quando o sistema é Windows.
ifeq ($(OS),Windows_NT)
# Cria a pasta de destino no Windows somente se ela ainda não existir.
	if not exist "$@" mkdir "$@"
# Seleciona os comandos usados fora do Windows.
else
# Cria a pasta de destino no Linux sem erro se ela já existir.
	mkdir -p $@
# Encerra a seleção de comandos por sistema operacional.
endif

# Define a ação de limpeza dos arquivos produzidos pela compilação.
clean:
# Seleciona os comandos específicos quando o sistema é Windows.
ifeq ($(OS),Windows_NT)
# Apaga os objetos da pasta obj no Windows, caso existam.
	if exist obj\*.o del /Q obj\*.o
# Apaga os arquivos de dependências da pasta obj no Windows, caso existam.
	if exist obj\*.d del /Q obj\*.d
# Apaga o executável da pasta bin no Windows, caso exista.
	if exist bin\tp1.out del /Q bin\tp1.out
# Seleciona os comandos usados fora do Windows.
else
# Remove objetos, dependências e executável nos sistemas que usam rm.
	rm -f $(OBJECTS) $(DEPFILES) $(TARGET)
# Encerra a seleção de comandos por sistema operacional.
endif

# Lê as dependências geradas e ignora a ausência delas antes da primeira compilação.
-include $(DEPFILES)
