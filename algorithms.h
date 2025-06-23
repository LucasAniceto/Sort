#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "sort_visualizer.h"


int bubble_sort(SortState *state, int start_y);
int selection_sort(SortState *state, int start_y);
int insertion_sort(SortState *state, int start_y);
int quicksort(SortState *state, int start_y);


int partition(SortState *state, int low, int high, int start_y);
int quicksort_recursive(SortState *state, int low, int high, int start_y);

#endif 