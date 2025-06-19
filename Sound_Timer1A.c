********************************************************************************************
 * Descripcion: El programa genera una melodía utilizando el TIMER 1A
 * Coneccion:   PB4 to BUZZER
********************************************************************************************/

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include <stdlib.h>

#define SW1 (1<<4)
#define Reloj_Sistema 16000000UL // 16 MHz

// ... (toda tu lista de notas se mantiene igual, es excelente)
#define NOTE_B0  31
#define NOTE_C1  33
// ... (etc.) ...
#define NOTE_DS8 4978
#define REST      0
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


/********************************************************************************************
 * SECCIÓN DE MELODÍAS CORREGIDAS PARA PAC-MAN
 ********************************************************************************************/

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


// ... (Tus funciones de configuración config_switches, config_timerB, etc. van aquí. No necesitan cambios)
void config_switches(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R5) == 0) {}

    GPIO_PORTF_CR_R |= SW1;
    GPIO_PORTF_DIR_R &= ~SW1;
    GPIO_PORTF_AFSEL_R &= ~SW1;
    GPIO_PORTF_PUR_R |= SW1;
    GPIO_PORTF_DEN_R |= SW1;
}

void config_timerB(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    while ((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R1) == 0) {}

    GPIO_PORTB_AFSEL_R |= (1 << 4);
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFFF0FFFF) | 0x00070000;
    GPIO_PORTB_DIR_R |= (1 << 4);
    GPIO_PORTB_DEN_R |= (1 << 4);
}

void config_pwm_pb4(uint32_t frecuencia){
    uint32_t periodo = Reloj_Sistema / frecuencia;

    SYSCTL_RCGCTIMER_R |= (1 << 1);
    while ((SYSCTL_PRTIMER_R & (1 << 1)) == 0);

    TIMER1_CTL_R &= ~(1 << 0);
    TIMER1_CFG_R = 0x4;
    TIMER1_TAMR_R = 0x0A;
    TIMER1_TAILR_R = periodo - 1;
    TIMER1_TAMATCHR_R = periodo / 2; // Ajustado a 50% de ciclo útil, común para buzzers
    TIMER1_CTL_R |= (1 << 0);
}

void stop_pwm_pb4(void){
    TIMER1_CTL_R &= ~(1 << 0);
}

void delay_ms(int tiempo_ms){
    NVIC_ST_CTRL_R = 0;
    NVIC_ST_RELOAD_R = 16000 - 1;
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R = 0x5;

    for(int i = 0; i < tiempo_ms; i++){
        while((NVIC_ST_CTRL_R & 0x10000) == 0);
    }

    NVIC_ST_CTRL_R = 0;
}

void playMelody(int* melody, int* durations, int length){
    for(int i = 0; i < length; i++){
        int base_duracion = abs(durations[i]);
        int duracion = 1000 / base_duracion;
        
        if(durations[i] < 0){
            duracion = duracion * 3 / 2;
        }
        
        if(melody[i] > 0){
            config_pwm_pb4(melody[i]);
        }
        
        delay_ms(duracion*1.3);
        stop_pwm_pb4();
    }
}

// ... (Tus funciones de sonido y config_leds van aquí)
void config_leds(void){
    GPIO_PORTF_DIR_R |= (1<<3) | (1<<2) | (1<<1);
    GPIO_PORTF_DR8R_R |= (1<<3) | (1<<2) | (1<<1);
    GPIO_PORTF_DEN_R |= (1<<3) | (1<<2) | (1<<1);
    GPIO_PORTF_DATA_R &= ~((1<<3) | (1<<2) | (1<<1));
}

void sonido_inicio(){
    playMelody(melody_inicio, durations_inicio, sizeof(melody_inicio)/sizeof(int));
}

void sonido_muerte(){
    playMelody(melody_muerte, durations_muerte, sizeof(melody_muerte)/sizeof(int));
}

void sonido_wakawaka(){
    playMelody(melody_wakawaka, durations_wakawaka, sizeof(melody_wakawaka)/sizeof(int));
}

void sonido_comerFantasma(){
    playMelody(melody_comerFantasma, durations_comerFantasma, sizeof(melody_comerFantasma)/sizeof(int));
}

void sonido_fantasmas(){
    playMelody(melody_fantasma, durations_fantasma, sizeof(melody_fantasma)/sizeof(int));
}

int main(void){
    config_switches();
    config_timerB();
    config_leds();

    // Toca la melodía de inicio una vez.
    //sonido_inicio();
    //delay_ms(1000);

    while(1){
        // ----- EJEMPLO DE USO -----
        
        // Toca la sirena de los fantasmas 5 veces
        for(int i=0; i<5; i++){
            //sonido_fantasmas();
        }
        delay_ms(500);

        // Toca el sonido de comer un fantasma
        //sonido_comerFantasma();
        //delay_ms(1000);
        
        // Toca el sonido de muerte
        sonido_muerte();
        delay_ms(1000);

        // Simula a Pac-Man comiendo 10 puntos (waka-waka)
        for(int i=0; i<10; i++){
            sonido_wakawaka();
            // No necesita delay extra, el sonido ya es corto.
        }
        delay_ms(1000);
    }
}
