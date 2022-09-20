///**
// * lab6_template.c
// *
// * Template file for CprE 288 Lab 6
// *
// * @author Diane Rover, 2/15/2020
// *
// */
//
//#include "Timer.h"
//#include "lcd.h"
//#include "cyBot_Scan.h"  // For scan sensors
//#include "uart.h"
//#include "open_interface.h"
//
//// Uncomment or add any include directives that are needed
//// #include "open_interface.h"
//// #include "movement.h"
//// #include "button.h"
//
//
////#warning "Possible unimplemented functions"
//#define REPLACEME 0
//
//
//int main(void) {
//
//    oi_t *sensor_data = oi_alloc();
//    oi_init(sensor_data);
//
//	timer_init(); // Must be called before lcd_init(), which uses timer functions
//	lcd_init();
//	uart_init();
//	cyBOT_init_Scan(0b0111);
//
//	cyBOT_Scan_t scanData;
//	char sentChar = ' ';
//
//	// YOUR CODE HERE
//
//    while (1)
//    {
//        sentChar = uart_receive();
//        if (sentChar == 'g')
//        {
//            int i;
//            for (i = 2; i <= 180; i+= 2)
//            {
//                sentChar = uart_receive();
//                if (sentChar == 's')
//                {
//                    break;
//                }
//                else
//                {
//                    cyBOT_Scan(i, &scanData);
//                }
//
//            }
//        }
//
//        // YOUR CODE HERE
//
//    }
//
//}
