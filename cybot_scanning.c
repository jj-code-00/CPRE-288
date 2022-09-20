#include "cybot_scanning.h"
#include "adc.h"
#include "servo.h"
#include "ping_template.h"
#include "timer.h"

volatile int right_calibration_value = 77;
volatile int left_calibration_value = 130;

void cyBOT_init_Scan(){
    adc_init();
    servo_init();
//    ping_init();
}

void cyBOT_Scan(int angle, cyBOT_Scan_t* getScan){
    servo_move(angle, left_calibration_value, right_calibration_value);
    getScan->IR_raw_val = adc_read();
//    getScan->sound_dist = ping_getDistance();
}

