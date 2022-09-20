/*
 * movement.h
 *
 *  Created on: Feb 4, 2022
 *      Author: spotter
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

//double move_forward (oi_t *sensor_data, double distance_mm);
double turn_right(oi_t *sensor_data,double degrees);
double turn_left(oi_t *sensor_data,double degrees);
//double move_forward_obstacle(oi_t *sensor_data, double distance_mm);
double move_backwards (oi_t *sensor_data, double distance_mm);
int move_forward_mission(oi_t *sensor_data, double distance_mm);



#endif /* MOVEMENT_H_ */
