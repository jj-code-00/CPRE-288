/*
 * main.c
 *
 *  Created on: Mar 11, 2022
 *      Author: spotter
 */

#include "Timer.h"
//#include "lcd.h"
#include "cyBot_scanning.h"  // For scan sensors
#include "uart-interrupt.h"
#include "driverlib/interrupt.h"
#include <math.h>
#include "open_interface.h"
#include "movement.h"
#include "adc.h"
#include "servo.h"
#include "ping_template.h"
#include <stdio.h>
const int IRVALUE = 1000;

#define SIZE 20
#define FSIZEX 102
#define FSIZEY 26

int currentAngle = 90;
double currentX = 0;
double currentY = 0;

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

void scanV2(cyBOT_Scan_t scanData){
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


            if (j == FSIZEX -1){
                field[j][k] = '\0';
            }
            if ((k >= botX - 14 && k <= botX + 14) && j == botY){
                field[j][k] = '-';
            }
            if (j == botY && k == botX)
            {
                field[j][k] = 'o';
            }
        }
    }
    int x;
    int y;
    double radianAngle;

    for (j = 0; j< 180; j++){
        cyBOT_Scan(j, &scanData);
        dist = IRdistance();

        if (scanData.IR_raw_val >= IRVALUE){
            if ( j< 90){
                radianAngle = j*(M_PI/180);
                x = round(dist * cos(radianAngle));
                y = round((dist * sin(radianAngle))/2.0);
                x = botX + x;
                y = botY - y;
                if ((x < FSIZEX - 1 && y < FSIZEY - 1)
                        && (x >= 0 && y >= 0))
                {
                    field[y][x] = 'X';
                }
            }
            else if (j > 90)
            {
                radianAngle = (180 - j) * (M_PI / 180);
                x = round(dist * cos(radianAngle));
                y = round((dist * sin(radianAngle)) / 2.0);
                x = botX - x;
                y = botY - y;
                if ((x < FSIZEX - 1 && y < FSIZEY - 1)
                        && (x>= 0 && y >= 0))
                {
                    field[y][x] = 'X';
                }
            }
            else
            {
                y = round(dist / 2.0);
                x = 0;
                y = botY - y;
                if ((x < FSIZEX - 1 && y < FSIZEY - 1)
                        && (x >= 0 && y >= 0))
                {
                    field[y][x] = 'X';
                }
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
//    lcd_init();
    uart_interrupt_init();

    oi_t *sensor_data = oi_alloc(); // do this only once at start of main()
    oi_init(sensor_data); // do this only once at start of main()
    cyBOT_init_Scan();
    double radAngle = 0;
    char currX[16] = " ";
    char currY[16]= " ";
    char currAngle[4] = " ";
    int bump;
//    serverCal();
    right_calibration_value = 77;
    left_calibration_value = 130;

    //initialize scanning tools and set calibrations
    cyBOT_Scan_t scanData;
    //until putty input is q continue
    while (1)
    {
        bump = 0;
        //if input is m then perform scanning
        if (command_byte == 'm' && command_flag == 1)
        {
            uart_sendStr("Scanning...\n\r");
            scanV2(scanData);
            command_flag = 0;
            command_byte = ' ';
            currentX = 0;
            currentY = 0;
            currentAngle = 90;

        }
        else if (command_byte == 'w' && command_flag == 1)
        {

            bump = move_forward_mission(sensor_data, 50);
            radAngle = currentAngle*(M_PI/180);
            currentX += 5.0 * cos(radAngle);
            currentY += 5.0 * sin(radAngle);
            if (bump == 1)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Bumped Right\n\r");
            }
            else if (bump == 2)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Bumped Left\n\r");
            }
            else if (bump == 3)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Cliff: Left\n\r");
            }
            else if (bump == 4)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Boundary: Front Left\n\r");
            }
            else if (bump == 5)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Cliff: Front Left\n\r");
            }
            else if (bump == 6)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Boundary: Front Right\n\r");
            }
            else if (bump == 7)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Cliff: Front Right\n\r");
            }
            else if (bump == 8)
            {
                radAngle = currentAngle * (M_PI / 180);
                currentX -= 5.0 * cos(radAngle);
                currentY -= 5.0 * sin(radAngle);
                uart_sendStr("Cliff: Right\n\r");
            }
            snprintf(currX,16,"%f",currentX);
            snprintf(currY,16,"%f",currentY);
            uart_sendStr("X: ");
            uart_sendStr(currX);
            uart_sendStr(" Y: ");
            uart_sendStr(currY);
            uart_sendStr("\n\r");
            command_flag = 0;
            command_byte = ' ';
        }
        else if (command_byte == 'a' && command_flag == 1)
        {
            turn_left(sensor_data, 15);
            command_flag = 0;
            command_byte = ' ';
            currentAngle += 15;
            snprintf(currAngle,4,"%d",currentAngle);
            uart_sendStr("Angle: ");
            uart_sendStr(currAngle);
            uart_sendStr("\n\r");
        }
        else if (command_byte == 's' && command_flag == 1)
        {
            radAngle = currentAngle*(M_PI/180);
            currentX -= 5.0 * cos(radAngle);
            currentY -= 5.0 * sin(radAngle);
            move_backwards(sensor_data, 50);
            snprintf(currX,16, "%f", currentX);
            snprintf(currY,16, "%f", currentY);
            uart_sendStr("X: ");
            uart_sendStr(currX);
            uart_sendStr(" Y: ");
            uart_sendStr(currY);
            uart_sendStr("\n\r");
            command_flag = 0;
            command_byte = ' ';

        }
        else if (command_byte == 'd' && command_flag == 1)
        {
            turn_right(sensor_data, 15);
            command_flag = 0;
            command_byte = ' ';
            currentAngle -= 15;
            snprintf(currAngle,4, "%d", currentAngle);
            uart_sendStr("Angle: ");
            uart_sendStr(currAngle);
            uart_sendStr("\n\r");
        }
        else if (command_byte == 'f' && command_flag == 1)
        {
            oi_update(sensor_data);
            char cliffLeft[25];
            char cliffFrontLeft[25];
            char cliffFrontRight[25];
            char cliffRight[25];
            snprintf(cliffLeft,25, "%d", sensor_data -> cliffLeftSignal);
            snprintf(cliffFrontLeft,25, "%d", sensor_data -> cliffFrontLeftSignal);
            snprintf(cliffFrontRight,25, "%d", sensor_data -> cliffFrontRightSignal);
            snprintf(cliffRight,25, "%d", sensor_data -> cliffRightSignal);
            uart_sendStr(cliffLeft);
            uart_sendStr("\n\r");
            uart_sendStr(cliffFrontLeft);
            uart_sendStr("\n\r");
            uart_sendStr(cliffFrontRight);
            uart_sendStr("\n\r");
            uart_sendStr(cliffRight);
            uart_sendStr("\n\r");
            command_flag = 0;
            command_byte = ' ';
        }
        else if (command_byte == 'z' && command_flag == 1){
            turn_left(sensor_data, 90);
            command_flag = 0;
            command_byte = ' ';
            currentAngle += 90;
            snprintf(currAngle, 4, "%d", currentAngle);
            uart_sendStr("Angle: ");
            uart_sendStr(currAngle);
            uart_sendStr("\n\r");
        }
        else if (command_byte == 'c' && command_flag == 1)
        {
            turn_right(sensor_data, 90);
            command_flag = 0;
            command_byte = ' ';
            currentAngle -= 90;
            snprintf(currAngle, 4, "%d", currentAngle);
            uart_sendStr("Angle: ");
            uart_sendStr(currAngle);
            uart_sendStr("\n\r");
        }
    }
//    oi_free(sensor_data);
}
