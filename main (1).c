/*
 * main.c
 *
 *  Created on: Mar 11, 2022
 *      Author: spotter
 */

#include "Timer.h"
#include "lcd.h"
#include "cyBot_scanning.h"  // For scan sensors
#include "uart-interrupt.h"
#include <stdbool.h>
#include "driverlib/interrupt.h"
#include <string.h>
#include <math.h>
#include "open_interface.h"
#include "movement.h"
#include "adc.h"
#include "servo.h"
#include "ping_template.h"



const int IRVALUE = 700;

#define SIZE 20
#define FSIZEX 102
#define FSIZEY 26

char field[FSIZEY][FSIZEX];

double IRdistance() {
    uint16_t avg = 0;
    uint16_t rawIRValue = 0;
    int i;
    for (i = 0; i < 16; i++)
    {
        avg += adc_read();
    }

    rawIRValue = avg / i;
    return 2440000 * pow(rawIRValue, -1.56);
}

/**
 * function to move forward while getting out of the way if it bumps into anything
 */
int move_forward_mission(oi_t *sensor_data, double distance_mm){
    double totalDistance = 0;

    //start moving forward
    oi_setWheels(50,50);

    //while distance traveled is less than distance to travel continue
    while (totalDistance < distance_mm){
        oi_update(sensor_data);

        //if either bump sensor triggers move backwards then turn either right or left, then turn the opposite to straighten out, then move forward
        //returns 1 to show something has been bumped into
        if (sensor_data -> bumpRight){
            move_backwards(sensor_data,50);
            totalDistance -= 50;
            oi_setWheels(0,0);
            return 1;
        }
        if (sensor_data -> bumpLeft){
            move_backwards(sensor_data,50);
            totalDistance -= 50;
            oi_setWheels(0,0);
            return 2;
        }

        //TODO untested
        if (sensor_data->cliffFrontLeft){
            move_backwards(sensor_data,50);
            totalDistance -= 50;
            oi_setWheels(0,0);
            return 3;
        }

        if (sensor_data->cliffFrontRight){
            move_backwards(sensor_data,50);
            totalDistance -= 50;
            oi_setWheels(0,0);
            return 4;
        }

        //if nothing is bumped into then keep driving forward until totaldistance == distance
        oi_setWheels(50, 50);
        totalDistance += sensor_data->distance;
    }

    //stop movement and return 0 for no bump
    oi_setWheels(0,0);
    return 0;
}

//adjust these to calibrate
int right_calibration_value = 77;
int left_calibration_value = 130;

void scanV2(cyBOT_Scan_t scanData){
    uint16_t IRVal = 0;
    float dist = 0.0;
    int botX = FSIZEX /2;
    int botY = FSIZEY -1;

    int j;
    int k;
    for(j = 0; j<FSIZEY; j++){
        for (k=0;k<FSIZEX;k++){
            if (k % 2 ==0){
                field[j][k] = ' ';
            }
            else {
                field[j][k] = '+';
            }

            if(j == botY && k == botX){
                field[j][k] = 'o';
            }
            if (j == FSIZEX -1){
                field[j][k] = '\0';
            }
        }
    }

    int i;
    int x;
    int y;
    int xCord;
    int yCord;
    double radianAngle;
    int angle = 0;

    for (i = 0; i< 180; i ++){
        angle = i;

        cyBOT_Scan(angle, &scanData, left_calibration_value,right_calibration_value);
        IRVal = scanData.IR_raw_val;
        dist = IRdistance();

        if (IRVal >= IRVALUE){
            if ( angle< 90){
                radianAngle = angle*(3.14/180);
                x = round(dist * cos(radianAngle));
                y = round((dist * sin(radianAngle))/2.0);
                xCord = botX + x;
                yCord = botY - y;
                field[yCord][xCord] = 'X';
            }
            else if (angle > 90){
                radianAngle = (180-angle)*(3.14/180);
                x = round(dist * cos(radianAngle));
                y = round((dist * sin(radianAngle))/2.0);
                xCord = botX - x;
                yCord = botY - y;
                field[yCord][xCord] = 'X';
            }
            else {
                y = round(dist / 2.0);
                xCord = 0;
                yCord = botY - y;
                field[yCord][xCord] = 'X';
            }
        }
    }
    servo_move(0,left_calibration_value,right_calibration_value);
    int l;
    int z;
    for (l = 0; l < FSIZEY; l++)
    {
        for (z = 0; z < FSIZEX; z++)
        {
            char send = field[l][z];
            uart_sendChar(send);
        }
        uart_sendChar('\n');
        uart_sendChar('\r');

    }
}

//calibration function
void serverCal(){
    while (command_byte != 'q'){
        if (command_byte == '1' && command_flag == 1){
            command_flag = 0;
            servo_move(135, left_calibration_value,right_calibration_value);
        }

        if (command_byte == '2' && command_flag == 1){
            command_flag = 0;
            servo_move(45,left_calibration_value,right_calibration_value);
        }
    }
}

int main(void)
{
    //initialize timer, lcd, and uart
    timer_init(); // Must be called before lcd_init(), which uses timer functions
    lcd_init();
    uart_interrupt_init();

    oi_t *sensor_data = oi_alloc(); // do this only once at start of main()
    oi_init(sensor_data); // do this only once at start of main()
    cyBOT_init_Scan();
//    serverCal();

    //initialize scanning tools and set calibrations
    cyBOT_Scan_t scanData;
    int distance = 0;
    int botAngle = 90;
    //until putty input is q continue
    while (command_byte != 'q')
    {
        int bump = 0;
        //if input is m then perform scanning
        if (command_byte == 'm' && command_flag == 1)
        {
            uart_sendStr("Scanning...\n\r");
            scanV2(scanData);
            command_flag = 0;
            botAngle = 90;
            distance = 0;

        }
        else if (command_byte == 'w' && command_flag == 1)
        {

            bump = move_forward_mission(sensor_data, 50);
            distance += 5;
            if (bump == 1)
            {
                distance = 0
                uart_sendStr("Bumped Right\n\r");
            }
            else if (bump == 2)
            {
                distance = 0
                uart_sendStr("Bumped Left\n\r");
            }
            else if (bump == 3){
                distance = 0
                uart_sendStr("Cliff front Left\n\r");
            }
            else if (bump == 4)
            {
                distance = 0
                uart_sendStr("Cliff front Right\n\r");
            }
            command_flag = 0;
        }
        else if (command_byte == 'a' && command_flag == 1)
        {
            botAngle += 15;
            turn_left(sensor_data, 15);
            command_flag = 0;
        }
        else if (command_byte == 's' && command_flag == 1)
        {

            move_backwards(sensor_data, 50);
            command_flag = 0;
        }
        else if (command_byte == 'd' && command_flag == 1)
        {
            botAngle -= 15;
            turn_right(sensor_data, 15);
            command_flag = 0;
        }
        else if (command_byte == 'f' && command_flag == 1)
        {
            oi_update(sensor_data);
            char cliffLeft[25];
            char cliffFrontLeft[25];
            char cliffFrontRight[25];
            char cliffRight[25];
            sprintf(cliffLeft, "%d", sensor_data -> cliffLeftSignal);
            sprintf(cliffFrontLeft, "%d", sensor_data -> cliffFrontLeftSignal);
            sprintf(cliffFrontRight, "%d", sensor_data -> cliffFrontRightSignal);
            sprintf(cliffRight, "%d", sensor_data -> cliffRightSignal);
            uart_sendStr(cliffLeft);
            uart_sendStr("\n\r");
            uart_sendStr(cliffFrontLeft);
            uart_sendStr("\n\r");
            uart_sendStr(cliffFrontRight);
            uart_sendStr("\n\r");
            uart_sendStr(cliffRight);
            uart_sendStr("\n\r");
            command_flag = 0;
        }
        else if (command_byte == 'r' && command_flag == 1){
            servo_move(0,left_calibration_value,right_calibration_value);
            command_flag = 0;
        }
    }

    oi_free(sensor_data);
}
