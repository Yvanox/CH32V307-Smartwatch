/*********************************************** *******************************
* File Name          : ST7789.h
* Author             : ElHombreSoqo
* Version            : V1.0.0
* Date               : 07 Jun 2022
* Description        : Macros for ST7789 screen.
*******************************************************************************/
#ifndef INC_ST7789_H_
#define INC_ST7789_H_

#include "ch32v30x.h"

#include "fonts.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Screen pixel size
#define ST7789_HEIGHT 280
#define ST7789_WIDTH  240

#define HORIZONTAL 0
#define VERTICAL   1

// Pin MACROS
#define DC_PIN      GPIO_Pin_1
#define DC_PORT     GPIOC
#define CS_PIN      GPIO_Pin_2
#define CS_PORT     GPIOC
#define RST_PIN     GPIO_Pin_0
#define RST_PORT    GPIOC

#define LCD_REST_LOW()   GPIO_WriteBit(RST_PORT, RST_PIN, Bit_RESET);
#define LCD_REST_HIGH()  GPIO_WriteBit(RST_PORT, RST_PIN, Bit_SET);

#define LCD_DC_LOW()     GPIO_WriteBit(DC_PORT, DC_PIN, Bit_RESET);
#define LCD_DC_HIGH()    GPIO_WriteBit(DC_PORT, DC_PIN, Bit_SET);

#define LCD_CS_LOW()     GPIO_WriteBit(CS_PORT, CS_PIN, Bit_RESET);
#define LCD_CS_HIGH()    GPIO_WriteBit(CS_PORT, CS_PIN, Bit_SET);

/**************  Color (RGB 5,6,5) **************/
#define LCD_DISP_RED                    0xF800
#define LCD_DISP_GREEN                  0x07E0
#define LCD_DISP_BLUE                   0x001F
#define LCD_DISP_ORANGE                 0xFC40
#define LCD_DISP_CYAN                   0x07FF
#define LCD_DISP_MAGENTA                0xF81F
#define LCD_DISP_YELLOW                 0xFFE0
#define LCD_DISP_WHITE                  0xFFFF
#define LCD_DISP_BLACK                  0x0000
#define LCD_DISP_GRAY                   0xEF5D
#define LCD_DISP_GRAY75                 0x39E7
#define LCD_DISP_GRAY50                 0x7BEF
#define LCD_DISP_GRAY25                 0xADB5




typedef struct{
	uint16_t WIDTH;
	uint16_t HEIGHT;
	uint8_t SCAN_DIR;
}LCD_ATTRIBUTES;


/*		Function prototypes		*/

void lcd_SendBytes(uint8_t _dat);

//Data related functions
void LCD_command_write(uint8_t command);
void LCD_data_write(uint8_t data);

void WriteData(uint8_t* buff, size_t buff_size);

//========================================================
//CASET (2Ah): Column Address Set
//RASET (2Bh): Row Address Set
//x1,y1: Initial address ,x2,y2: Destination address
//========================================================
void lcd_set_address(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

//Screen Initialization
void LCD_InitReg(void);
void LCD_SetAttributes(uint8_t Scan_dir);
void LCD_Init(uint8_t Scan_dir);

void LCD_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend);

void FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor);
void WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void RTC_ST7789_Time(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);
void RTC_ST7789_Date(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor);

#endif /* INC_ST7789_H_ */
