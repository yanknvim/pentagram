#include <stdint.h>
#define LED_ADDR 0xFE000000

int main(void) {

    while (true) {
        *(volatile uint8_t *)(LED_ADDR) = 1;
        *(volatile uint8_t *)(LED_ADDR) = 0;
    }
}
