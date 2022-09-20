/*
 * movement.c
 *
 *  Created on: Feb 4, 2022
 *      Author: spotter
 */
#include "open_interface.h"
#include "Timer.h"
//#include "lcd.h"

const int OFFSET = 0;

//double move_forward (oi_t *sensor_data, double distance_mm){
//
//    // the following code could be put in function move_forward()
//        double sum = 0; // distance member in oi_t struct is type double
////        lcd_printf("%f", sum);
//        oi_setWheels(50,50);
//         while (sum < distance_mm) {
//          oi_update(sensor_data);
//          sum += sensor_data -> distance; // use -> notation since pointer
////          lcd_printf("%f", sum);
//         }
//        oi_setWheels(0,0); //stop
//        return 0;
//}

double turn_left(oi_t *sensor_data,double degrees){
    double current_degrees = 0;
    oi_setWheels(50,-50);
//    lcd_printf("%f", current_degrees);
    while (current_degrees < degrees - OFFSET){
        oi_update(sensor_data);
        current_degrees += sensor_data -> angle;
//        lcd_printf("%f", current_degrees);
    }
    oi_setWheels(0,0);
    return 0;
}

double turn_right(oi_t *sensor_data,double degrees){
    double current_degrees = 0;
    oi_setWheels(-50,50);
//    lcd_printf("%f", current_degrees);
    while (current_degrees > -1 * degrees + OFFSET){
        oi_update(sensor_data);
        current_degrees += sensor_data -> angle;
//        lcd_printf("%f", current_degrees);
    }
    oi_setWheels(0,0);
    return 0;
}
double move_backwards (oi_t *sensor_data, double distance_mm){

    // the following code could be put in function move_forward()
        double sum = 0; // distance member in oi_t struct is type double
//        lcd_printf("%f", sum);
        oi_setWheels(-50,-50); //move forward at full speed
         while (sum > distance_mm * -1) {
          oi_update(sensor_data);
          sum += sensor_data -> distance; // use -> notation since pointer
//          lcd_printf("%f", sum);
         }
        oi_setWheels(0,0); //stop
        return 0;
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

        if (sensor_data->cliffLeftSignal < 1000){
            move_backwards(sensor_data,50);
            oi_setWheels(0,0);
            return 3;
        }

        if (sensor_data->cliffFrontLeftSignal > 2700){
            move_backwards(sensor_data,50);
            oi_setWheels(0,0);
            return 4;
        }

        if (sensor_data->cliffFrontLeftSignal < 1000)
        {
            move_backwards(sensor_data, 50);
            oi_setWheels(0, 0);
            return 5;
        }

        if (sensor_data->cliffFrontRightSignal > 2700)
        {
            move_backwards(sensor_data, 50);
            oi_setWheels(0, 0);
            return 6;
        }

        if (sensor_data->cliffFrontRightSignal < 1000)
        {
            move_backwards(sensor_data, 50);
            oi_setWheels(0, 0);
            return 7;
        }

        if (sensor_data->cliffRightSignal < 1000)
        {
            move_backwards(sensor_data, 50);
            oi_setWheels(0, 0);
            return 8;
        }

        //if nothing is bumped into then keep driving forward until totaldistance == distance
        oi_setWheels(150, 150);
        totalDistance += sensor_data->distance;
    }

    //stop movement and return 0 for no bump
    oi_setWheels(0,0);
    return 0;
}

//double move_forward_obstacle(oi_t *sensor_data, double distance_mm){
//    double totalDistance = 0;
//    oi_setWheels(250,250);
//    while (totalDistance < distance_mm){
////        lcd_printf("%f",totalDistance);
//        oi_update(sensor_data);
//        if (sensor_data -> bumpRight){
//            move_backwards(sensor_data,150);
////            lcd_printf("%f",totalDistance);
//            turn_left(sensor_data,90);
//            move_forward(sensor_data, 250);
////            lcd_printf("%f",totalDistance);
//            turn_right(sensor_data,90);
//            totalDistance -= 150;
//        }
//        if (sensor_data -> bumpLeft){
//            move_backwards(sensor_data,150);
////            lcd_printf("%f",totalDistance);
//            turn_right(sensor_data,90);
//            move_forward(sensor_data, 250);
////            lcd_printf("%f",totalDistance);
//            turn_left(sensor_data,90);
//            totalDistance -= 150;
//        }
//        oi_setWheels(250,250);
//        totalDistance += sensor_data -> distance;
//    }
//    oi_setWheels(0,0);
//    return 0;
//}





