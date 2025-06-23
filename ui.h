#ifndef UI_H
#define UI_H

#include "sort_visualizer.h"

void init_colors(void);
void show_speed_controls(SortState *state, int y);

void draw_array(SortState *state, int start_y);
void show_current_array(SortState *state);

int show_menu(SortState *state);
void speed_menu(SortState *state);
int algorithm_study_menu(void);

void generate_random_array(SortState *state);
void input_manual_array(SortState *state);

int wait_with_controls(SortState *state, int start_y);

void show_detailed_algorithm_info(const char* name, const char* description, 
                                const char* how_it_works, const char* complexity,
                                const char* advantages, const char* disadvantages,
                                const char* use_cases, const char* variants);
void show_algorithms_comparison(void);

#endif