#ifndef SORT_VISUALIZER_H
#define SORT_VISUALIZER_H

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

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
#define USE_SIMPLE_CHARS 0
#define MAX_LOG_ENTRIES 1000
#define MAX_LOG_LENGTH 200

typedef enum {
    SPEED_VERY_SLOW = 0,
    SPEED_SLOW = 1,
    SPEED_NORMAL = 2,
    SPEED_FAST = 3,
    SPEED_VERY_FAST = 4
} SpeedLevel;

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

extern int speed_delays[];
extern char* speed_names[];

#endif 