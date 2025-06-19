/********************************************************************************************
 * Descripcion: El programa genera una melodía utilizando el TIMER3A con playback basado en interrupciones
 *              y evita el uso de TIMER0, TIMER1 y TIMER2 (reservados para VGA).
 * Conexion:   PB4 -> BUZZER (T3CCP0)
 ********************************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "sysctl.h"
#include "timer.h"
#include "interrupt.h"
#include "gpio.h"

#define SW1             (1U<<4)
#define Reloj_Sistema   16000000UL // 16 MHz

// Notas
#define REST            0
#define NOTE_B4         494
#define NOTE_C5         523
#define NOTE_CS5        554
#define NOTE_D5         587
#define NOTE_DS5        622
#define NOTE_E5         659
#define NOTE_F5         698
#define NOTE_FS5        740
#define NOTE_G5         784
#define NOTE_GS5        831
#define NOTE_A5         880
#define NOTE_AS5        932
#define NOTE_B5         988
#define NOTE_C6         1047
#define NOTE_CS6        1109
#define NOTE_D6         1175
#define NOTE_DS6        1245
#define NOTE_E6         1319
#define NOTE_F6         1397
#define NOTE_FS6        1480
#define NOTE_G6         1568
#define NOTE_GS6        1661
#define NOTE_A6         1760
#define NOTE_AS6        1865
#define NOTE_B6         1976
#define NOTE_C7         2093
#define NOTE_DS7        2489

// Melodías de Pac-Man
int melody_inicio[]      = { NOTE_B4, NOTE_B5, NOTE_FS5, NOTE_DS5, /* ...resto...*/ };
int durations_inicio[]   = { 8, 8, 8, -16, /* ...resto...*/ };
int melody_muerte[]      = { NOTE_F6, NOTE_E6, NOTE_DS6, /* ...*/ };
int durations_muerte[]   = { 32, 32, 32, /* ...*/ };
int melody_wakawaka[]    = { NOTE_D4, NOTE_B3 };
int durations_wakawaka[] = { 12, 12 };
int melody_comerFantasma[]    = { NOTE_C5, NOTE_E5, NOTE_G5, /*...*/ };
int durations_comerFantasma[] = { 16, 16, 16, /*...*/ };
int melody_fantasma[]         = { NOTE_AS4, NOTE_A4, /*...*/ };
int durations_fantasma[]      = { 16, 16, /*...*/ };

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
    // Habilita señal de reloj para TIMER3
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    while(!(SYSCTL_PRTIMER_R & SYSCTL_PRTIMER_R3));
    
    // Deshabilita Timer3A (modo conteo descendente, inactivo)
    TIMER3_CTL_R &= ~0x141;
    // Configura como 16 bits
    TIMER3_CFG_R = 0x4;
    // Modo PWM (TnAMS=1, TnCMR=0, TnMR=2)
    TIMER3_TAMR_R = 0x0A;

    // Calcular periodo total
    uint32_t period = Reloj_Sistema / freq;
    // Prescaler (8 bits más significativos)
    uint8_t prescale = (period >> 16) & 0xFF;
    // Parte baja del periodo
    uint16_t tail = (period & 0xFFFF) - 1;
    // Carga prescaler y periodo
    TIMER3_TAPR_R   = prescale;
    TIMER3_TAILR_R  = tail;

    // Duty cycle al 50% (puedes parametrizar)
    // Parte baja de match
    uint16_t match = (tail + 1) / 2;
    // Prescaler de match (la misma parte alta div 2)
    uint8_t prescaleMatch = prescale / 2;
    TIMER3_TAPMR_R       = prescaleMatch;
    TIMER3_TAMATCHR_R    = match - 1;

    // Habilita Timer3A y comienza PWM
    TIMER3_CTL_R |= 0x141;
}

void config_timer3b_interrupt(uint32_t ms) {
    uint32_t ticks = (Reloj_Sistema/1000) * ms;
    
    // Ya habilitado TIMER3 en PWM
    // Configurar B como periódica
    TimerDisable(TIMER3_BASE, TIMER_B);
    TimerLoadSet(TIMER3_BASE, TIMER_B, ticks - 1);
    TimerIntClear(TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    TimerIntEnable(TIMER3_BASE, TIMER_TIMB_TIMEOUT);
    IntPrioritySet(INT_TIMER3B, 0x00);
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
