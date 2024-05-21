#include "ch55x.h"
#include "keyboard.h"
#include "time.h"
#include "neo.h"

static void LED_on() {
    int i;
    int c1[3] = {0, 9, 13};
    for (i = 0; i < 3; i++) {
        NEO_writeColor(c1[i], 255, 0, 255);
    }

    int c2[5] = {1, 3, 6, 10, 14};
    for (i = 0; i < 5; i++) {
        NEO_writeColor(c2[i], 0, 0, 255);
    }

    int c3[4] = {4, 7, 11, 15};
    for (i = 0; i < 4; i++) {
        NEO_writeColor(c3[i], 0, 255, 0);
    }

    int c4[5] = {2, 5, 8, 12, 16};
    for (i = 0; i < 5; i++) {
        NEO_writeColor(c4[i], 255, 0, 0);
    }

    NEO_update();
}

static void LED_off() {
    NEO_clearAll();
    NEO_update();
}

static void LED_control() {
    if (UIF_SUSPEND) {
        if (!(USB_MIS_ST & bUMS_SUSPEND)) {
            LED_on();
        } else {
            LED_off();
        }
    }
}

#ifdef SPLIT_SIDE_CENTRAL
#include "usb.h"

void USB_interrupt();
void USB_ISR() __interrupt(INT_NO_USB) {
    LED_control();
    USB_interrupt();
}
#endif

void TMR0_interrupt();
void TMR0_ISR() __interrupt(INT_NO_TMR0) {
    TMR0_interrupt();
}

#if defined(SPLIT_ENABLE) && !defined(SPLIT_SOFT_SERIAL_PIN)
#ifdef SPLIT_SIDE_PERIPHERAL
void UART0_interrupt();
void UART0_ISR() __interrupt(INT_NO_UART0) {
    UART0_interrupt();
}
#endif

static void UART0_init() {
    // UART0 @ Timer1, 750k bps
    SM0 = 0;
    SM1 = 1;
    PCON |= SMOD;

    TMOD = TMOD & ~bT1_GATE & ~bT1_CT & ~MASK_T1_MOD | bT1_M1;
    T2MOD |= bTMR_CLK | bT1_CLK;
#if CH55X == 2
    TH1 = 254;
#elif CH55X == 9
    TH1 = 255;
#endif
    TR1 = 1;
    TI = 1;
}
#endif

static void main() {
    NEO_init();
    CLK_init();
#if defined(SPLIT_ENABLE) && !defined(SPLIT_SOFT_SERIAL_PIN)
    UART0_init();
#endif
#ifdef SPLIT_SIDE_CENTRAL
    TMR0_init();
    USB_init();
#endif
    keyboard_init();

#ifdef UART0_ALT
    PIN_FUNC |= bUART0_PIN_X;
#endif
#ifdef UART1_ALT
    PIN_FUNC |= bUART1_PIN_X;
#endif

#if CH55X == 9
    // This is to get same behavior as CH552 to drive pins high immediately
    P0_DIR = 0xFF;
    P1_DIR = 0xFF;
    P2_DIR = 0xFF;
    P3_DIR = 0xFF;
#endif

    EA = 1;

    while (1) {
        keyboard_scan();
    }
}
