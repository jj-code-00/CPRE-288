/**
 * Driver for ping sensor
 * @file ping.c
 * @author
 */

#include "ping_template.h"
#include "Timer.h"
#include <stdio.h>

volatile unsigned long START_TIME = 0;
volatile unsigned long END_TIME = 0;
volatile enum{LOW, HIGH, DONE} STATE = LOW; // State of ping echo pulse
volatile unsigned long pulseWidth = 0;
volatile int numOverflows = 0;
volatile float numMilliseconds = 0;

void ping_init (void){

  // YOUR CODE HERE
    SYSCTL_RCGCGPIO_R |= 0x2; //clock on port B
    SYSCTL_RCGCTIMER_R |= 0x8; //clock on timer 3

    while ((SYSCTL_PRGPIO_R & 0x2) == 0) {}; //wait for clock on port B
    while ((SYSCTL_PRTIMER_R & 0x8) == 0) {}; //wait for clock on timer 3

    GPIO_PORTB_DIR_R &= ~0x8;
    GPIO_PORTB_DEN_R |= 0x08;

    GPIO_PORTB_AFSEL_R |= 0x08;
    GPIO_PORTB_PCTL_R |= 0x7000;



    TIMER3_CTL_R &= ~0x100; //disable timer B step 1

    TIMER3_CFG_R |= 0x4; //step 2

    TIMER3_TBMR_R |= 0x7; //step 3
    TIMER3_TBMR_R &= ~0x10; //step 3

    TIMER3_CTL_R |= 0xC00; //step 4

    TIMER3_TBPR_R |= 0xFF; //step 5

    TIMER3_TBILR_R |= 0xFFFF; //step 6

    TIMER3_IMR_R |= 0x400; //step 7

    TIMER3_ICR_R |= 0x400;
    NVIC_EN1_R |= 0x10;

//    NVIC_PRI9_R |= 0xe0e0e000; //set all other priority to max
//    NVIC_PRI9_R &= ~0xE0; //set priority to Timer3B

    NVIC_PRI9_R |= 0x20;

    IntRegister(INT_TIMER3B, TIMER3B_Handler);

    IntMasterEnable();

    // Configure and enable the timer step 8
    TIMER3_CTL_R |= 0x100;
}

void ping_trigger (void){
    STATE = LOW;
    // Disable timer and disable timer interrupt
    TIMER3_CTL_R &= ~0x100;
    TIMER3_IMR_R &= ~0x400;
    // Disable alternate function (disconnect timer from port pin)
    GPIO_PORTB_AFSEL_R &= ~0x08;

    // YOUR CODE HERE FOR PING TRIGGER/START PULSE
    GPIO_PORTB_DIR_R |= 0x8; //set GPIO output
    GPIO_PORTB_DATA_R |= 0x8; // data 1 port B
    STATE = HIGH;
    //wait goes here
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~0x8; //data 0 port B
    STATE = LOW;
    GPIO_PORTB_DIR_R &= ~0x8; //set GPIO input

    // Clear an interrupt that may have been erroneously triggered
    TIMER3_ICR_R |= 0x400;

    // Re-enable alternate function, timer interrupt, and timer
    GPIO_PORTB_AFSEL_R |= 0x08;
    TIMER3_IMR_R |= 0x400;;
    TIMER3_CTL_R |= 0x100;
}

void TIMER3B_Handler(void){

  // YOUR CODE HERE

    if (STATE == LOW){
            START_TIME = TIMER3_TBR_R;
            STATE = HIGH;
    }

    else if (STATE == HIGH){
        END_TIME = TIMER3_TBR_R;
        STATE = DONE;
    }

    TIMER3_ICR_R |= 0x400;
}

float ping_getDistance (void){
    ping_trigger();
    timer_waitMillis(18.5);
    if (START_TIME < END_TIME) {
        pulseWidth = 0xFFFFFF - END_TIME + START_TIME;
        numOverflows++;
    }
    else {
        pulseWidth = START_TIME - END_TIME;
    }

    numMilliseconds = pulseWidth / 16000.0;
    return (pulseWidth / 32000000.0) * 34300.0;
}
