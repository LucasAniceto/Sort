#include "utils.h"

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

int count_lines(const char* str) {
    int count = 1;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '\n') count++;
    }
    return count;
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
                case 27:
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