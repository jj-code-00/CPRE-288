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



const int IRVALUE = 2000;
int numObjects;
int numObjects2;

typedef struct
{
    double distanceCM;
    int radialWidth;
    int angle;
    double radiusCM;
    int enabled;
} object_Record;

object_Record confirmedObjects[10];
object_Record confirmedObjects2[10];




#define SIZE 20
#define FSIZEX 102
#define FSIZEY 26

object_Record mans[SIZE];
char field[FSIZEY][FSIZEX];

void mapping(){


    int b;
    for (b = 0;b<SIZE;b++){
        mans[b] = confirmedObjects[b];
    }
//    int p = 0;
//    for (b=SIZE / 2;b<SIZE;b++){
//        mans[b] = confirmedObjects2[p];
//        mans[b].angle = confirmedObjects2[p].angle +180.0;
//        p++;
//    }


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

            if(j == FSIZEY-1 && k == FSIZEX /2){
                field[j][k] = 'o';
            }
            if (j == FSIZEX -1){
                field[j][k] = '\0';
            }
        }
    }

    int botX = FSIZEX /2;
    int botY = FSIZEY -1;
    int i;
    double radianAngle;
    for (i = 0; i< SIZE; i ++){
        int x;
        int y;
        int xCord;
        int yCord;
        //TODO add checks for width to determine X or *
        if (mans[i].angle < 90 && mans[i].distanceCM != 0){
            radianAngle = mans[i].angle*(3.14/180);
            x = round(mans[i].distanceCM * cos(radianAngle));
            y = round((mans[i].distanceCM * sin(radianAngle))/2.0);
            xCord = botX + x;
            yCord = botY - y;
            field[yCord][xCord] = 'X';
        }
        else if (mans[i].angle > 90){
            radianAngle = (180-mans[i].angle)*(3.14/180);
            x = round(mans[i].distanceCM * cos(radianAngle));
            y = round((mans[i].distanceCM * sin(radianAngle))/2.0);
            xCord = botX - x;
            yCord = botY - y;
            field[yCord][xCord] = 'X';
        }
        else {
            y = round(mans[i].distanceCM / 2.0);
            xCord = 0;
            yCord = botY - y;
            field[yCord][xCord] = 'X';
        }
    }
    int l;
    int z;
    for (l = 0;l <FSIZEY; l++){
        for (z = 0; z<FSIZEX; z++){
            char send = field[l][z];
            uart_sendChar(send);

//            printf("%c",field[l][z]);
        }
        uart_sendChar('\n');
        uart_sendChar('\r');
//        printf("\n");
    }
}

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
 * Function that does two scans and populates a global struct array
 */
void scan(cyBOT_Scan_t scanData)
{
    numObjects = 0;
    int i;
    int IRValues[91][3];
    int objectInFront;
    int indexStart = 0;
    int indexStop = 0;

    //reset struct to empty state
    int k;
    for (k = 0; k < 10; k++)
    {
        confirmedObjects[k].angle = 0;
        confirmedObjects[k].distanceCM = 0;
        confirmedObjects[k].radialWidth = 0;
        confirmedObjects[k].radiusCM = 0;
        confirmedObjects[k].enabled = 1;
    }
    char IRValuess[5];

    //reset position to 0
//    cyBOT_Scan(0, &scanData);
//    cyBOT_Scan(0, &scanData);

    //first scan printing IR Values to lcd and adding them to IRValues arrays first col
    for (i = 0; i <= 90; i++)
    {
        cyBOT_Scan(2 * i, &scanData);
        IRValues[i][0] = scanData.IR_raw_val;
        sprintf(IRValuess,"%d",scanData.IR_raw_val);
        lcd_printf(IRValuess);
    }

    //second scan printing IR Values to lcd and adding them to IRValues arrays second col
    for (i = 90; i >= 0; i--)
    {
        cyBOT_Scan(2 * i, &scanData);
        IRValues[i][1] = scanData.IR_raw_val;
        sprintf(IRValuess,"%d",scanData.IR_raw_val);
        lcd_printf(IRValuess);
    }

    //average each IRValue on every other angle measure
    for (i = 0; i <= 90; i++)
    {
        IRValues[i][2] = (IRValues[i][0] + IRValues[i][1]) / 2;
    }

    //update objects
    objectInFront = 0;
    for (i = 0; i <= 90; i++)
    {
        //new object found, save its starting index and update objectInFront
        if (IRValues[i][2] > IRVALUE && !objectInFront /*&& roundDist > 5
         && roundDist < 50 && i > 10*/)
        {
            indexStart = i;
            objectInFront = 1;
        }

        //continuing an object, just needs to continue to next loop
        else if (IRValues[i][2] > IRVALUE )
        {
            continue;
        }

        //end of an object. Save indexStop and set object in from to 0
        else if (IRValues[i][2] <= IRVALUE && objectInFront == 1)
        {
            indexStop = i;
            objectInFront = 0;

            //if it was only caught in less than 1 index it wont be saved as an object
            if (indexStop - indexStart > 1 && indexStop < 90 && indexStart > 0)
            {
                confirmedObjects[numObjects].angle = (indexStart + indexStop);
                confirmedObjects[numObjects].radialWidth = (indexStop * 2) - (indexStart * 2);
                numObjects += 1;
            }
        }

        //no object found, to continue, or end
        else continue;
    }

    //loop to find distance of all confirmed objects, newIndex is meant to be used for stacking
    int newIndex = numObjects;
    for (i = 0; i < numObjects; i++)
    {

        //current scans distance to object
        double object1distance = 0;

        //scan once and update object1distance, then set distanceCM and radiusCM
        cyBOT_Scan(confirmedObjects[i].angle, &scanData);
        cyBOT_Scan(confirmedObjects[i].angle, &scanData);
        object1distance += IRdistance();
        cyBOT_Scan(confirmedObjects[i].angle, &scanData);
        object1distance += IRdistance();
        object1distance = object1distance / 2;
        confirmedObjects[i].distanceCM = object1distance;
        confirmedObjects[i].radiusCM = tan((confirmedObjects[i].radialWidth / 2.0)* (M_PI/180.0)) * confirmedObjects[i].distanceCM;
        if(confirmedObjects[i].distanceCM > 50.0){
            confirmedObjects[i].enabled = 0;
        }
    }
    numObjects = newIndex;
    mapping();
}

void scan360(cyBOT_Scan_t scanData, oi_t *sensor_data)
{
    scan(scanData);
    turn_left(sensor_data,180);
    numObjects2 = 0;
    int i;
    int IRValues[91][3];
    int objectInFront;
    int indexStart = 0;
    int indexStop = 0;

    //reset struct to empty state
    int k;
    for (k = 0; k < 10; k++)
    {
        confirmedObjects2[k].angle = 0;
        confirmedObjects2[k].distanceCM = 0;
        confirmedObjects2[k].radialWidth = 0;
        confirmedObjects2[k].radiusCM = 0;
        confirmedObjects2[k].enabled = 1;
    }
    char IRValuess[5];

    //reset position to 0
//    cyBOT_Scan(0, &scanData);
//    cyBOT_Scan(0, &scanData);

    //first scan printing IR Values to lcd and adding them to IRValues arrays first col
    for (i = 0; i <= 90; i++)
    {
        cyBOT_Scan(2 * i, &scanData);
        IRValues[i][0] = scanData.IR_raw_val;
        sprintf(IRValuess,"%d",scanData.IR_raw_val);
        lcd_printf(IRValuess);
    }

    //second scan printing IR Values to lcd and adding them to IRValues arrays second col
    for (i = 90; i >= 0; i--)
    {
        cyBOT_Scan(2 * i, &scanData);
        IRValues[i][1] = scanData.IR_raw_val; //updates i to be an IR value
        sprintf(IRValuess,"%d",scanData.IR_raw_val);
        lcd_printf(IRValuess);
    }

    //average each IRValue on every other angle measure
    for (i = 0; i <= 90; i++)
    {
        IRValues[i][2] = (IRValues[i][0] + IRValues[i][1]) / 2;
    }

    //update objects
    objectInFront = 0;
    for (i = 0; i <= 90; i++)
    {
        //new object found, save its starting index and update objectInFront
        if (IRValues[i][2] > IRVALUE && !objectInFront)
        {
            indexStart = i;
            objectInFront = 1;
        }

        //continuing an object, just needs to continue to next loop
        else if (IRValues[i][2] > IRVALUE )
        {
            continue;
        }

        //end of an object. Save indexStop and set object in from to 0
        else if (IRValues[i][2] <= IRVALUE && objectInFront == 1)
        {
            indexStop = i;
            objectInFront = 0;

            //if it was only caught in less than 1 index it wont be saved as an object
            if (indexStop - indexStart > 1 && indexStop < 90 && indexStart > 0)
            {
                confirmedObjects2[numObjects2].angle = (indexStart + indexStop);
                confirmedObjects2[numObjects2].radialWidth = (indexStop * 2) - (indexStart * 2);
                numObjects2 += 1;
            }
        }

        //no object found, to continue, or end
        else continue;
    }

    //loop to find distance of all confirmed objects, newIndex is meant to be used for stacking
    int newIndex = numObjects2;
    for (i = 0; i < numObjects2; i++)
    {

        //current scans distance to object
        double object1distance = 0;

        //scan once and update object1distance, then set distanceCM and radiusCM
        cyBOT_Scan(confirmedObjects2[i].angle, &scanData);
        cyBOT_Scan(confirmedObjects2[i].angle, &scanData);
        object1distance += IRdistance();
        cyBOT_Scan(confirmedObjects2[i].angle, &scanData);
        object1distance += IRdistance();
        object1distance = object1distance / 2;
        confirmedObjects2[i].distanceCM = object1distance;
        confirmedObjects2[i].radiusCM = tan((confirmedObjects2[i].radialWidth / 2.0)* (M_PI/180.0)) * confirmedObjects2[i].distanceCM;
        if(confirmedObjects2[i].distanceCM > 50.0){
            confirmedObjects2[i].enabled = 0;
        }
    }
    numObjects2 = newIndex;
    turn_right(sensor_data,180);

}

/**
 * function to move forward while getting out of the way if it bumps into anything
 */
int move_forward_mission(oi_t *sensor_data, double distance_mm){
    double totalDistance = 0;

    //start moving forward
    oi_setWheels(150,150);

    //while distance traveled is less than distance to travel continue
    while (totalDistance < distance_mm){
        lcd_printf("%f",totalDistance);
        oi_update(sensor_data);

        //if either bump sensor triggers move backwards then turn either right or left, then turn the opposite to straighten out, then move forward
        //returns 1 to show something has been bumped into
        if (sensor_data -> bumpRight){
            move_backwards(sensor_data,50);
            lcd_printf("%f",totalDistance);
            turn_left(sensor_data,90);
            move_forward(sensor_data, 50);
            lcd_printf("%f",totalDistance);
            turn_right(sensor_data,90);
            totalDistance -= 100;
            oi_setWheels(0,0);
            return 1;
        }
        if (sensor_data -> bumpLeft){
            move_backwards(sensor_data,50);
            lcd_printf("%f",totalDistance);
            turn_right(sensor_data,90);
            move_forward(sensor_data, 50);
            lcd_printf("%f",totalDistance);
            turn_left(sensor_data,90);
            totalDistance -= 100;
            oi_setWheels(0,0);
            return 1;
        }

        //if nothing is bumped into then keep driving forward until totaldistance == distance
        oi_setWheels(100, 100);
        totalDistance += sensor_data->distance;
    }

    //stop movement and return 0 for no bump
    oi_setWheels(0,0);
    return 0;
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
//    ping_init();
//
//    float distA = ping_getDistance();
//    distA = ping_getDistance();

//    servo_move(45);
//    timer_waitMicros(5000);
//    servo_move(135);
//    timer_waitMicros(5000);

    //initialize scanning tools and set calibrations

    right_calibration_value = 280000;
    left_calibration_value = 1267000;
    cyBOT_Scan_t scanData;

    //until putty input is q continue
    while (command_byte != 'q')
    {
        //if input is m then perform scanning
        if (command_byte == 'm' && command_flag == 1)
        {
            command_flag = 0;

            uart_sendStr("\n\r----------------------\n\r");

            while(1){
                scan(scanData);
                char angle[4];
                char distance[10];
                char radialWidth[4];
                char radius[10];
                char enable[2];
                int j;
                int l = 0;
                double smallestWidth = -1;
                while (smallestWidth == -1){
                    if(confirmedObjects[l].enabled == 1){
                        smallestWidth = confirmedObjects[l].radiusCM;
                    }
                    l++;
                }

                //0 for confirmedObjects 1 for confirmedObjects2
                int origOrTwo = 0;
                int smallestIndex = 0;
                for (j = 0; j < numObjects; j++)
                {
                    if (smallestWidth >= confirmedObjects[j].radiusCM
                            && confirmedObjects[j].enabled == 1)
                    {
                        smallestIndex = j;
                        smallestWidth = confirmedObjects[j].radiusCM;
                        origOrTwo = 0;
                    }

                    sprintf(angle, "%d", confirmedObjects[j].angle);
                    sprintf(distance, "%f", confirmedObjects[j].distanceCM);
                    sprintf(radialWidth, "%d", confirmedObjects[j].radialWidth);
                    sprintf(radius, "%f", confirmedObjects[j].radiusCM);
                    sprintf(enable, "%d", confirmedObjects[j].enabled);

                    uart_sendStr(angle);
                    uart_sendStr(" degrees\n\r");
                    uart_sendStr(distance);
                    uart_sendStr(" cm\n\r");
                    uart_sendStr(radialWidth);
                    uart_sendStr(" radial width\n\r");
                    uart_sendStr(radius);
                    uart_sendStr(" radius cm\n\r");
                    uart_sendStr("Enabled: ");
                    uart_sendStr(enable);
                    uart_sendStr("\n\r\n");

                }

                for (j = 0; j < numObjects2; j++)
                {
                    if (smallestWidth >= confirmedObjects2[j].radiusCM
                            && confirmedObjects2[j].enabled == 1)
                    {
                        smallestIndex = j;
                        smallestWidth = confirmedObjects2[j].radiusCM;
                        origOrTwo = 1;
                    }

                    sprintf(angle, "%d", confirmedObjects2[j].angle);
                    sprintf(distance, "%f", confirmedObjects2[j].distanceCM);
                    sprintf(radialWidth, "%d", confirmedObjects2[j].radialWidth);
                    sprintf(radius, "%f", confirmedObjects2[j].radiusCM);
                    sprintf(enable, "%d", confirmedObjects2[j].enabled);

                    uart_sendStr(angle);
                    uart_sendStr(" degrees\n\r");
                    uart_sendStr(distance);
                    uart_sendStr(" cm\n\r");
                    uart_sendStr(radialWidth);
                    uart_sendStr(" radial width\n\r");
                    uart_sendStr(radius);
                    uart_sendStr(" radius cm\n\r");
                    uart_sendStr("Enabled: ");
                    uart_sendStr(enable);
                    uart_sendStr("\n\r\n");

                }

                int angleToTurn = 0;
                if (origOrTwo == 1){
                    angleToTurn = confirmedObjects2[smallestIndex].angle;
                    turn_left(sensor_data,180);
                }
                else {
                    angleToTurn = confirmedObjects[smallestIndex].angle;
                }
                if (angleToTurn < 90)
                {
                    int turn = 90 - angleToTurn;
                    turn_right(sensor_data, turn);
                }
                else if (angleToTurn > 90)
                {
                    int turn = angleToTurn - 90;
                    turn_left(sensor_data, turn);
                }
                double distanceToMove = (confirmedObjects[smallestIndex].distanceCM * 10.0) - 50.0;

                int bumped = move_forward_mission(sensor_data, distanceToMove);

               if(bumped == 1){
                   bumped = 0;
                   continue;
               }
               else{
                   break;
               }
            }

        }
        //TODO needs testing
        else if (command_byte == 'w')
        {
            while (command_byte != 'q')
            {
                command_flag = 0;
                if (command_byte == 'w'  && command_flag == 1)
                {
                    move_forward(sensor_data, 1);
                }
                if (command_byte == 'a' && command_flag == 1)
                {
                    turn_left(sensor_data, 15);
                }
                if (command_byte == 's' && command_flag == 1)
                {
                    move_backwards(sensor_data, 1);
                }
                if (command_byte == 'd' && command_flag == 1)
                {
                    turn_right(sensor_data, 15);
                }
            }
        }
        else if (command_byte == 'c')
        {
            servo_move(45);
            timer_waitMicros(5000);
            servo_move(135);
            timer_waitMicros(5000);
        }
    }

    oi_free(sensor_data);
}
