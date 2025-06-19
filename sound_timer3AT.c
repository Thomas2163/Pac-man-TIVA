/********************************************************************************************
 * Descripcion: El programa genera una melodía utilizando el TIMER3A con playback basado en interrupciones
 *              y evita el uso de TIMER0, TIMER1 y TIMER2 (reservados para VGA).
 * Conexion:   PB4 -> BUZZER (T3CCP0)
 ********************************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "hw_memmap.h"

#include "hw_types.h"
#include "sysctl.h"
#include "gpio.h"
#include "pin_map.h"
#include "timer.h"
#include "interrupt.h"

// Definición manual en caso de no disponer de pin_map.h:
#ifndef GPIO_PB4_T3CCP0
#define GPIO_PB4_T3CCP0 0x00070000  // PCTL valor AF7 en PB4 (T3CCP0)
#endif

#define SW1             (1U<<4)
#define Reloj_Sistema   80000000UL // para 80 MHz, o 16000000UL para 16 MHz

// Notas
// Notas
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
#define REST      0
// (el resto de tu código sigue igual)

// Melodías de Pac-Man
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


// Variables globales para ISR
volatile int *g_melody;
volatile int *g_durations;
volatile int  g_melodyLength;
volatile int  g_noteIndex;

void config_switches(void) {
    // PF4 como entrada con pull-up
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
}

void config_timer3_pwm(uint32_t freq) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB));

    GPIOPinConfigure(GPIO_PB4_T3CCP0);
    GPIOPinTypeTimer(GPIO_PORTB_BASE, GPIO_PIN_4);

    TimerDisable(TIMER3_BASE, TIMER_A);

    // Calcular periodo para la frecuencia deseada
    uint32_t period = Reloj_Sistema / freq;
    uint8_t prescale = 0;
    uint32_t load = period;

    // Ajusta automáticamente prescaler y load
    while (load > 65535 && prescale < 255) {
        prescale++;
        load = period / (prescale + 1);
    }

    if (load > 65535) {
        TimerDisable(TIMER3_BASE, TIMER_A); // Nota muy grave, ignora
        return;
    }

    TimerPrescaleSet(TIMER3_BASE, TIMER_A, prescale);
    TimerLoadSet(TIMER3_BASE, TIMER_A, load - 1);

    // Duty cycle 50%
    uint32_t match = load / 2;
    TimerPrescaleMatchSet(TIMER3_BASE, TIMER_A, prescale);
    TimerMatchSet(TIMER3_BASE, TIMER_A, match - 1);

    TimerEnable(TIMER3_BASE, TIMER_A);
}

void config_timer3b_interrupt(uint32_t ms) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER3));

    TimerDisable(TIMER3_BASE, TIMER_B);

    uint32_t ticks = (Reloj_Sistema / 1000) * ms;
    uint8_t prescale = 0;
    uint32_t load = ticks;

    // Ajusta automáticamente prescaler y load
    while (load > 65535 && prescale < 255) {
        prescale++;
        load = ticks / (prescale + 1);
    }

    if (load > 65535) {
        TimerDisable(TIMER3_BASE, TIMER_B); // Tiempo muy largo, ignora
        return;
    }

    TimerPrescaleSet(TIMER3_BASE, TIMER_B, prescale);
    TimerLoadSet(TIMER3_BASE, TIMER_B, load - 1);

    TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    IntPrioritySet(INT_TIMER3B, 0);
    IntEnable(INT_TIMER3B);

    TimerEnable(TIMER3_BASE, TIMER_B);
}

void startMelody_IT(int *melody, int *durations, int length) {
    g_melody = melody;
    g_durations = durations;
    g_melodyLength = length;
    g_noteIndex = 0;
    
    // Primera nota
    if(melody[0] > 0) config_timer3_pwm(melody[0]);
    // Programar interrupción tras duración de nota
    int base = abs(durations[0]);
    int ms = 1000 / base;
    if(durations[0] < 0) ms = ms * 3 / 2;
    config_timer3b_interrupt(ms);
}

// ISR para avanzar notas
void Timer3B_Handler(void) {
    TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    // Detener nota actual
    TimerDisable(TIMER3_BASE, TIMER_A);
    // Siguiente nota
    g_noteIndex++;
    if(g_noteIndex < g_melodyLength) {
        int freq = g_melody[g_noteIndex];
        int dur = abs(g_durations[g_noteIndex]);
        int ms = 1000 / dur;
        if(g_durations[g_noteIndex] < 0) ms = ms * 3 / 2;
        // Configurar siguiente nota
        if(freq > 0) config_timer3_pwm(freq);
        // Programar próxima interrupción
        TimerLoadSet(TIMER3_BASE, TIMER_B, (Reloj_Sistema/1000)*ms - 1);
    }
    // Si se acaba, no volver a habilitar B
}

int main(void) {
    // Configurar reloj a 16 MHz
   SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    
    // Configurar switch en PF4
    config_switches();
    
    // Registrar ISR de Timer3B y habilitar interrupciones
    IntRegister(INT_TIMER3B, Timer3B_Handler);
    IntEnable(INT_TIMER3B);
    IntMasterEnable();
    
    // Iniciar melodía de inicio usando interrupciones
    startMelody_IT(melody_inicio, durations_inicio, sizeof(melody_inicio)/sizeof(int));
    
    // Bucle principal vacío: el playback ocurre en ISR
    while(1) {
        // Se puede agregar lógica adicional aquí
    }
}

