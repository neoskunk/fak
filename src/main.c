#include "ch55x.h"
#include "keyboard.h"
#include "time.h"
#include "neo.h"

const int DEBOUNCE_MILLIS = 100;
typedef enum
{
    false,
    true
} bool;

volatile bool _led_on = false;
volatile bool _suspended = false;
volatile uint16_t _last_state_change = 0;

static void LED_on()
{
    int i;

    // red
    int r1[2] = {1, 2};
    for (i = 0; i < 2; i++)
    {
        NEO_writeColor(r1[i], 255, 0, 0);
    }

    // orange
    NEO_writeColor(0, 255, 30, 0);

    // yellow
    int r2[3] = {3, 4, 5};
    for (i = 0; i < 3; i++)
    {
        NEO_writeColor(r2[i], 255, 255, 0);
    }

    // green
    int r3[3] = {6, 7, 8};
    for (i = 0; i < 3; i++)
    {
        NEO_writeColor(r3[i], 0, 255, 0);
    }

    // cyan
    NEO_writeColor(9, 0, 255, 255);

    // blue
    int r4[3] = {10, 11, 12};
    for (i = 0; i < 3; i++)
    {
        NEO_writeColor(r4[i], 0, 0, 255);
    }

    // purple
    int r5[4] = {13, 14, 15, 16};
    for (i = 0; i < 4; i++)
    {
        NEO_writeColor(r5[i], 255, 0, 255);
    }

    NEO_update();
    _led_on = true;
}

static void LED_off()
{
    NEO_clearAll();
    NEO_update();
    _led_on = false;
}

static void LED_control()
{
    if (_suspended && _led_on && get_timer() - _last_state_change > DEBOUNCE_MILLIS)
    {
        LED_off();
    }
    else if (!_suspended && !_led_on && get_timer() - _last_state_change > DEBOUNCE_MILLIS)
    {
        LED_on();
    }
}

static void change_state()
{
    if (UIF_SUSPEND)
    {
        if (USB_MIS_ST & bUMS_SUSPEND)
        {
            _suspended = true;
        }
        else
        {
            _suspended = false;
        }

        _last_state_change = get_timer();
    }
}

#ifdef SPLIT_SIDE_CENTRAL
#include "usb.h"

void USB_interrupt();
void USB_ISR() __interrupt(INT_NO_USB)
{
    change_state();
    USB_interrupt();
}
#endif

void TMR0_interrupt();
void TMR0_ISR() __interrupt(INT_NO_TMR0)
{
    TMR0_interrupt();
    LED_control();
}

#if defined(SPLIT_ENABLE) && !defined(SPLIT_SOFT_SERIAL_PIN)
#ifdef SPLIT_SIDE_PERIPHERAL
void UART0_interrupt();
void UART0_ISR() __interrupt(INT_NO_UART0)
{
    UART0_interrupt();
}
#endif

static void UART0_init()
{
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

static void main()
{
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

    while (1)
    {
        keyboard_scan();
    }
}
