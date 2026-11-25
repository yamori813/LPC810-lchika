#include <stdint.h>

// レジスタアドレス定義
#define SYSAHBCLKCTRL  (*(volatile uint32_t *)0x40048080)
#define GPIO_DIR0      (*(volatile uint32_t *)0xA0002000)
#define GPIO_NOT0      (*(volatile uint32_t *)0xA0002300)

// 簡易ウェイト関数
void delay(volatile uint32_t count) {
    while (count--) {
        __asm__("nop");
    }
}

int main(void) {
    // GPIOモジュールへクロック供給 (Bit 6 = GPIO)
    SYSAHBCLKCTRL |= (1 << 6);

    // PIO0_1 (5番ピン) を出力に設定
    GPIO_DIR0 |= (1 << 1);

    while (1) {
        // PIO0_1 の出力を反転
        GPIO_NOT0 = (1 << 1);
        delay(500000);
    }
    return 0;
}

