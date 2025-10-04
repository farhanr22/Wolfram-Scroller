#include "utils.h"

#include <stdlib.h>
#include <string.h>

// Fills up the array with random 0s and 1s
void randomize_array(int* array, int size) {
    for (int i = 0; i < size; i++) {
        array[i] = rand() % 2;
    }
}

// Clears the array and puts a single 1 right in the middle
void init_middle(int* arr, int size) {
    memset(arr, 0, size * sizeof(int));
    arr[size / 2] = 1;
}

// Decodes the rule to find the state of the next cell.
// The 3-cell neighborhood (left, center, right) forms a 3-bit number (from 0 to 7),
// and we just grab the corresponding bit from the rule byte.
int next_bit(int left, int center, int right, int rule) {
    // Example: if the neighbors are 1, 1, 0, that's binary 110, which is 6.
    // This will shift the 'rule' byte 6 bits to the right and grab the result.
    return (rule >> ((left * 4) + (center * 2) + right)) & 1;
}

// Calculates the entire next generation for our 1D buffer
void next_gen(int* data, int width, int rule) {
    // we need a temporary copy of the current generation to read from,
    // otherwise we'd be calculating based on already-updated cells.
    int tmp[width];
    memcpy(tmp, data, width * sizeof(int));

    for (int i = 0; i < width; i++) {
        // The ternary operators here handle the "wrap-around" logic for the edges.
        // If we're at the far left (i=0), the 'left' neighbor is the far right element.
        int left = tmp[(i == 0) ? width - 1 : i - 1];
        int middle = tmp[i];
        int right = tmp[(i == width - 1) ? 0 : i + 1];

        data[i] = next_bit(left, middle, right, rule);
    }
}

// Allocates memory for a 2D array (an array of pointers to arrays)
int** new_grid(int w) {
    int** tmp = malloc(sizeof(int*) * w);
    for (int i = 0; i < w; i++) {
        // calloc is nice here because it initializes the memory to zero 
        tmp[i] = calloc(w, sizeof(int));
    }
    return tmp;
}

// Frees the memory for our 2D grid.
// Has to free each row first, then the array that held the pointers to the rows.
void free_grid(int** grid, int w) {
    for (int i = 0; i < w; i++) {
        free(grid[i]);
    }
    free(grid);
}