#ifndef UTILS_H
#define UTILS_H

#include <stdio.h> // Included for the debug macro's printf

// A simple macro for debug prints.
// To use, just uncomment the #define line below and recompile.
// #define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) printf("DEBUG: " fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...) \
    do {                      \
    } while (0)
#endif

// === Cellular Automata Logic ===

// Fills an array with random 0s and 1s.
void randomize_array(int* array, int size);

// Zeros out an array and puts a single 1 in the middle.
void init_middle(int* arr, int size);

// Calculates the next state of a single cell based on a rule and its neighbors.
int next_bit(int left, int center, int right, int rule);

// Computes the entire next generation for a 1D array of cells.
void next_gen(int* data, int width, int rule);

// === Grid Memory Management ===

// Allocates a new 2D grid of a given width (and height).
int** new_grid(int w);

// Frees all memory used by a 2D grid.
void free_grid(int** grid, int w);

#endif // UTILS_H