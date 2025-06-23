#ifndef UTILS_H
#define UTILS_H

#include "sort_visualizer.h"

void reset_highlights(SortState *state);
void init_sorted_positions(SortState *state);
void update_sorted_positions(SortState *state);

void init_log(SortState *state);
void add_log_entry(SortState *state, const char* message);
void set_status(SortState *state, const char* format, ...);
void show_log(SortState *state);

int get_current_delay(SortState *state);
int count_lines(const char* str);

#endif 