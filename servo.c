/*
 * servo.c
 *
 *  Created on: Apr 8, 2022
 *      Author: spotter
 */

#include "servo.h"
#include <math.h>
#include "timer.h"


void servo_init (void) {
    SYSCTL_RCGCGPIO_R |= 0x2; //clock on port B
    SYSCTL_RCGCTIMER_R |= 0x2; //clock on timer 1

    while ((SYSCTL_PRGPIO_R & 0x2) == 0) {}; //wait for clock on port B
    while ((SYSCTL_PRTIMER_R & 0x2) == 0) {}; //wait for clock on timer 1

    GPIO_PORTB_DIR_R &= ~0x20;
    GPIO_PORTB_DEN_R |= 0x20;

    GPIO_PORTB_AFSEL_R |= 0x20; //set bit 5
    GPIO_PORTB_PCTL_R |= 0x700000; //set bits 22-20 for PMC5 = 7

    TIMER1_CTL_R &= ~0x100; //disable timer 1
    TIMER1_CFG_R |= 0x4;
    TIMER1_TBMR_R &= ~0x4;
    TIMER1_TBMR_R |= 0xA;
    TIMER1_TBMR_R &= ~0x10; //sets count down mode

//    TIMER1_CTL_R &= ~0x8000;
//    TIMER1_TBPR_R |= 0xFF; //prescale
//    TIMER1_TBILR_R |= 0xFFFF; //start timer at max

    TIMER1_CTL_R |= 0x100; //re-enable timer
}

void servo_move (uint16_t degrees, int left,int right) {

     int actual135 = left;
     int actual45 = right;

    TIMER1_CTL_R &= ~0x100;
//    GPIO_PORTB_AFSEL_R &= ~0x20; //?

    TIMER1_TBILR_R = 0xE200;
    TIMER1_TBPR_R = 0x4;

//    float ms = degrees / 180.0 + 1.0;

    float ms = (degrees - (3.5 * actual45 - 2.5 * actual135)) / (2 * (actual135 - actual45));

    unsigned long cyclesHigh = 320000 - round(16000 * ms);

    TIMER1_TBMATCHR_R = cyclesHigh & 0xFFFF;
    TIMER1_TBPMR_R = cyclesHigh >> 16;


//    GPIO_PORTB_AFSEL_R |= 0x20;
    TIMER1_CTL_R |= 0x100;
    timer_waitMillis(30);
}

