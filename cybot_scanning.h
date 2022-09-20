/*
 * cybot_scanning.h
 *
 *  Created on: Apr 28, 2022
 *      Author: spotter
 */

#ifndef CYBOT_SCANNING_H_
#define CYBOT_SCANNING_H_

extern volatile int right_calibration_value;
extern volatile int left_calibration_value;

typedef struct{
    float sound_dist;
    int IR_raw_val;
} cyBOT_Scan_t;

void cyBOT_init_Scan();

void cyBOT_Scan(int angle, cyBOT_Scan_t* getScan);

typedef struct{
    int right;  // Right (0 degree) calibration value
    int left;   // Left (180 degree) calibration value
} cyBOT_SERVRO_cal_t;

//cyBOT_SERVRO_cal_t cyBOT_SERVO_cal(void);





#endif /* CYBOT_SCANNING_H_ */
