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
void MRT_IRQHandler(void);

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
    (uint32_t)Default_Handler,   // 15: SysTick

	// Chip Level - LPC8xx
    (uint32_t)Default_Handler,                         // SPI0 controller
    (uint32_t)Default_Handler,                         // SPI1 controller
    0,                                       // Reserved
    (uint32_t)Default_Handler,                        // UART0
    (uint32_t)Default_Handler,                        // UART1
    (uint32_t)Default_Handler,                        // UART2
    0,                                       // Reserved
    0,                                       // Reserved
    (uint32_t)Default_Handler,                          // I2C controller
    (uint32_t)Default_Handler,                          // Smart Counter Timer
    (uint32_t)MRT_IRQHandler,                          // Multi-Rate Timer
    (uint32_t)Default_Handler,                          // Comparator
    (uint32_t)Default_Handler,                          // PIO1 (0:11)
    (uint32_t)Default_Handler,                          // Brown Out Detect
    0,                                       // Reserved
    (uint32_t)Default_Handler,                          // Wakeup timer
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    0,                                       // Reserved
    (uint32_t)Default_Handler,                      // PIO INT0
    (uint32_t)Default_Handler,                      // PIO INT1
    (uint32_t)Default_Handler,                      // PIO INT2
    (uint32_t)Default_Handler,                      // PIO INT3
    (uint32_t)Default_Handler,                      // PIO INT4
    (uint32_t)Default_Handler,                      // PIO INT5
    (uint32_t)Default_Handler,                      // PIO INT6
    (uint32_t)Default_Handler,                      // PIO INT7
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

