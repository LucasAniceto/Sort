#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <locale.h>

#define MAX_SIZE 20
#define USE_SIMPLE_CHARS 1 // Mude para 0 para usar caracteres especiais

// Configurações de velocidade
typedef enum {
    SPEED_VERY_SLOW = 0,
    SPEED_SLOW = 1,
    SPEED_NORMAL = 2,
    SPEED_FAST = 3,
    SPEED_VERY_FAST = 4
} SpeedLevel;

int speed_delays[] = {800000, 500000, 300000, 150000, 50000}; // microsegundos
char* speed_names[] = {"Muito Lento", "Lento", "Normal", "Rápido", "Muito Rápido"};

typedef struct {
    int arr[MAX_SIZE];
    int size;
    int comparing[2];
    int swapping[2];
    int sorted_until;
    char status[100];
    int comparisons;
    int swaps;
    SpeedLevel speed;
} SortState;

// Cores para visualização
void init_colors() {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);   // Normal
    init_pair(2, COLOR_RED, COLOR_BLACK);     // Comparando
    init_pair(3, COLOR_GREEN, COLOR_BLACK);   // Trocando
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);  // Pivot (quicksort)
    init_pair(5, COLOR_CYAN, COLOR_BLACK);    // Ordenado
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Destaque
    init_pair(7, COLOR_BLUE, COLOR_BLACK);    // Informativo
}

// Limpa estado de comparação/troca
void reset_highlights(SortState *state) {
    state->comparing[0] = -1;
    state->comparing[1] = -1;
    state->swapping[0] = -1;
    state->swapping[1] = -1;
}

// Função para obter delay atual
int get_current_delay(SortState *state) {
    return speed_delays[state->speed];
}

// Mostra controles de velocidade
void show_speed_controls(SortState *state, int y) {
    attron(COLOR_PAIR(7));
    mvprintw(y, 5, "Velocidade: [<] %s [>] | ESC: Menu | SPACE: Pausar/Continuar", 
             speed_names[state->speed]);
    attroff(COLOR_PAIR(7));
}

// Desenha o array na tela
void draw_array(SortState *state, int start_y) {
    int max_val = 0;
    for (int i = 0; i < state->size; i++) {
        if (state->arr[i] > max_val) max_val = state->arr[i];
    }
    
    // Limpa área de visualização
    for (int i = 0; i < 20; i++) {
        move(start_y + i, 0);
        clrtoeol();
    }
    
    // Título
    attron(A_BOLD);
    mvprintw(start_y - 2, 5, "Visualizacao do Array:");
    attroff(A_BOLD);
    
    // Desenha as barras verticais
    for (int i = 0; i < state->size; i++) {
        int height = (state->arr[i] * 12) / max_val;
        if (height == 0) height = 1;
        
        // Define cor da barra
        int color = 1;
        if (state->comparing[0] == i || state->comparing[1] == i) {
            color = 2;
        } else if (state->swapping[0] == i || state->swapping[1] == i) {
            color = 3;
        } else if (i < state->sorted_until) {
            color = 5;
        }
        
        // Calcula posição X para melhor espaçamento
        int x_pos = 5 + (i * 5);
        
        // Desenha a barra vertical
        attron(COLOR_PAIR(color));
        
        #if USE_SIMPLE_CHARS
        // Versão com caracteres simples
        for (int j = 0; j < height; j++) {
            move(start_y + 12 - j, x_pos);
            printw("[#]");
        }
        #else
        // Versão com caracteres especiais
        for (int j = 0; j < height; j++) {
            move(start_y + 12 - j, x_pos);
            if (j == height - 1) {
                printw("╔═╗");
            } else if (j == 0) {
                printw("╚═╝");
            } else {
                printw("║ ║");
            }
        }
        #endif
        
        attroff(COLOR_PAIR(color));
        
        // Mostra o valor abaixo da barra
        move(start_y + 13, x_pos);
        if (state->comparing[0] == i || state->comparing[1] == i) {
            attron(COLOR_PAIR(2) | A_BOLD);
        } else if (state->swapping[0] == i || state->swapping[1] == i) {
            attron(COLOR_PAIR(3) | A_BOLD);
        }
        printw("%3d", state->arr[i]);
        attroff(COLOR_PAIR(2) | COLOR_PAIR(3) | A_BOLD);
        
        // Mostra o índice
        move(start_y + 14, x_pos + 1);
        attron(A_DIM);
        printw("%d", i);
        attroff(A_DIM);
    }
    
    // Linha separadora
    move(start_y + 15, 3);
    for (int i = 0; i < state->size * 5 + 2; i++) {
        printw("-");
    }
    
    // Mostra status e estatísticas
    attron(COLOR_PAIR(6) | A_BOLD);
    move(start_y + 17, 5);
    printw("> Status: %s", state->status);
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    move(start_y + 18, 5);
    printw("Comparacoes: ");
    attron(COLOR_PAIR(2));
    printw("%d", state->comparisons);
    attroff(COLOR_PAIR(2));
    printw(" | Trocas: ");
    attron(COLOR_PAIR(3));
    printw("%d", state->swaps);
    attroff(COLOR_PAIR(3));
    
    // Mostra controles de velocidade
    show_speed_controls(state, start_y + 19);
    
    refresh();
}

// Função para aguardar com possibilidade de controle
int wait_with_controls(SortState *state, int start_y) {
    int delay = get_current_delay(state);
    int steps = delay / 10000; // Divide em pequenos passos para responsividade
    
    for (int i = 0; i < steps; i++) {
        usleep(10000);
        
        // Verifica se há tecla pressionada (não bloqueante)
        nodelay(stdscr, TRUE);
        int ch = getch();
        nodelay(stdscr, FALSE);
        
        if (ch != ERR) {
            switch (ch) {
                case 27: // ESC
                    return -1; // Sinal para voltar ao menu
                case ' ': // SPACE - pausa
                    attron(COLOR_PAIR(6));
                    mvprintw(0, 5, "PAUSADO - Pressione SPACE para continuar ou ESC para menu");
                    attroff(COLOR_PAIR(6));
                    refresh();
                    
                    while (1) {
                        int pause_ch = getch();
                        if (pause_ch == ' ') {
                            move(0, 0);
                            clrtoeol();
                            refresh();
                            break;
                        } else if (pause_ch == 27) {
                            return -1;
                        }
                    }
                    break;
                case '<':
                case ',':
                    if (state->speed > SPEED_VERY_SLOW) {
                        state->speed--;
                        show_speed_controls(state, start_y + 19);
                        refresh();
                    }
                    break;
                case '>':
                case '.':
                    if (state->speed < SPEED_VERY_FAST) {
                        state->speed++;
                        show_speed_controls(state, start_y + 19);
                        refresh();
                    }
                    break;
            }
        }
    }
    return 0;
}

// Mostra informações detalhadas sobre algoritmo
void show_detailed_algorithm_info(const char *name, const char *description, 
                                const char *how_it_works, const char *complexity, 
                                const char *advantages, const char *disadvantages,
                                const char *use_cases, const char *variants) {
    clear();
    int y = 0;
    
    // Título
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(y++, 5, "=== %s - GUIA COMPLETO ===", name);
    attroff(COLOR_PAIR(6) | A_BOLD);
    y++;
    
    // Descrição
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "DESCRIÇÃO:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "%s", description);
    y++;
    
    // Como funciona
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "COMO FUNCIONA:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "%s", how_it_works);
    y++;
    
    // Complexidade
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "COMPLEXIDADE:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "%s", complexity);
    y++;
    
    // Vantagens
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y++, 5, "VANTAGENS:");
    attroff(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y++, 5, "%s", advantages);
    y++;
    
    // Desvantagens
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y++, 5, "DESVANTAGENS:");
    attroff(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y++, 5, "%s", disadvantages);
    y++;
    
    // Casos de uso
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y++, 5, "QUANDO USAR:");
    attroff(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y++, 5, "%s", use_cases);
    y++;
    
    // Variantes
    if (strlen(variants) > 0) {
        attron(COLOR_PAIR(7) | A_BOLD);
        mvprintw(y++, 5, "VARIANTES:");
        attroff(COLOR_PAIR(7) | A_BOLD);
        mvprintw(y++, 5, "%s", variants);
        y++;
    }
    
    attron(COLOR_PAIR(6));
    mvprintw(y + 1, 5, "Pressione ENTER para ver a visualização ou ESC para voltar...");
    attroff(COLOR_PAIR(6));
    
    refresh();
    
    int ch = getch();
    clear();
    return;
}

// Bubble Sort
int bubble_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "BUBBLE SORT",
        "Um dos algoritmos mais simples, que compara elementos adjacentes\n     e os troca se estiverem fora de ordem.",
        "1. Percorre o array comparando pares adjacentes\n     2. Troca elementos se o primeiro for maior que o segundo\n     3. Repete até que nenhuma troca seja necessária\n     4. A cada passada, o maior elemento 'borbulha' para o final",
        "• Melhor caso: O(n) - array já ordenado\n     • Caso médio: O(n²)\n     • Pior caso: O(n²) - array em ordem decrescente\n     • Espaço: O(1) - ordenação in-place",
        "• Implementação muito simples\n     • Estável (mantém ordem relativa de elementos iguais)\n     • Detecta quando o array já está ordenado\n     • Funciona bem como ferramenta educacional",
        "• Extremamente ineficiente para arrays grandes\n     • Faz muitas comparações desnecessárias\n     • Desempenho sempre quadrático (exceto melhor caso)",
        "• Ensino de algoritmos básicos\n     • Arrays muito pequenos (< 10 elementos)\n     • Situações onde simplicidade é mais importante que eficiência",
        "• Cocktail Sort (bidirecional)\n     • Odd-Even Sort (para computação paralela)"
    );
    
    strcpy(state->status, "Iniciando Bubble Sort...");
    state->comparisons = 0;
    state->swaps = 0;
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 0; i < state->size - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < state->size - i - 1; j++) {
            reset_highlights(state);
            state->comparing[0] = j;
            state->comparing[1] = j + 1;
            state->comparisons++;
            
            sprintf(state->status, "Passada %d: Comparando posições %d e %d (valores: %d e %d)", 
                    i + 1, j, j + 1, state->arr[j], state->arr[j + 1]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            if (state->arr[j] > state->arr[j + 1]) {
                reset_highlights(state);
                state->swapping[0] = j;
                state->swapping[1] = j + 1;
                
                int temp = state->arr[j];
                state->arr[j] = state->arr[j + 1];
                state->arr[j + 1] = temp;
                state->swaps++;
                swapped = 1;
                
                sprintf(state->status, "Trocando %d com %d", state->arr[j + 1], state->arr[j]);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
        
        reset_highlights(state);
        state->sorted_until = state->size - i - 1;
        sprintf(state->status, "Fim da passada %d. Maior elemento na posição correta.", i + 1);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (!swapped) {
            sprintf(state->status, "Nenhuma troca realizada. Array já está ordenado!");
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            break;
        }
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Bubble Sort concluído!");
    draw_array(state, start_y);
    return 0;
}

// Selection Sort
int selection_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "SELECTION SORT",
        "Encontra o menor elemento do array e o coloca na primeira posição,\n     depois repete para o restante do array.",
        "1. Encontra o menor elemento do array inteiro\n     2. Troca com o primeiro elemento\n     3. Encontra o menor do restante (excluindo o primeiro)\n     4. Troca com o segundo elemento\n     5. Repete até ordenar todo o array",
        "• Melhor caso: O(n²)\n     • Caso médio: O(n²)\n     • Pior caso: O(n²)\n     • Espaço: O(1) - ordenação in-place\n     • Sempre faz exatamente n-1 trocas",
        "• Minimiza o número de trocas (apenas n-1)\n     • Desempenho consistente (sempre O(n²))\n     • Simples de implementar\n     • Funciona bem quando trocas são custosas",
        "• Ineficiente para arrays grandes\n     • Não é estável\n     • Não se beneficia de dados parcialmente ordenados\n     • Sempre faz O(n²) comparações",
        "• Arrays pequenos onde trocas são custosas\n     • Situações onde consistência de performance é importante\n     • Memória limitada (usa apenas O(1) espaço extra)",
        "• Heap Sort (versão otimizada usando heap)\n     • Bidirectional Selection Sort"
    );
    
    strcpy(state->status, "Iniciando Selection Sort...");
    state->comparisons = 0;
    state->swaps = 0;
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 0; i < state->size - 1; i++) {
        int min_idx = i;
        reset_highlights(state);
        state->comparing[0] = i;
        
        sprintf(state->status, "Procurando o menor elemento a partir da posição %d", i);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        for (int j = i + 1; j < state->size; j++) {
            reset_highlights(state);
            state->comparing[0] = min_idx;
            state->comparing[1] = j;
            state->comparisons++;
            
            sprintf(state->status, "Comparando posição %d (valor: %d) com posição %d (valor: %d)", 
                    min_idx, state->arr[min_idx], j, state->arr[j]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            if (state->arr[j] < state->arr[min_idx]) {
                min_idx = j;
                sprintf(state->status, "Novo menor encontrado: %d na posição %d", state->arr[j], j);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
        
        if (min_idx != i) {
            reset_highlights(state);
            state->swapping[0] = i;
            state->swapping[1] = min_idx;
            
            int temp = state->arr[i];
            state->arr[i] = state->arr[min_idx];
            state->arr[min_idx] = temp;
            state->swaps++;
            
            sprintf(state->status, "Trocando posição %d (valor: %d) com posição %d (valor: %d)", 
                    i, state->arr[min_idx], min_idx, state->arr[i]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
        }
        
        reset_highlights(state);
        state->sorted_until = i + 1;
        sprintf(state->status, "Posição %d ordenada com valor %d", i, state->arr[i]);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Selection Sort concluído!");
    draw_array(state, start_y);
    return 0;
}

// Insertion Sort
int insertion_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "INSERTION SORT",
        "Constrói o array ordenado um elemento por vez, inserindo cada\n     novo elemento na posição correta entre os já ordenados.",
        "1. Considera o primeiro elemento como ordenado\n     2. Pega o próximo elemento (chave)\n     3. Compara com elementos à esquerda\n     4. Move elementos maiores para a direita\n     5. Insere a chave na posição correta\n     6. Repete até o final do array",
        "• Melhor caso: O(n) - array já ordenado\n     • Caso médio: O(n²)\n     • Pior caso: O(n²) - array em ordem decrescente\n     • Espaço: O(1) - ordenação in-place",
        "• Muito eficiente para arrays pequenos\n     • Excelente para dados quase ordenados\n     • Estável e adaptativo\n     • Ordenação online (pode ordenar conforme recebe dados)\n     • Simples de implementar",
        "• Ineficiente para arrays grandes\n     • O(n²) comparações no pior caso\n     • Muitas movimentações de elementos",
        "• Arrays pequenos (< 50 elementos)\n     • Dados quase ordenados ou parcialmente ordenados\n     • Como sub-rotina em algoritmos híbridos (ex: Timsort)\n     • Ordenação incremental de dados que chegam aos poucos",
        "• Binary Insertion Sort (usa busca binária)\n     • Shell Sort (versão com gaps)\n     • Library Sort"
    );
    
    strcpy(state->status, "Iniciando Insertion Sort...");
    state->comparisons = 0;
    state->swaps = 0;
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 1; i < state->size; i++) {
        int key = state->arr[i];
        int j = i - 1;
        
        reset_highlights(state);
        state->comparing[0] = i;
        sprintf(state->status, "Inserindo elemento %d (posição %d) na posição correta", key, i);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        while (j >= 0 && state->arr[j] > key) {
            reset_highlights(state);
            state->comparing[0] = j;
            state->comparing[1] = j + 1;
            state->comparisons++;
            
            sprintf(state->status, "Comparando %d com %d - Movendo %d para a direita", 
                    state->arr[j], key, state->arr[j]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            state->arr[j + 1] = state->arr[j];
            state->swaps++;
            
            reset_highlights(state);
            state->swapping[0] = j;
            state->swapping[1] = j + 1;
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            j--;
        }
        
        state->arr[j + 1] = key;
        reset_highlights(state);
        state->sorted_until = i + 1;
        
        sprintf(state->status, "Elemento %d inserido na posição %d", key, j + 1);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Insertion Sort concluído!");
    draw_array(state, start_y);
    return 0;
}

// Partição do QuickSort
int partition(SortState *state, int low, int high, int start_y) {
    int pivot = state->arr[high];
    int i = low - 1;
    
    reset_highlights(state);
    state->comparing[0] = high; // Pivot
    sprintf(state->status, "Pivot escolhido: %d (posição %d)", pivot, high);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int j = low; j < high; j++) {
        reset_highlights(state);
        state->comparing[0] = high; // Pivot
        state->comparing[1] = j;
        state->comparisons++;
        
        sprintf(state->status, "Comparando %d com pivot %d", state->arr[j], pivot);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (state->arr[j] < pivot) {
            i++;
            if (i != j) {
                reset_highlights(state);
                state->swapping[0] = i;
                state->swapping[1] = j;
                
                int temp = state->arr[i];
                state->arr[i] = state->arr[j];
                state->arr[j] = temp;
                state->swaps++;
                
                sprintf(state->status, "Trocando %d (pos %d) com %d (pos %d)", 
                        state->arr[j], i, state->arr[i], j);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
    }
    
    reset_highlights(state);
    state->swapping[0] = i + 1;
    state->swapping[1] = high;
    
    int temp = state->arr[i + 1];
    state->arr[i + 1] = state->arr[high];
    state->arr[high] = temp;
    state->swaps++;
    
    sprintf(state->status, "Colocando pivot %d na posição correta: %d", pivot, i + 1);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    reset_highlights(state);
    return i + 1;
}

// QuickSort recursivo
int quicksort_recursive(SortState *state, int low, int high, int start_y) {
    if (low < high) {
        sprintf(state->status, "Particionando subarray de índice %d a %d", low, high);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        int pi = partition(state, low, high, start_y);
        if (pi == -1) return -1; // ESC pressionado
        
        sprintf(state->status, "Partição completa. Pivot na posição %d", pi);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (quicksort_recursive(state, low, pi - 1, start_y) == -1) return -1;
        if (quicksort_recursive(state, pi + 1, high, start_y) == -1) return -1;
    }
    return 0;
}

// QuickSort wrapper
int quicksort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "QUICK SORT",
        "Algoritmo de divisão e conquista que escolhe um pivot e particiona\n     o array: menores à esquerda, maiores à direita.",
        "1. Escolhe um elemento como pivot (geralmente o último)\n     2. Reorganiza o array: menores que pivot à esquerda, maiores à direita\n     3. Coloca o pivot na posição final correta\n     4. Aplica recursivamente nas duas partições\n     5. Combina os resultados (já estão ordenados)",
        "• Melhor caso: O(n log n) - pivot sempre divide o array ao meio\n     • Caso médio: O(n log n)\n     • Pior caso: O(n²) - pivot sempre é o menor ou maior\n     • Espaço: O(log n) - devido à recursão",
        "• Muito rápido na prática (um dos mais usados)\n     • Ordenação in-place\n     • Divide e conquista é elegante\n     • Boa localidade de cache",
        "• Pior caso é O(n²)\n     • Não é estável\n     • Desempenho depende muito da escolha do pivot\n     • Pode causar stack overflow (muita recursão)",
        "• Arrays grandes (> 1000 elementos)\n     • Quando espaço extra é limitado\n     • Dados com distribuição aleatória\n     • Implementações de bibliotecas padrão",
        "• 3-way QuickSort (lida melhor com elementos duplicados)\n     • Randomized QuickSort (escolha aleatória do pivot)\n     • Dual-Pivot QuickSort (usado no Java)"
    );
    
    strcpy(state->status, "Iniciando QuickSort...");
    state->comparisons = 0;
    state->swaps = 0;
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    if (quicksort_recursive(state, 0, state->size - 1, start_y) == -1) return -1;
    
    strcpy(state->status, "QuickSort concluído!");
    draw_array(state, start_y);
    return 0;
}

// Menu de configuração de velocidade
void speed_menu(SortState *state) {
    clear();
    attron(COLOR_PAIR(6));
    mvprintw(2, 10, "=== CONFIGURAÇÃO DE VELOCIDADE ===");
    attroff(COLOR_PAIR(6));
    
    mvprintw(4, 5, "Escolha a velocidade de visualização:");
    
    for (int i = 0; i <= SPEED_VERY_FAST; i++) {
        if (i == state->speed) {
            attron(COLOR_PAIR(3) | A_BOLD);
            mvprintw(6 + i, 5, ">>> %d. %s <<<", i + 1, speed_names[i]);
            attroff(COLOR_PAIR(3) | A_BOLD);
        } else {
            mvprintw(6 + i, 5, "    %d. %s", i + 1, speed_names[i]);
        }
    }
    
    mvprintw(12, 5, "Durante a visualização você também pode usar:");
    mvprintw(13, 5, "• < ou , : Diminuir velocidade");
    mvprintw(14, 5, "• > ou . : Aumentar velocidade");
    mvprintw(15, 5, "• SPACE  : Pausar/Continuar");
    mvprintw(16, 5, "• ESC    : Voltar ao menu");
    
    attron(COLOR_PAIR(2));
    mvprintw(18, 5, "Pressione 1-5 para escolher ou ESC para voltar...");
    attroff(COLOR_PAIR(2));
    
    refresh();
    
    int ch = getch();
    if (ch >= '1' && ch <= '5') {
        state->speed = ch - '1';
    }
}

// Menu de seleção de algoritmo para estudo
int algorithm_study_menu() {
    clear();
    attron(COLOR_PAIR(6));
    mvprintw(2, 10, "=== GUIA DE ESTUDO DOS ALGORITMOS ===");
    attroff(COLOR_PAIR(6));
    
    mvprintw(4, 5, "Selecione um algoritmo para estudar em detalhes:");
    
    mvprintw(6, 5, "1. Bubble Sort - O algoritmo mais simples");
    mvprintw(7, 5, "2. Selection Sort - Minimiza o número de trocas");
    mvprintw(8, 5, "3. Insertion Sort - Ótimo para dados quase ordenados");
    mvprintw(9, 5, "4. Quick Sort - Muito rápido na prática");
    mvprintw(10, 5, "5. Comparação entre todos os algoritmos");
    mvprintw(11, 5, "0. Voltar ao menu principal");
    
    attron(COLOR_PAIR(3));
    mvprintw(13, 5, "Opção: ");
    attroff(COLOR_PAIR(3));
    refresh();
    
    return getch() - '0';
}

// Mostra comparação detalhada entre algoritmos
void show_algorithms_comparison() {
    clear();
    attron(COLOR_PAIR(6));
    mvprintw(0, 15, "=== COMPARAÇÃO ENTRE ALGORITMOS DE ORDENAÇÃO ===");
    attroff(COLOR_PAIR(6));
    
    int y = 2;
    
    // Tabela de complexidades
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 2, "COMPLEXIDADE DE TEMPO:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 2, "┌────────────────┬────────────┬────────────┬────────────┐");
    mvprintw(y++, 2, "│ Algoritmo      │ Melhor     │ Médio      │ Pior       │");
    mvprintw(y++, 2, "├────────────────┼────────────┼────────────┼────────────┤");
    mvprintw(y++, 2, "│ Bubble Sort    │ O(n)       │ O(n²)      │ O(n²)      │");
    mvprintw(y++, 2, "│ Selection Sort │ O(n²)      │ O(n²)      │ O(n²)      │");
    mvprintw(y++, 2, "│ Insertion Sort │ O(n)       │ O(n²)      │ O(n²)      │");
    mvprintw(y++, 2, "│ Quick Sort     │ O(n log n) │ O(n log n) │ O(n²)      │");
    mvprintw(y++, 2, "└────────────────┴────────────┴────────────┴────────────┘");
    y++;
    
    // Características
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y++, 2, "CARACTERÍSTICAS PRINCIPAIS:");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "BUBBLE SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "• Mais fácil de entender • Detecta array ordenado • Muito lento");
    y++;
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "SELECTION SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "• Poucas trocas (n-1) • Performance consistente • Não é estável");
    y++;
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "INSERTION SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "• Excelente para dados quase ordenados • Estável • Ordenação online");
    y++;
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "QUICK SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "• Mais rápido na prática • In-place • Pode ser O(n²) no pior caso");
    y++;
    
    // Recomendações
    attron(COLOR_PAIR(7) | A_BOLD);
    mvprintw(y++, 2, "QUANDO USAR CADA UM:");
    attroff(COLOR_PAIR(7) | A_BOLD);
    mvprintw(y++, 2, "• Arrays pequenos (< 10): Insertion Sort");
    mvprintw(y++, 2, "• Arrays médios (10-50): Insertion Sort ou Quick Sort");
    mvprintw(y++, 2, "• Arrays grandes (> 50): Quick Sort");
    mvprintw(y++, 2, "• Dados quase ordenados: Insertion Sort");
    mvprintw(y++, 2, "• Minimizar trocas: Selection Sort");
    mvprintw(y++, 2, "• Aprendizado: Bubble Sort");
    
    mvprintw(y + 2, 2, "Pressione qualquer tecla para voltar...");
    refresh();
    getch();
}

// Menu principal
int show_menu(SortState *state) {
    clear();
    attron(COLOR_PAIR(6));
    mvprintw(2, 10, "=== VISUALIZADOR DE ALGORITMOS DE ORDENAÇÃO ===");
    attroff(COLOR_PAIR(6));
    
    mvprintw(4, 5, "Uma ferramenta educacional para aprender algoritmos de ordenação");
    
    mvprintw(6, 5, "Escolha uma opção:");
    mvprintw(8, 5, "1. Bubble Sort");
    mvprintw(9, 5, "2. Selection Sort");
    mvprintw(10, 5, "3. Insertion Sort");
    mvprintw(11, 5, "4. Quick Sort");
    mvprintw(12, 5, "5. Gerar novo array aleatório");
    mvprintw(13, 5, "6. Inserir array manualmente");
    mvprintw(14, 5, "7. Configurar velocidade");
    mvprintw(15, 5, "8. Guia de estudo dos algoritmos");
    mvprintw(16, 5, "0. Sair");
    
    attron(COLOR_PAIR(3));
    mvprintw(18, 5, "Legenda: ");
    
    attron(COLOR_PAIR(2));
    printw("[#] Comparando  ");
    attroff(COLOR_PAIR(2));
    
    attron(COLOR_PAIR(3));
    printw("[#] Trocando  ");
    attroff(COLOR_PAIR(3));
    
    attron(COLOR_PAIR(5));
    printw("[#] Ordenado");
    attroff(COLOR_PAIR(5));
    
    mvprintw(20, 5, "Velocidade atual: %s", speed_names[state->speed]);
    mvprintw(21, 5, "Opção: ");
    refresh();
    
    return getch() - '0';
}

// Gera array aleatório
void generate_random_array(SortState *state) {
    clear();
    mvprintw(5, 5, "Digite o tamanho do array (1-%d): ", MAX_SIZE);
    refresh();
    
    echo();
    scanw("%d", &state->size);
    noecho();
    
    if (state->size < 1) state->size = 1;
    if (state->size > MAX_SIZE) state->size = MAX_SIZE;
    
    srand(time(NULL));
    for (int i = 0; i < state->size; i++) {
        state->arr[i] = rand() % 100 + 1;
    }
    
    mvprintw(7, 5, "Array gerado com sucesso!");
    mvprintw(8, 5, "Valores: ");
    for (int i = 0; i < state->size; i++) {
        printw("%d ", state->arr[i]);
    }
    mvprintw(10, 5, "Pressione qualquer tecla para continuar...");
    refresh();
    getch();
}

// Entrada manual do array
void input_manual_array(SortState *state) {
    clear();
    mvprintw(5, 5, "Digite o tamanho do array (1-%d): ", MAX_SIZE);
    refresh();
    
    echo();
    scanw("%d", &state->size);
    
    if (state->size < 1) state->size = 1;
    if (state->size > MAX_SIZE) state->size = MAX_SIZE;
    
    mvprintw(7, 5, "Digite os %d elementos:", state->size);
    for (int i = 0; i < state->size; i++) {
        mvprintw(8 + i, 5, "Elemento %d: ", i + 1);
        refresh();
        scanw("%d", &state->arr[i]);
    }
    noecho();
    
    mvprintw(9 + state->size, 5, "Array inserido com sucesso!");
    mvprintw(10 + state->size, 5, "Pressione qualquer tecla para continuar...");
    refresh();
    getch();
}

int main() {
    setlocale(LC_ALL, "Portuguese");
    // Inicializa ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    // Verifica suporte a cores
    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal não suporta cores\n");
        return 1;
    }
    
    init_colors();
    
    // Inicializa estado
    SortState state;
    state.size = 10;
    state.speed = SPEED_NORMAL;
    reset_highlights(&state);
    state.sorted_until = 0;
    
    // Gera array inicial
    srand(time(NULL));
    for (int i = 0; i < state.size; i++) {
        state.arr[i] = rand() % 100 + 1;
    }
    
    int choice;
    int original_arr[MAX_SIZE];
    
    while (1) {
        choice = show_menu(&state);
        
        if (choice == 0) break;
        
        if (choice == 7) {
            speed_menu(&state);
            continue;
        }
        
        if (choice == 8) {
            int study_choice = algorithm_study_menu();
            if (study_choice == 5) {
                show_algorithms_comparison();
            } else if (study_choice >= 1 && study_choice <= 4) {
                // Salva array original
                for (int i = 0; i < state.size; i++) {
                    original_arr[i] = state.arr[i];
                }
                
                clear();
                int result = 0;
                
                switch (study_choice) {
                    case 1: result = bubble_sort(&state, 3); break;
                    case 2: result = selection_sort(&state, 3); break;
                    case 3: result = insertion_sort(&state, 3); break;
                    case 4: result = quicksort(&state, 3); break;
                }
                
                if (result != -1) {
                    mvprintw(20, 5, "Pressione 'r' para repetir ou qualquer tecla para voltar ao menu");
                    refresh();
                    
                    int key = getch();
                    if (key == 'r' || key == 'R') {
                        // Restaura array original
                        for (int i = 0; i < state.size; i++) {
                            state.arr[i] = original_arr[i];
                        }
                        state.sorted_until = 0;
                        reset_highlights(&state);
                        clear();
                        
                        // Repete o mesmo algoritmo
                        switch (study_choice) {
                            case 1: bubble_sort(&state, 3); break;
                            case 2: selection_sort(&state, 3); break;
                            case 3: insertion_sort(&state, 3); break;
                            case 4: quicksort(&state, 3); break;
                        }
                        
                        mvprintw(20, 5, "Pressione qualquer tecla para voltar ao menu...");
                        refresh();
                        getch();
                    }
                }
            }
            continue;
        }
        
        // Salva array original para poder repetir
        for (int i = 0; i < state.size; i++) {
            original_arr[i] = state.arr[i];
        }
        
        clear();
        int result = 0;
        
        switch (choice) {
            case 1:
                result = bubble_sort(&state, 3);
                break;
            case 2:
                result = selection_sort(&state, 3);
                break;
            case 3:
                result = insertion_sort(&state, 3);
                break;
            case 4:
                result = quicksort(&state, 3);
                break;
            case 5:
                generate_random_array(&state);
                continue;
            case 6:
                input_manual_array(&state);
                continue;
            default:
                continue;
        }
        
        if (result != -1) {
            mvprintw(20, 5, "Pressione 'r' para repetir com o mesmo array ou qualquer tecla para voltar ao menu");
            refresh();
            
            int key = getch();
            if (key == 'r' || key == 'R') {
                // Restaura array original
                for (int i = 0; i < state.size; i++) {
                    state.arr[i] = original_arr[i];
                }
                state.sorted_until = 0;
                reset_highlights(&state);
                clear();
                
                // Repete o mesmo algoritmo
                switch (choice) {
                    case 1: bubble_sort(&state, 3); break;
                    case 2: selection_sort(&state, 3); break;
                    case 3: insertion_sort(&state, 3); break;
                    case 4: quicksort(&state, 3); break;
                }
                
                mvprintw(20, 5, "Pressione qualquer tecla para voltar ao menu...");
                refresh();
                getch();
            }
        }
    }
    
    endwin();
    return 0;
}