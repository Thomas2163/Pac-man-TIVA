/********************************************************************************************
 * Descripcion: El programa genera una melodía utilizando el TIMER3A con playback basado en 
 *              interrupciones. Ajusta la duración de cada nota con el mismo factor 1.3×  
 * Conexion:   PB2 -> BUZZER (T3CCP0)
 ********************************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "hw_memmap.h"
#include "sysctl.h"
#include "gpio.h"
#include "pin_map.h"
#include "timer.h"
#include "interrupt.h"

#ifndef GPIO_PB2_T3CCP0
#define GPIO_PB2_T3CCP0  (0x7 << (2 * 4))  // AF7 en PB2 = T3CCP0
#endif

#define Reloj_Sistema   80000000UL // 80 MHz

// … tu lista completa de notas …
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0

// Melodía de inicio
// 1. Melodía de INICIO
int melody_inicio[] = {
  NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_B5, NOTE_FS5, NOTE_DS5,
  NOTE_C5, NOTE_C6, NOTE_G6, NOTE_E6, NOTE_C6, NOTE_G6, NOTE_E6,
  NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5, NOTE_B5, NOTE_FS5, NOTE_DS5,
  NOTE_DS5, NOTE_E5, NOTE_F5, NOTE_F5, NOTE_FS5, NOTE_G5,
  NOTE_G5, NOTE_GS5, NOTE_A5, NOTE_B5
};
int durations_inicio[] = {
  8, 8, 8, 8, 8, -16, 8,
  8, 8, 8, 8, 16, -16, 8,
  8, 8, 8, 8, 8, -16, 8,
  16, 16, 16, 16, 16, 16,
  16, 16, 8, 8
};

// 2. Sonido de MUERTE (Versión 2, más caótica y fiel)
int melody_muerte[] = {
  NOTE_F6, NOTE_E6, NOTE_DS6, NOTE_D6, NOTE_C6, NOTE_A5, 
  NOTE_G5, NOTE_E5, NOTE_C5, NOTE_G4, NOTE_C4
};
int durations_muerte[] = {
  32, 32, 32, 32, 32, 32,
  24, 24, 24, 24, 16 
};

// 3. Sonido de WAKA-WAKA (Versión 2, más corta y percusiva)
int melody_wakawaka[] = {
  NOTE_D4, NOTE_B3 
};
int durations_wakawaka[] = {
  12, 12
};

// 4. Sonido al COMER UN FANTASMA
int melody_comerFantasma[] = {
  NOTE_C5, NOTE_E5, NOTE_G5, NOTE_C6, REST,
  NOTE_E6, NOTE_G6, NOTE_C7
};
int durations_comerFantasma[] = {
  16, 16, 16, 16, 16,
  16, 16, 16
};

// 5. Sonido de SIRENA DE FANTASMAS
int melody_fantasma[] = {
  NOTE_AS4, NOTE_A4, NOTE_AS4, NOTE_A4, NOTE_AS4, NOTE_A4
};
int durations_fantasma[] = {
  16, 16, 16, 16, 16, 16
};

volatile int *g_melody;
volatile int *g_durations;
volatile int  g_length;
volatile int  g_index;

// Configura TIMER3A en PWM 50% en PB2
void config_timer3a_pwm(uint32_t freq) {
    uint32_t period = Reloj_Sistema / freq;
    uint32_t prescale = period >> 16;
    uint32_t load     = period & 0xFFFF;
    if(load == 0) { prescale--; load = 0xFFFF; }

    TimerDisable(TIMER3_BASE, TIMER_A);
    TimerPrescaleSet(TIMER3_BASE, TIMER_A, prescale);
    TimerLoadSet     (TIMER3_BASE, TIMER_A, load - 1);
    TimerMatchSet    (TIMER3_BASE, TIMER_A, (load/2) - 1);
    TimerEnable      (TIMER3_BASE, TIMER_A);
}

// Configura TIMER3B split para timeout en 'ticks'
void config_timer3b_ticks(uint32_t ticks) {
    uint32_t prescale = ticks >> 16;
    uint32_t load     = ticks & 0xFFFF;
    if(load == 0) { prescale--; load = 0xFFFF; }

    TimerDisable(TIMER3_BASE, TIMER_B);
    TimerPrescaleSet(TIMER3_BASE, TIMER_B, prescale);
    TimerLoadSet     (TIMER3_BASE, TIMER_B, load - 1);
    TimerIntClear    (TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    TimerIntEnable   (TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    IntEnable        (INT_TIMER3B);
    TimerEnable      (TIMER3_BASE, TIMER_B);
}

// Arranca la melodía usando interrupciones
void startMelody_IT(int *melody, int *durations, int length) {
    g_melody    = melody;
    g_durations = durations;
    g_length    = length;
    g_index     = 0;

    // calcula ms base y aplica punteado + factor 1.3
    int base = abs(durations[0]);
    int ms   = 1000 / base;
    if(durations[0] < 0) ms = ms * 3 / 2;
    ms = (ms * 13) / 10;  // factor 1.3×

    if(melody[0] > 0) {
        config_timer3a_pwm(melody[0]);
    }
    uint32_t ticks = (Reloj_Sistema / 1000) * ms;
    config_timer3b_ticks(ticks);
}

// ISR: avanza a la siguiente nota
void Timer3B_Handler(void) {
    TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    TimerDisable(TIMER3_BASE, TIMER_A);

    g_index++;
    if(g_index < g_length) {
        int freq = g_melody[g_index];
        int base = abs(g_durations[g_index]);
        int ms   = 1000 / base;
        if(g_durations[g_index] < 0) 
            ms = ms * 3 / 2;
        ms = (ms * 13) / 10;  // aplica factor 1.3×

        if(freq > 0) {
            config_timer3a_pwm(freq);
        }
        uint32_t ticks = (Reloj_Sistema / 1000) * ms;
        config_timer3b_ticks(ticks);
    }
}

int main(void) {
    // reloj 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 |
                   SYSCTL_USE_PLL    |
                   SYSCTL_XTAL_16MHZ |
                   SYSCTL_OSC_MAIN);

    // habilita TIMER3 y configura split A=PWM, B=periódico
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));
    TimerConfigure(TIMER3_BASE,
                   TIMER_CFG_SPLIT_PAIR |
                   TIMER_CFG_A_PWM      |
                   TIMER_CFG_B_PERIODIC);

    // configura PB2 para T3CCP0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));
    GPIOPinConfigure(GPIO_PB2_T3CCP0);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_2);

    // registra ISR y arranca
    IntRegister(INT_TIMER3B, Timer3B_Handler);
    IntMasterEnable();
    startMelody_IT(melody_inicio, durations_inicio,
                   sizeof(melody_inicio)/sizeof(int));

    while(1) {}
    return 0;
}


