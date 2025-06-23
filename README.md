# 🎯 Visualizador de Algoritmos de Ordenação

Uma ferramenta educacional interativa em C para aprender e visualizar algoritmos de ordenação em tempo real no terminal.

## ✨ Características

- **Visualização em tempo real** com barras coloridas e animações
- **4 algoritmos implementados**: Bubble Sort, Selection Sort, Insertion Sort e Quick Sort
- **Controle de velocidade** com 5 níveis diferentes
- **Sistema de logging** para acompanhar cada operação
- **Guia educacional completo** com explicações detalhadas de cada algoritmo
- **Interface interativa** com controles durante a execução
- **Comparação entre algoritmos** com análise de complexidade
- **Arrays personalizáveis** (aleatórios ou inserção manual)

## 🚀 Instalação e Execução

### Pré-requisitos
- GCC (compilador C)
- Biblioteca ncurses
- Terminal com suporte a cores

### Instalação da ncurses

**Ubuntu/Debian:**
```bash
sudo apt-get install libncurses5-dev libncursesw5-dev
```

**CentOS/RHEL/Fedora:**
```bash
sudo yum install ncurses-devel
# ou para Fedora mais recente:
sudo dnf install ncurses-devel
```

**macOS:**
```bash
brew install ncurses
```

### Teste de Compatibilidade de Caracteres

Antes de compilar, teste se seu terminal suporta caracteres especiais:

```bash
echo "╔═══╗ ║ ║ ╚═══╝ → ✓ ★ ◆"
```

**Se os caracteres aparecerem corretamente:** Prossiga para compilação
**Se aparecerem símbolos estranhos:** Execute primeiro:

```bash
export LANG=pt_BR.UTF-8 && export LC_ALL=pt_BR.UTF-8
```

### Compilação e Execução

```bash
# Compilar o programa
gcc *.c -o sort -lncursesw

# Executar
./sort
```

## 🎮 Como Usar

### Menu Principal

Ao iniciar o programa, você verá as seguintes opções:

1. **Bubble Sort** - Algoritmo de ordenação por bolha
2. **Selection Sort** - Ordenação por seleção
3. **Insertion Sort** - Ordenação por inserção  
4. **Quick Sort** - Ordenação rápida
5. **Gerar novo array aleatório** - Cria um array com valores aleatórios
6. **Inserir array manualmente** - Permite inserir valores personalizados
7. **Checar array atual** - Visualiza estatísticas do array atual
8. **Configurar velocidade** - Ajusta a velocidade da animação
9. **Guia de estudo dos algoritmos** - Acesso ao material educacional
0. **Ver log de execução** - Histórico detalhado das operações

### Controles Durante a Visualização

| Tecla | Função |
|-------|--------|
| `ESPAÇO` | Pausar/Continuar animação |
| `<` ou `,` | Diminuir velocidade |
| `>` ou `.` | Aumentar velocidade |
| `L` | Habilitar/Desabilitar log |
| `ESC` | Voltar ao menu principal |

### Legenda de Cores

- 🔴 **Vermelho**: Elementos sendo comparados
- 🟢 **Verde**: Elementos sendo trocados
- 🔵 **Ciano**: Elementos já ordenados
- ⚪ **Branco**: Elementos não processados

## 📚 Algoritmos Implementados

### 1. Bubble Sort
- **Complexidade**: O(n²)
- **Características**: Simples, estável, detecta arrays ordenados
- **Ideal para**: Ensino, arrays muito pequenos

### 2. Selection Sort
- **Complexidade**: O(n²)
- **Características**: Minimiza trocas (n-1), performance consistente
- **Ideal para**: Quando trocas são custosas

### 3. Insertion Sort
- **Complexidade**: O(n) melhor caso, O(n²) pior caso
- **Características**: Excelente para dados quase ordenados, estável
- **Ideal para**: Arrays pequenos, dados parcialmente ordenados

### 4. Quick Sort
- **Complexidade**: O(n log n) médio, O(n²) pior caso
- **Características**: Muito rápido na prática, divide e conquista
- **Ideal para**: Arrays grandes, uso geral

## 🎓 Recursos Educacionais

### Guia de Estudo
- Explicações detalhadas de cada algoritmo
- Análise de complexidade temporal e espacial
- Vantagens e desvantagens
- Casos de uso recomendados
- Variantes e otimizações

### Comparação Interativa
- Tabela comparativa de complexidades
- Recomendações por tamanho de dados
- Análise de características únicas

### Sistema de Log
- Registro de cada operação realizada
- Contadores de comparações e trocas
- Histórico navegável
- Opção de limpar log

## ⚙️ Configurações

### Velocidades Disponíveis
1. **Muito Lento** - 800ms entre operações
2. **Lento** - 500ms entre operações
3. **Normal** - 300ms entre operações (padrão)
4. **Rápido** - 150ms entre operações
5. **Muito Rápido** - 50ms entre operações

### Personalização de Arrays
- **Tamanho**: 1 a 20 elementos
- **Valores**: 1 a 100
- **Modo aleatório**: Geração automática
- **Modo manual**: Inserção personalizada

## 🛠️ Estrutura do Projeto

```
sort_visualizer/
├── main.c              # Programa principal e loop do menu
├── sort_visualizer.h   # Definições e estruturas principais
├── algorithms.c        # Implementação dos algoritmos
├── algorithms.h        # Cabeçalhos dos algoritmos
├── ui.c               # Interface do usuário e visualização
├── ui.h               # Cabeçalhos da interface
├── utils.c            # Funções utilitárias
├── utils.h            # Cabeçalhos das utilidades
└── README.md          # Este arquivo
```

## 🐛 Solução de Problemas

### Caracteres não aparecem corretamente
```bash
export LANG=pt_BR.UTF-8 && export LC_ALL=pt_BR.UTF-8
```

### Erro de compilação relacionado à ncurses
```bash
# Ubuntu/Debian
sudo apt-get install libncurses5-dev libncursesw5-dev

# Verifique se está usando -lncursesw (com 'w')
gcc *.c -o sort -lncursesw
```

### Terminal muito pequeno
- Redimensione o terminal para pelo menos 80x24 caracteres
- Use um terminal em tela cheia para melhor experiência

### Cores não funcionam
- Verifique se seu terminal suporta cores
- Teste com: `echo $TERM`
- Experimente terminais como: xterm-256color, gnome-terminal

## 📖 Exemplo de Uso Educacional

1. **Inicie com Bubble Sort** para entender conceitos básicos
2. **Compare com Selection Sort** para ver diferentes abordagens
3. **Experimente Insertion Sort** com dados quase ordenados
4. **Finalize com Quick Sort** para algoritmos avançados
5. **Use o guia de estudo** para aprofundar conhecimentos
6. **Analise o log** para entender o número de operações

## 🤝 Contribuindo

Este é um projeto educacional. Sugestões de melhorias são bem-vindas:

- Novos algoritmos de ordenação
- Melhorias na interface
- Otimizações de performance
- Correções de bugs
- Tradução para outros idiomas

## 📄 Licença

Projeto educacional de código aberto. Use livremente para fins educacionais.

---

**Desenvolvido para o aprendizado de algoritmos de ordenação** 🎓