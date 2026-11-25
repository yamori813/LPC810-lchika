#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _etext;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern int main(void);

void Reset_Handler(void);
void Default_Handler(void) { while (1); }

// Cortex-M0+ 最小ベクタテーブル (LPC81x用)
__attribute__((section(".isr_vector")))
const uint32_t vector_table[] = {
    (uint32_t)&_estack,         // 0: 初期スタックポインタ (0x10000400)
    (uint32_t)Reset_Handler,    // 1: Reset
    (uint32_t)Default_Handler,  // 2: NMI
    (uint32_t)Default_Handler,  // 3: HardFault
    0, 0, 0,                    // 4-6: Reserved
    0,                          // 7: NXP チェックサム領域 (ISP書き込み時に自動計算)
    0, 0, 0,                    // 8-10: Reserved
    (uint32_t)Default_Handler,  // 11: SVCall
    0, 0,                       // 12-13: Reserved
    (uint32_t)Default_Handler,  // 14: PendSV
    (uint32_t)Default_Handler   // 15: SysTick
};

void Reset_Handler(void) {
    // .data セクションを Flash から RAM へコピー
    uint32_t *src = &_etext;
    uint32_t *dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    // .bss セクションをゼロクリア
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }

    main();
    while (1);
}

