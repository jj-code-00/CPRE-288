///**
// * @file lab9_template.c
// * @author
// * Template file for CprE 288 Lab 9
// */
//
//#include "Timer.h"
//#include "lcd.h"
//#include "ping_template.h"
//#include <stdio.h>
//#include "servo.h"
//
//// Uncomment or add any include directives that are needed
//
//#define REPLACEME 0
//
//
//
//int main(void) {
//	timer_init(); // Must be called before lcd_init(), which uses timer functions
//	lcd_init();
//
//	servo_init();
//	servo_move(45);
//	timer_waitMillis(500);
//	servo_move(90);
//    timer_waitMillis(500);
//
//	servo_move(150);
//
//
//
////	ping_init();
//
////	while (1)
////    {
////
//////
//////        float pingDistance = ping_getDistance();
//////        char printPulseWidth[9];
//////        char printNumMilliseconds[9];
//////        char printPing[20];
//////        char printNumOverflows[4];
//////        sprintf(printPulseWidth,"%lu",pulseWidth);
//////        sprintf(printNumMilliseconds,"%f",numMilliseconds);
//////        sprintf(printPing, "%f", pingDistance);
//////        sprintf(printNumOverflows,"%d",numOverflows);
//////
//////        lcd_printf("Pw: %s\n%s ms\n%s cm\n%s overflows\n",printPulseWidth,printNumMilliseconds,printPing,printNumOverflows);
//////
//////        timer_waitMillis(5000);
////    }
//
//}
