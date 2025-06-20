#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>
#include <stdarg.h>

#define MAX_SIZE 20
#define USE_SIMPLE_CHARS 0 // 0 para usar caracteres UTF-8, 1 para ASCII
#define MAX_LOG_ENTRIES 1000
#define MAX_LOG_LENGTH 200

typedef enum {
    SPEED_VERY_SLOW = 0,
    SPEED_SLOW = 1,
    SPEED_NORMAL = 2,
    SPEED_FAST = 3,
    SPEED_VERY_FAST = 4
} SpeedLevel;

int speed_delays[] = {800000, 500000, 300000, 150000, 50000}; 
char* speed_names[] = {"Muito Lento", "Lento", "Normal", "Rápido", "Muito Rápido"};

typedef struct {
    int arr[MAX_SIZE];
    int size;
    int comparing[2];
    int swapping[2];
    int sorted_until;
    int sorted_positions[MAX_SIZE];
    char status[100];
    int comparisons;
    int swaps;
    SpeedLevel speed;
    char log[MAX_LOG_ENTRIES][MAX_LOG_LENGTH];
    int log_count;
    int log_enabled;
} SortState;

void show_speed_controls(SortState *state, int y);

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

void reset_highlights(SortState *state) {
    state->comparing[0] = -1;
    state->comparing[1] = -1;
    state->swapping[0] = -1;
    state->swapping[1] = -1;
}

void init_sorted_positions(SortState *state) {
    for (int i = 0; i < MAX_SIZE; i++) {
        state->sorted_positions[i] = 0;
    }
}

void init_log(SortState *state) {
    state->log_count = 0;
    state->log_enabled = 1;
}

void add_log_entry(SortState *state, const char* message) {
    if (!state->log_enabled || state->log_count >= MAX_LOG_ENTRIES) return;
    
    strncpy(state->log[state->log_count], message, MAX_LOG_LENGTH - 1);
    state->log[state->log_count][MAX_LOG_LENGTH - 1] = '\0';
    state->log_count++;
}

void set_status(SortState *state, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(state->status, sizeof(state->status), format, args);
    va_end(args);
    
    if (state->log_enabled) {
        add_log_entry(state, state->status);
    }
}

void update_sorted_positions(SortState *state) {
    int sorted_arr[MAX_SIZE];
    for (int i = 0; i < state->size; i++) {
        sorted_arr[i] = state->arr[i];
    }
    
    for (int i = 0; i < state->size - 1; i++) {
        for (int j = 0; j < state->size - i - 1; j++) {
            if (sorted_arr[j] > sorted_arr[j + 1]) {
                int temp = sorted_arr[j];
                sorted_arr[j] = sorted_arr[j + 1];
                sorted_arr[j + 1] = temp;
            }
        }
    }
    
    for (int i = 0; i < state->size; i++) {
        if (state->arr[i] == sorted_arr[i]) {
            state->sorted_positions[i] = 1;
        } else {
            state->sorted_positions[i] = 0;
        }
    }
}

int get_current_delay(SortState *state) {
    return speed_delays[state->speed];
}

void show_speed_controls(SortState *state, int y) {
    attron(COLOR_PAIR(7));
    mvprintw(y, 5, "Velocidade: [<] %s [>] | ESC: Menu | SPACE: Pausar | L: %s Log", 
             speed_names[state->speed], state->log_enabled ? "Desabilitar" : "Habilitar");
    attroff(COLOR_PAIR(7));
}

void show_log(SortState *state) {
    clear();
    
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 5, "=== LOG DE EXECUÇÃO DO ALGORITMO ===");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    if (state->log_count == 0) {
        mvprintw(2, 5, "Nenhuma entrada no log.");
        mvprintw(4, 5, "Execute um algoritmo com o log habilitado para ver o histórico.");
    } else {
        mvprintw(1, 5, "Total de %d operações registradas:", state->log_count);
        
        int start_line = 0;
        int max_lines = LINES - 5; 
        int current_page = 0;
        int total_pages = (state->log_count + max_lines - 1) / max_lines;
        
        while (1) {
            for (int i = 3; i < LINES - 2; i++) {
                move(i, 0);
                clrtoeol();
            }
            
            for (int i = 0; i < max_lines && (start_line + i) < state->log_count; i++) {
                mvprintw(3 + i, 2, "%3d. %s", start_line + i + 1, state->log[start_line + i]);
            }
            
            attron(COLOR_PAIR(7));
            mvprintw(LINES - 2, 5, "Página %d/%d | ↑↓: Navegar | ESC: Voltar | C: Limpar log", 
                     current_page + 1, total_pages);
            attroff(COLOR_PAIR(7));
            
            refresh();
            
            int key = getch();
            switch (key) {
                case 27: // ESC
                    return;
                case 'c':
                case 'C':
                    state->log_count = 0;
                    mvprintw(LINES - 1, 5, "Log limpo!");
                    refresh();
                    sleep(1);
                    return;
                case KEY_UP:
                    if (current_page > 0) {
                        current_page--;
                        start_line = current_page * max_lines;
                    }
                    break;
                case KEY_DOWN:
                    if (current_page < total_pages - 1) {
                        current_page++;
                        start_line = current_page * max_lines;
                    }
                    break;
            }
        }
    }
    
    attron(COLOR_PAIR(6));
    mvprintw(LINES - 1, 5, "Pressione qualquer tecla para voltar...");
    attroff(COLOR_PAIR(6));
    refresh();
    getch();
}

void draw_array(SortState *state, int start_y) {
    int max_val = 0;
    for (int i = 0; i < state->size; i++) {
        if (state->arr[i] > max_val) max_val = state->arr[i];
    }
    update_sorted_positions(state);
    
    for (int i = 0; i < 20; i++) {
        move(start_y + i, 0);
        clrtoeol();
    }
    
    attron(A_BOLD);
    mvprintw(start_y - 2, 5, "Visualização do Array:");
    attroff(A_BOLD);
    
    for (int i = 0; i < state->size; i++) {
        int height = (state->arr[i] * 12) / max_val;
        if (height == 0) height = 1;
        
        int color = 1; 
        
        if (state->comparing[0] == i || state->comparing[1] == i) {
            color = 2; 
        } else if (state->swapping[0] == i || state->swapping[1] == i) {
            color = 3; 
        } else if (state->sorted_positions[i]) {
            color = 5; 
        }
        int x_pos = 5 + (i * 5);
        
        attron(COLOR_PAIR(color));
        
        #if USE_SIMPLE_CHARS
        for (int j = 0; j < height; j++) {
            move(start_y + 12 - j, x_pos);
            printw("[#]");
        }
        #else
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
        
        move(start_y + 13, x_pos);
        if (state->comparing[0] == i || state->comparing[1] == i) {
            attron(COLOR_PAIR(2) | A_BOLD);
        } else if (state->swapping[0] == i || state->swapping[1] == i) {
            attron(COLOR_PAIR(3) | A_BOLD);
        } else if (state->sorted_positions[i]) {
            attron(COLOR_PAIR(5) | A_BOLD);
        }
        printw("%3d", state->arr[i]);
        attroff(COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(5) | A_BOLD);
        
        move(start_y + 14, x_pos + 1);
        attron(A_DIM);
        printw("%d", i);
        attroff(A_DIM);
    }
    
    move(start_y + 15, 3);
    for (int i = 0; i < state->size * 5 + 2; i++) {
        printw("─");
    }
    
    attron(COLOR_PAIR(6) | A_BOLD);
    move(start_y + 17, 5);
    printw("> Status: %s", state->status);
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    move(start_y + 18, 5);
    printw("Comparações: ");
    attron(COLOR_PAIR(2));
    printw("%d", state->comparisons);
    attroff(COLOR_PAIR(2));
    printw(" | Trocas: ");
    attron(COLOR_PAIR(3));
    printw("%d", state->swaps);
    attroff(COLOR_PAIR(3));

    int sorted_count = 0;
    for (int i = 0; i < state->size; i++) {
        if (state->sorted_positions[i]) sorted_count++;
    }
    printw(" | ");
    attron(COLOR_PAIR(5));
    printw("Ordenados: %d/%d", sorted_count, state->size);
    attroff(COLOR_PAIR(5));
    
    show_speed_controls(state, start_y + 19);
    
    refresh();
}

int wait_with_controls(SortState *state, int start_y) {
    int delay = get_current_delay(state);
    int steps = delay / 10000; 
    
    for (int i = 0; i < steps; i++) {
        usleep(10000);
        
        nodelay(stdscr, TRUE);
        int ch = getch();
        nodelay(stdscr, FALSE);
        
        if (ch != ERR) {
            switch (ch) {
                case 27: 
                    return -1; 
                case ' ': 
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
                case 'l':
                case 'L':
                    state->log_enabled = !state->log_enabled;
                    show_speed_controls(state, start_y + 19);
                    refresh();
                    break;
            }
        }
    }
    return 0;
}

int count_lines(const char* str) {
    int count = 1;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') count++;
    }
    return count;
}

void show_detailed_algorithm_info(const char* name, const char* description, 
                                const char* how_it_works, const char* complexity,
                                const char* advantages, const char* disadvantages,
                                const char* use_cases, const char* variants) {
    clear();
    int y = 2;
    
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(y++, 5, "=== %s - GUIA COMPLETO ===", name);
    attroff(COLOR_PAIR(6) | A_BOLD);
    y++;
    
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "DETALHAMENTO:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    move(y, 5);
    printw("%s", description);
    y += count_lines(description) + 1;
    
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "COMO FUNCIONA:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    move(y, 5);
    printw("%s", how_it_works);
    y += count_lines(how_it_works) + 1;
    
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 5, "COMPLEXIDADE:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    move(y, 5);
    printw("%s", complexity);
    y += count_lines(complexity) + 1;
    
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y++, 5, "VANTAGENS:");
    attroff(COLOR_PAIR(2) | A_BOLD);
    move(y, 5);
    printw("%s", advantages);
    y += count_lines(advantages) + 1;
       
    attron(COLOR_PAIR(2) | A_BOLD);
    mvprintw(y++, 5, "DESVANTAGENS:");
    attroff(COLOR_PAIR(2) | A_BOLD);
    move(y, 5);
    printw("%s", disadvantages);
    y += count_lines(disadvantages) + 1;
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y++, 5, "QUANDO USAR:");
    attroff(COLOR_PAIR(5) | A_BOLD);
    move(y, 5);
    printw("%s", use_cases);
    y += count_lines(use_cases) + 1;
    
    if (strlen(variants) > 0) {
        attron(COLOR_PAIR(7) | A_BOLD);
        mvprintw(y++, 5, "VARIANTES:");
        attroff(COLOR_PAIR(7) | A_BOLD);
        move(y, 5);
        printw("%s", variants);
        y += count_lines(variants) + 1;
    }
    
    attron(COLOR_PAIR(6));
    mvprintw(y + 1, 5, "Pressione ENTER para ver a visualização ou ESC para voltar...");
    attroff(COLOR_PAIR(6));
    
    refresh();
    
    int ch = getch();
    clear();
    return;
}

int bubble_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "BUBBLE SORT",
        "Um dos algoritmos mais simples, que compara elementos adjacentes\n     e os troca se estiverem fora de ordem.",
        "1. Percorre o array comparando pares adjacentes\n     2. Troca elementos se o primeiro for maior que o segundo\n     3. Repete até que nenhuma troca seja necessária\n     4. A cada passada, o maior elemento 'borbulha' para o final",
        "Melhor caso: O(n) - array já ordenado\n     Caso médio: O(n²)\n     Pior caso: O(n²) - array em ordem decrescente\n     Espaço: O(1) - ordenação in-place",
        "Implementação muito simples\n     Estável (mantém ordem relativa de elementos iguais)\n     Detecta quando o array já está ordenado\n     Funciona bem como ferramenta educacional",
        "Extremamente ineficiente para arrays grandes\n     Faz muitas comparações desnecessárias\n     Desempenho sempre quadrático (exceto melhor caso)",
        "Ensino de algoritmos básicos\n     Arrays muito pequenos (< 10 elementos)\n     Situações onde simplicidade é mais importante que eficiência",
        "Cocktail Sort (bidirecional)\n     Odd-Even Sort (para computação paralela)"
    );
    
    strcpy(state->status, "Iniciando Bubble Sort...");
    add_log_entry(state, "=== INÍCIO DO BUBBLE SORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 0; i < state->size - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < state->size - i - 1; j++) {
            reset_highlights(state);
            state->comparing[0] = j;
            state->comparing[1] = j + 1;
            state->comparisons++;
            
            set_status(state, "Passada %d: Comparando posições %d e %d (valores: %d e %d)", 
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
                
                set_status(state, "Trocando %d com %d", state->arr[j + 1], state->arr[j]);
                draw_array(state, start_y);
                if (wait_with_controls(state, start_y) == -1) return -1;
            }
        }
        
        reset_highlights(state);
        state->sorted_until = state->size - i - 1;
        set_status(state, "Fim da passada %d. Maior elemento na posição correta.", i + 1);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (!swapped) {
            set_status(state, "Nenhuma troca realizada. Array já está ordenado!");
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            break;
        }
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Bubble Sort concluído!");
    add_log_entry(state, "=== BUBBLE SORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

int selection_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "SELECTION SORT",
        "Encontra o menor elemento do array e o coloca na primeira posição,\n     depois repete para o restante do array.",
        "1. Encontra o menor elemento do array inteiro\n     2. Troca com o primeiro elemento\n     3. Encontra o menor do restante (excluindo o primeiro)\n     4. Troca com o segundo elemento\n     5. Repete até ordenar todo o array",
        "Melhor caso: O(n²)\n     Caso médio: O(n²)\n     Pior caso: O(n²)\n     Espaço: O(1) - ordenação in-place\n     Sempre faz exatamente n-1 trocas",
        "Minimiza o número de trocas (apenas n-1)\n     Desempenho consistente (sempre O(n²))\n     Simples de implementar\n     Funciona bem quando trocas são custosas",
        "Ineficiente para arrays grandes\n     Não é estável\n     Não se beneficia de dados parcialmente ordenados\n     Sempre faz O(n²) comparações",
        "Arrays pequenos onde trocas são custosas\n     Situações onde consistência de performance é importante\n     Memória limitada (usa apenas O(1) espaço extra)",
        "Heap Sort (versão otimizada usando heap)\n     Bidirectional Selection Sort"
    );
    
    strcpy(state->status, "Iniciando Selection Sort...");
    add_log_entry(state, "=== INÍCIO DO SELECTION SORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 0; i < state->size - 1; i++) {
        int min_idx = i;
        reset_highlights(state);
        state->comparing[0] = i;
        
        set_status(state, "Procurando o menor elemento a partir da posição %d", i);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        for (int j = i + 1; j < state->size; j++) {
            reset_highlights(state);
            state->comparing[0] = min_idx;
            state->comparing[1] = j;
            state->comparisons++;
            
            set_status(state, "Comparando posição %d (valor: %d) com posição %d (valor: %d)", 
                    min_idx, state->arr[min_idx], j, state->arr[j]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
            
            if (state->arr[j] < state->arr[min_idx]) {
                min_idx = j;
                set_status(state, "Novo menor encontrado: %d na posição %d", state->arr[j], j);
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
            
            set_status(state, "Trocando posição %d (valor: %d) com posição %d (valor: %d)", 
                    i, state->arr[min_idx], min_idx, state->arr[i]);
            draw_array(state, start_y);
            if (wait_with_controls(state, start_y) == -1) return -1;
        }
        
        reset_highlights(state);
        state->sorted_until = i + 1;
        set_status(state, "Posição %d ordenada com valor %d", i, state->arr[i]);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Selection Sort concluído!");
    add_log_entry(state, "=== SELECTION SORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

int insertion_sort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "INSERTION SORT",
        "Constrói o array ordenado um elemento por vez, inserindo cada\n     novo elemento na posição correta entre os já ordenados.",
        "1. Considera o primeiro elemento como ordenado\n     2. Pega o próximo elemento (chave)\n     3. Compara com elementos à esquerda\n     4. Move elementos maiores para a direita\n     5. Insere a chave na posição correta\n     6. Repete até o final do array",
        "Melhor caso: O(n) - array já ordenado\n     Caso médio: O(n²)\n     Pior caso: O(n²) - array em ordem decrescente\n     Espaço: O(1) - ordenação in-place",
        "Muito eficiente para arrays pequenos\n     Excelente para dados quase ordenados\n     Estável e adaptativo\n     Ordenação online (pode ordenar conforme recebe dados)\n     Simples de implementar",
        "Ineficiente para arrays grandes\n     O(n²) comparações no pior caso\n     Muitas movimentações de elementos",
        "Arrays pequenos (< 50 elementos)\n     Dados quase ordenados ou parcialmente ordenados\n     Como sub-rotina em algoritmos híbridos (ex: Timsort)\n     Ordenação incremental de dados que chegam aos poucos",
        "Binary Insertion Sort (usa busca binária)\n     Shell Sort (versão com gaps)\n     Library Sort"
    );
    
    strcpy(state->status, "Iniciando Insertion Sort...");
    add_log_entry(state, "=== INÍCIO DO INSERTION SORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int i = 1; i < state->size; i++) {
        int key = state->arr[i];
        int j = i - 1;
        
        reset_highlights(state);
        state->comparing[0] = i;
        set_status(state, "Inserindo elemento %d (posição %d) na posição correta", key, i);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        while (j >= 0 && state->arr[j] > key) {
            reset_highlights(state);
            state->comparing[0] = j;
            state->comparing[1] = j + 1;
            state->comparisons++;
            
            set_status(state, "Comparando %d com %d - Movendo %d para a direita", 
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
        
        set_status(state, "Elemento %d inserido na posição %d", key, j + 1);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
    }
    
    reset_highlights(state);
    state->sorted_until = 0;
    strcpy(state->status, "Insertion Sort concluído!");
    add_log_entry(state, "=== INSERTION SORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

int partition(SortState *state, int low, int high, int start_y) {
    int pivot = state->arr[high];
    int i = low - 1;
    
    reset_highlights(state);
    state->comparing[0] = high; 
    set_status(state, "Pivot escolhido: %d (posição %d)", pivot, high);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    for (int j = low; j < high; j++) {
        reset_highlights(state);
        state->comparing[0] = high;
        state->comparing[1] = j;
        state->comparisons++;
        
        set_status(state, "Comparando %d com pivot %d", state->arr[j], pivot);
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
                
                set_status(state, "Trocando %d (pos %d) com %d (pos %d)", 
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
    
    set_status(state, "Colocando pivot %d na posição correta: %d", pivot, i + 1);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    reset_highlights(state);
    return i + 1;
}

int quicksort_recursive(SortState *state, int low, int high, int start_y) {
    if (low < high) {
        set_status(state, "Particionando subarray de índice %d a %d", low, high);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        int pi = partition(state, low, high, start_y);
        if (pi == -1) return -1; 
        
        set_status(state, "Partição completa. Pivot na posição %d", pi);
        draw_array(state, start_y);
        if (wait_with_controls(state, start_y) == -1) return -1;
        
        if (quicksort_recursive(state, low, pi - 1, start_y) == -1) return -1;
        if (quicksort_recursive(state, pi + 1, high, start_y) == -1) return -1;
    }
    return 0;
}

int quicksort(SortState *state, int start_y) {
    show_detailed_algorithm_info(
        "QUICK SORT",
        "Algoritmo de divisão e conquista que escolhe um pivot e particiona\n     o array: menores à esquerda, maiores à direita.",
        "1. Escolhe um elemento como pivot (geralmente o último)\n     2. Reorganiza o array: menores que pivot à esquerda, maiores à direita\n     3. Coloca o pivot na posição final correta\n     4. Aplica recursivamente nas duas partições\n     5. Combina os resultados (já estão ordenados)",
        "Melhor caso: O(n log n) - pivot sempre divide o array ao meio\n     Caso médio: O(n log n)\n     Pior caso: O(n²) - pivot sempre é o menor ou maior\n     Espaço: O(log n) - devido à recursão",
        "Muito rápido na prática (um dos mais usados)\n     Ordenação in-place\n     Divide e conquista é elegante\n     Boa localidade de cache",
        "Pior caso é O(n²)\n     Não é estável\n     Desempenho depende muito da escolha do pivot\n     Pode causar stack overflow (muita recursão)",
        "Arrays grandes (> 1000 elementos)\n     Quando espaço extra é limitado\n     Dados com distribuição aleatória\n     Implementações de bibliotecas padrão",
        "3-way QuickSort (lida melhor com elementos duplicados)\n     Randomized QuickSort (escolha aleatória do pivot)\n     Dual-Pivot QuickSort (usado no Java)"
    );
    
    strcpy(state->status, "Iniciando QuickSort...");
    add_log_entry(state, "=== INÍCIO DO QUICKSORT ===");
    state->comparisons = 0;
    state->swaps = 0;
    init_sorted_positions(state);
    draw_array(state, start_y);
    if (wait_with_controls(state, start_y) == -1) return -1;
    
    if (quicksort_recursive(state, 0, state->size - 1, start_y) == -1) return -1;
    
    strcpy(state->status, "QuickSort concluído!");
    add_log_entry(state, "=== QUICKSORT CONCLUÍDO ===");
    draw_array(state, start_y);
    return 0;
}

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
    mvprintw(13, 5, " < ou , : Diminuir velocidade");
    mvprintw(14, 5, " > ou . : Aumentar velocidade");
    mvprintw(15, 5, " SPACE  : Pausar/Continuar");
    mvprintw(16, 5, " L      : Habilitar/Desabilitar Log");
    mvprintw(17, 5, " ESC    : Voltar ao menu");
    
    attron(COLOR_PAIR(2));
    mvprintw(19, 5, "Pressione 1-5 para escolher ou ESC para voltar...");
    attroff(COLOR_PAIR(2));
    
    refresh();
    
    int ch = getch();
    if (ch >= '1' && ch <= '5') {
        state->speed = ch - '1';
    }
}

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

void show_algorithms_comparison() {
    clear();
    
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(0, 15, "=== COMPARAÇÃO ENTRE ALGORITMOS DE ORDENAÇÃO ===");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    int y = 2;
    
    attron(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 2, "COMPLEXIDADE DE TEMPO:");
    attroff(COLOR_PAIR(3) | A_BOLD);
    mvprintw(y++, 2, "┌────────────────┬───────────┬───────────┬──────────┐");
    mvprintw(y++, 2, "│ Algoritmo      │ Melhor    │ Médio     │ Pior     │");
    mvprintw(y++, 2, "├────────────────┼───────────┼───────────┼──────────┤");
    mvprintw(y++, 2, "│ Bubble Sort    │ O(n)      │ O(n²)     │ O(n²)    │");
    mvprintw(y++, 2, "│ Selection Sort │ O(n²)     │ O(n²)     │ O(n²)    │");
    mvprintw(y++, 2, "│ Insertion Sort │ O(n)      │ O(n²)     │ O(n²)    │");
    mvprintw(y++, 2, "│ Quick Sort     │ O(n log n)│ O(n log n)│ O(n²)    │");
    mvprintw(y++, 2, "└────────────────┴───────────┴───────────┴──────────┘");
    y++;
    
    attron(COLOR_PAIR(5) | A_BOLD);
    mvprintw(y++, 2, "CARACTERÍSTICAS PRINCIPAIS:");
    attroff(COLOR_PAIR(5) | A_BOLD);
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "• BUBBLE SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "  → Mais fácil de entender  → Detecta array ordenado  → Muito lento");
    y++;
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "• SELECTION SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "  → Poucas trocas (n-1)  → Performance consistente  → Não é estável");
    y++;
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "• INSERTION SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "  → Excelente para dados quase ordenados  → Estável  → Ordenação online");
    y++;
    
    attron(COLOR_PAIR(2));
    mvprintw(y++, 2, "• QUICK SORT:");
    attroff(COLOR_PAIR(2));
    mvprintw(y++, 2, "  → Mais rápido na prática  → In-place  → Pode ser O(n²) no pior caso");
    y++;
    
    attron(COLOR_PAIR(7) | A_BOLD);
    mvprintw(y++, 2, "QUANDO USAR CADA UM:");
    attroff(COLOR_PAIR(7) | A_BOLD);
    mvprintw(y++, 2, "✓ Arrays pequenos (< 10): Insertion Sort");
    mvprintw(y++, 2, "✓ Arrays médios (10-50): Insertion Sort ou Quick Sort");
    mvprintw(y++, 2, "✓ Arrays grandes (> 50): Quick Sort");
    mvprintw(y++, 2, "✓ Dados quase ordenados: Insertion Sort");
    mvprintw(y++, 2, "✓ Minimizar trocas: Selection Sort");
    mvprintw(y++, 2, "✓ Aprendizado: Bubble Sort");
    
    attron(COLOR_PAIR(6));
    mvprintw(y + 2, 2, "Pressione qualquer tecla para voltar...");
    attroff(COLOR_PAIR(6));
    refresh();
    getch();
}

void show_current_array(SortState *state) {
    clear();
    
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(2, 10, "=== ARRAY ATUAL ===");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    mvprintw(4, 5, "Tamanho do array: %d elementos", state->size);
    
    mvprintw(6, 5, "Valores:");
    int line = 7;
    int col = 5;
    
    for (int i = 0; i < state->size; i++) {
        if (col > 70) {
            line++;
            col = 5;
        }
        
        attron(COLOR_PAIR(3));
        mvprintw(line, col, "[%d]", i);
        attroff(COLOR_PAIR(3));
        col += 4;
        
        mvprintw(line, col, "%d", state->arr[i]);
        col += snprintf(NULL, 0, "%d", state->arr[i]) + 2; 
    }
    
    mvprintw(line + 2, 5, "Visualização gráfica:");
    
    int max_val = 0;
    for (int i = 0; i < state->size; i++) {
        if (state->arr[i] > max_val) max_val = state->arr[i];
    }
    
    for (int i = 0; i < state->size; i++) {
        int height = (state->arr[i] * 8) / max_val; 
        if (height == 0) height = 1;
        
        int x_pos = 5 + (i * 4);
        
        attron(COLOR_PAIR(1));
        for (int j = 0; j < height; j++) {
            mvprintw(line + 12 - j, x_pos, "██");
        }
        attroff(COLOR_PAIR(1));
        
        mvprintw(line + 13, x_pos, "%2d", state->arr[i]);
        
        attron(A_DIM);
        mvprintw(line + 14, x_pos, "%2d", i);
        attroff(A_DIM);
    }
    
    mvprintw(line + 16, 5, "Estatísticas:");
    
    int min_val = state->arr[0];
    int max_val_stats = state->arr[0];
    int sum = 0;
    
    for (int i = 0; i < state->size; i++) {
        if (state->arr[i] < min_val) min_val = state->arr[i];
        if (state->arr[i] > max_val_stats) max_val_stats = state->arr[i];
        sum += state->arr[i];
    }
    
    double average = (double)sum / state->size;
    
    mvprintw(line + 17, 5, "Menor valor: %d", min_val);
    mvprintw(line + 18, 5, "Maior valor: %d", max_val_stats);
    mvprintw(line + 19, 5, "Soma total: %d", sum);
    mvprintw(line + 20, 5, "Média: %.2f", average);
    
    int is_sorted = 1;
    for (int i = 0; i < state->size - 1; i++) {
        if (state->arr[i] > state->arr[i + 1]) {
            is_sorted = 0;
            break;
        }
    }
    
    if (is_sorted) {
        attron(COLOR_PAIR(5) | A_BOLD);
        mvprintw(line + 22, 5, "✓ Este array já está ORDENADO!");
        attroff(COLOR_PAIR(5) | A_BOLD);
    } else {
        attron(COLOR_PAIR(2));
        mvprintw(line + 22, 5, "✗ Este array NÃO está ordenado");
        attroff(COLOR_PAIR(2));
    }
    
    attron(COLOR_PAIR(6));
    mvprintw(line + 24, 5, "Pressione qualquer tecla para voltar ao menu...");
    attroff(COLOR_PAIR(6));
    
    refresh();
    getch();
}
int show_menu(SortState *state) {
    clear();
    attron(COLOR_PAIR(6) | A_BOLD);
    mvprintw(2, 10, "=== VISUALIZADOR DE ALGORITMOS DE ORDENAÇÃO ===");
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    mvprintw(4, 5, "Uma ferramenta educacional para aprender algoritmos de ordenação");
    
    mvprintw(6, 5, "Escolha uma opção:");
    mvprintw(8, 5, "1. Bubble Sort");
    mvprintw(9, 5, "2. Selection Sort");
    mvprintw(10, 5, "3. Insertion Sort");
    mvprintw(11, 5, "4. Quick Sort");
    mvprintw(12, 5, "5. Gerar novo array aleatório");
    mvprintw(13, 5, "6. Inserir array manualmente");
    mvprintw(14, 5, "7. Checar array atual");
    mvprintw(15, 5, "8. Configurar velocidade");
    mvprintw(16, 5, "9. Guia de estudo dos algoritmos");
    mvprintw(17, 5, "0. Ver log de execução");
    mvprintw(18, 5, "Pressione ESC para sair");
    
    
    attron(COLOR_PAIR(3));
    mvprintw(20, 5, "Legenda: ");
    
    attron(COLOR_PAIR(2));
    printw("[█] Comparando  ");
    attroff(COLOR_PAIR(2));
    
    attron(COLOR_PAIR(3));
    printw("[█] Trocando  ");
    attroff(COLOR_PAIR(3));
    
    attron(COLOR_PAIR(5));
    printw("[█] Ordenado");
    attroff(COLOR_PAIR(5));
    
    mvprintw(22, 5, "Velocidade atual: %s | Log: %s", 
    speed_names[state->speed], state->log_enabled ? "Habilitado" : "Desabilitado");
    mvprintw(23, 5, "Opção: ");
    refresh();
    

    refresh();
    
    int key = getch();
    
    if (key == 27) { 
        return -1; 
    }
    
    if (key >= '0' && key <= '9') {
        return key - '0';
    }
    
    return -2; 
}

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
    setlocale(LC_ALL, "");
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    
    if (has_colors() == FALSE) {
        endwin();
        printf("Seu terminal não suporta cores\n");
        return 1;
    }
    
    init_colors();
    
    SortState state;
    state.size = 10;
    state.speed = SPEED_NORMAL;
    reset_highlights(&state);
    init_sorted_positions(&state);
    init_log(&state);
    
    srand(time(NULL));
    for (int i = 0; i < state.size; i++) {
        state.arr[i] = rand() % 100 + 1;
    }
    
    int choice;
    int original_arr[MAX_SIZE];
    
    while (1) {
        for (int i = 0; i < state.size; i++) {
            original_arr[i] = state.arr[i];
        }
        
        choice = show_menu(&state);
        
        if (choice == -1) { 
            break;
        }
        
        if (choice == 0) {
            show_log(&state);
            continue;
        }
        
        if (choice == 7) {
            show_current_array(&state);
            continue;
        }
        
        if (choice == 8) {
            speed_menu(&state);
            continue;
        }
        
        if (choice == 9) {
            int study_choice = algorithm_study_menu();
            if (study_choice == 5) {
                show_algorithms_comparison();
            } else if (study_choice >= 1 && study_choice <= 4) {
                clear();
                int result = 0;
                
                switch (study_choice) {
                    case 1: result = bubble_sort(&state, 3); break;
                    case 2: result = selection_sort(&state, 3); break;
                    case 3: result = insertion_sort(&state, 3); break;
                    case 4: result = quicksort(&state, 3); break;
                }
                
                for (int i = 0; i < state.size; i++) {
                    state.arr[i] = original_arr[i];
                }
                init_sorted_positions(&state);
                reset_highlights(&state);
                
                if (result != -1) {
                    mvprintw(20, 5, "Pressione 'r' para repetir ou qualquer tecla para voltar ao menu");
                    refresh();
                    
                    int key = getch();
                    if (key == 'r' || key == 'R') {
                        clear();
                        
                        switch (study_choice) {
                            case 1: bubble_sort(&state, 3); break;
                            case 2: selection_sort(&state, 3); break;
                            case 3: insertion_sort(&state, 3); break;
                            case 4: quicksort(&state, 3); break;
                        }
                        
                        for (int i = 0; i < state.size; i++) {
                            state.arr[i] = original_arr[i];
                        }
                        init_sorted_positions(&state);
                        reset_highlights(&state);
                        
                        mvprintw(20, 5, "Pressione qualquer tecla para voltar ao menu...");
                        refresh();
                        getch();
                    }
                }
            }
            continue;
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
        
        for (int i = 0; i < state.size; i++) {
            state.arr[i] = original_arr[i];
        }
        init_sorted_positions(&state);
        reset_highlights(&state);
        
        if (result != -1) {
            mvprintw(20, 5, "Pressione 'r' para repetir com o mesmo array ou qualquer tecla para voltar ao menu");
            refresh();
            
            int key = getch();
            if (key == 'r' || key == 'R') {
                clear();
                
                switch (choice) {
                    case 1: bubble_sort(&state, 3); break;
                    case 2: selection_sort(&state, 3); break;
                    case 3: insertion_sort(&state, 3); break;
                    case 4: quicksort(&state, 3); break;
                }
                
                for (int i = 0; i < state.size; i++) {
                    state.arr[i] = original_arr[i];
                }
                init_sorted_positions(&state);
                reset_highlights(&state);
                
                mvprintw(20, 5, "Pressione qualquer tecla para voltar ao menu...");
                refresh();
                getch();
            }
        }
    }
    
    endwin();
    return 0;
}