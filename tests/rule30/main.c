#include <stdint.h>

void printchar(char c) {
    volatile char *uart = (char*)(0xFF000000);
    volatile uint8_t *is_busy = (uint8_t*)(0xFF000001);

    *uart = c;

    while (*is_busy == 0) {}
    while (*is_busy == 1) {}
}

void draw(int* s, int length) {
    for (int i = 0; i < length; i++) {
        if (s[i] == 1) {
            printchar('#');
        } else {
            printchar('.');
        }
    }
    printchar('\n');
}

int main(void) {
    int width = 60;
    int gen = 100;

    int state[width];
    int next[width];

    for (int i = 0; i < width; i++) {
        state[i] = 0;
    }

    state[width / 2] = 1;

    for (int i = 0; i < gen; i++) {
        draw(state, width);
        for (int j = 0; j < width; j++) {
            int left = j == 0 ? 0 : state[j - 1];
            int right = j == width ? width : state[j + 1];
            next[j] = left ^ (state[j] | right);
        }

        for (int j = 0; j < width; j++) {
            state[j] = next[j];
        }
    }
}
