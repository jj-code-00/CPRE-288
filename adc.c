#include "adc.h"

void adc_init (void){

    SYSCTL_RCGCADC_R |= 0x1;
    SYSCTL_RCGCGPIO_R |= 0x2;

    while ((SYSCTL_RCGCADC_R & 0x1) == 0) {}; //may need to be removed
    while ((SYSCTL_RCGCGPIO_R & 0x2) == 0) {};

    GPIO_PORTB_AFSEL_R |= 0x03;

    //GPIO_PORTB_DEN_R &= ~0x03;

    GPIO_PORTB_AMSEL_R |= 0x10;

    ADC0_SSPRI_R &= ~0x123;
    ADC0_SSPRI_R |= 0x3210;

    ADC0_ACTSS_R &= ~0x1;
    ADC0_EMUX_R &= ~0xF;

    ADC0_SSMUX0_R &= ~0x000F;
    ADC0_SSMUX0_R += 10;

    ADC0_SSCTL0_R |= 0x0006;
    ADC0_IM_R &= ~0x0001;
    ADC0_ACTSS_R |= 0x0001;

}

uint16_t adc_read (void){
    ADC0_PSSI_R = 0x0001;
    while((ADC0_RIS_R & 0x01)== 0){};
    uint16_t result = ADC0_SSFIFO0_R & 0xFFF;
    ADC0_ISC_R = 0x0001;
    return result;
}
