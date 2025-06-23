#include "sort_visualizer.h"
#include "ui.h"
#include "algorithms.h"
#include "utils.h"

int speed_delays[] = {800000, 500000, 300000, 150000, 50000}; 
char* speed_names[] = {"Muito Lento", "Lento", "Normal", "Rápido", "Muito Rápido"};

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