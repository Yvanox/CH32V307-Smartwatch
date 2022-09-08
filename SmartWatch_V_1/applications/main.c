/*********************************************** *******************************
* File Name          : main.c
* Author             : ElHombreSoqo
* Version            : V1.1.0
* Date               : 07 Sep 2022
* Description        : Main program and initialization.
*******************************************************************************/
#include "ch32v30x.h"
#include <rtthread.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>

/* Custom libraries */
#include "ST7789.h"     //Screen
#include "MPU6050.h"    //Accelerometer

#include "watch.h"

/*********************************************************************
 * @fn      main
 *
 * @brief   Main program.
 *
 * @return  none
 */

int main(void)
{

    SPI_7789_INIT();                                //SPI initialization (NOTE: this uses a custom SPIsend API, check ch32v30x_spi.h/.c)
    LCD_Init(VERTICAL);                             //Screen initialization and orientation
    MPU_Init();                                     //Accelerometer initialization
    rtc_sample();                                   //RTC sample function from RT-thread document center
    ADC_Batt_Init();                                //ADC Initialization

    rt_thread_mdelay(5);
    FillRectangle(0, 0, 240, 280, LCD_DISP_BLACK);  //Fill screen with black color

    Main_Threads();                                 //This is where all the threads are

}
