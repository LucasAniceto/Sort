#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "ui.h"
#include "utils.h"

#include <unistd.h>

void init_colors() {
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);
    init_pair(4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(5, COLOR_CYAN, COLOR_BLACK);
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(7, COLOR_BLUE, COLOR_BLACK);
}

void show_speed_controls(SortState *state, int y) {
    attron(COLOR_PAIR(7));
    mvprintw(y, 5, "Velocidade: [<] %s [>] | ESC: Menu | SPACE: Pausar | L: %s Log", 
             speed_names[state->speed], state->log_enabled ? "Desabilitar" : "Habilitar");
    attroff(COLOR_PAIR(7));
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
    
    getch();
    clear();
    return;
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
    
    int key = getch();
    
    if (key == 27) {
        return -1; 
    }
    
    if (key >= '0' && key <= '9') {
        return key - '0';
    }
    
    return -2;
}

void speed_menu(SortState *state) {
    clear();
    attron(COLOR_PAIR(6));
    mvprintw(2, 10, "=== CONFIGURAÇÃO DE VELOCIDADE ===");
    attroff(COLOR_PAIR(6));
    
    mvprintw(4, 5, "Escolha a velocidade de visualização:");
    
    for (int i = 0; i <= (int)SPEED_VERY_FAST; i++) {
        if (i == (int)state->speed) {
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
        state->speed = (SpeedLevel)(ch - '1');
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