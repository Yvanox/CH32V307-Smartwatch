/*********************************************** *******************************
* File Name          : watch.h
* Author             : ElHombreSoqo
* Version            : V1.0.0
* Date               : 07 Sep 2022
* Description        : Main threads and functionality
*******************************************************************************/
#ifndef APPLICATIONS_WATCH_H_
#define APPLICATIONS_WATCH_H_

#include "ch32v30x.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <rthw.h>
#include "drivers/pin.h"
#include <board.h>

/* Custom libraries */
#include "ST7789.h"
#include "MPU6050.h"

/* Button defines */
#define UP_PIN 66       //UP Button
#define SELECT_PIN 20   //SELECT Button
#define DOWN_PIN 49      //DOWN Button

#define SAMPLE_UART_NAME       "uart1"

/*  Thread defines  */

#define THREAD_PRIORITY_LOW         30
#define THREAD_PRIORITY_MED         25
#define THREAD_PRIORITY_HIGH        20
#define THREAD_PRIORITY_VERY_HIGH   5

#define THREAD_STACK_SIZE_BIG       2048
#define THREAD_STACK_SIZE           1024
#define THREAD_STACK_SIZE_NANO      128

#define THREAD_TIMESLICE            5
#define THREAD_TIMESLICE_UART       10

/* Global define */
#define UP_PIN 66 //UP Button
#define SELECT_PIN 20 //SELECT Button
#define DOWN_PIN 49 //DOWN Button

int Main_Threads();

#endif /* APPLICATIONS_WATCH_H_ */
