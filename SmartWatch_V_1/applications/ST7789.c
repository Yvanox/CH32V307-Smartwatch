/*********************************************** *******************************
* File Name          : ST7789.c
* Author             : ElHombreSoqo
* Version            : V1.0.0
* Date               : 07 Jun 2022
* Description        : Init routine code and custom functions for a ST7789 screen
*
* Special thanks to the "Controllers Tech" Youtube channel (helped me a lot with
* the development of these drivers)
*******************************************************************************/

#include "ST7789.h"
LCD_ATTRIBUTES LCD_ST7789;

void lcd_SendBytes(uint8_t _dat)
{
    rt_thread_mdelay(1);
    SPI_SendData(SPI1, (uint8_t *)&_dat, 1);
	rt_thread_mdelay(1);
}

void LCD_command_write(uint8_t command)
{ //  Write orders
    LCD_DC_LOW();
    LCD_CS_LOW();
    lcd_SendBytes(command);
    LCD_CS_HIGH();
}
void LCD_data_write(uint8_t data)
{ // Writing data
    LCD_DC_HIGH();
    LCD_CS_LOW();
    lcd_SendBytes(data);
    LCD_CS_HIGH();
}

void WriteData(uint8_t* buff, size_t buff_size)
{
    LCD_DC_HIGH();
    SPI_SendData(SPI1, buff, buff_size);
}

void LCD_SetAttributes(uint8_t Scan_dir)
{
	//Get the screen scan direction
		LCD_ST7789.SCAN_DIR = Scan_dir;
		uint8_t MemoryAccessReg = 0x00;

	    //Get GRAM and LCD width and height
	    if(Scan_dir == HORIZONTAL) {
	    	LCD_ST7789.HEIGHT	= ST7789_HEIGHT;
	    	LCD_ST7789.WIDTH   = ST7789_WIDTH;
	        MemoryAccessReg = 0X60;
	    } else {
	    	LCD_ST7789.HEIGHT	= ST7789_WIDTH;
	    	LCD_ST7789.WIDTH   = ST7789_HEIGHT;
	        MemoryAccessReg = 0X00;
	    }

	    // Set the read / write scan direction of the frame memory
	    LCD_command_write(0x36);            //MX, MY, RGB mode
	    LCD_data_write(MemoryAccessReg);	//Set RGB
}

void LCD_InitReg(void)
{

    rt_thread_mdelay(5);
	LCD_REST_LOW()
    rt_thread_mdelay(10);
	LCD_REST_HIGH()
	rt_thread_mdelay(120);

    LCD_command_write(0x3A); //  0x3A    // Interface Pixel Format
    LCD_data_write(0x05);                // 16bit/pixel

    /*********** Frame Rate Setting ***********/
    LCD_command_write(0xB2); //0xB2     // Porch Setting
    LCD_data_write(0x0C);               // Back porch in normal mode
    LCD_data_write(0x0C);               // Front porch in normal mode
    LCD_data_write(0x00);               // Disable separate porch control
    LCD_data_write(0x33);               // Back and front porch in idle mode
    LCD_data_write(0x33);               // Back and front porch in partial mode

    LCD_command_write(0xB7); //  0xB7   // Gate Control
    //LCD_data_write(0x72);             // VGH:13.26, VGL:-10.43
    LCD_data_write(0x35);

    LCD_command_write(0xBB); // 0xBB     // VCOM Setting
    //LCD_data_write(0x3d);             // VCOM: 1.425
    LCD_data_write(0x32);

    LCD_command_write(0xC2); // 0xC2     // LCM Control
    //LCD_data_write(0x2c);
    LCD_data_write(0x01);

    /* VRH Set */
    LCD_command_write(0xC3); //0xC3     // VDV and VRH Command Enable
    LCD_data_write(0x15);
    //LCD_data_write(0xFF);

    /* VDV Set */
    LCD_command_write(0xC4);
    LCD_data_write(0x20);

    LCD_command_write(0xC6); //  0xC6   // Frame Rate Control in Normal Mode
    LCD_data_write(0x0F);               // 60Hz

    LCD_command_write(0xD0); // 0xD0    // Power Control 1
    LCD_data_write(0xA4);
    LCD_data_write(0xA1);               // AVDD:6.8V, AVCL:-4.8V, VDDS:2.3V

    /* Positive Voltage Gamma Control */
    LCD_command_write(0xE0);
    LCD_data_write(0xD0);
    LCD_data_write(0x08);
    LCD_data_write(0x0E);
    LCD_data_write(0x09);
    LCD_data_write(0x09);
    LCD_data_write(0x05);
    LCD_data_write(0x31);
    LCD_data_write(0x33);
    LCD_data_write(0x48);
    LCD_data_write(0x17);
    LCD_data_write(0x14);
    LCD_data_write(0x15);
    LCD_data_write(0x31);
    LCD_data_write(0x34);

    /* Negative Voltage Gamma Control */
    LCD_command_write(0xE1);
    LCD_data_write(0xD0);
    LCD_data_write(0x08);
    LCD_data_write(0x0E);
    LCD_data_write(0x09);
    LCD_data_write(0x09);
    LCD_data_write(0x15);
    LCD_data_write(0x31);
    LCD_data_write(0x33);
    LCD_data_write(0x48);
    LCD_data_write(0x17);
    LCD_data_write(0x14);
    LCD_data_write(0x15);
    LCD_data_write(0x31);
    LCD_data_write(0x34);

    LCD_command_write( 0x21);           // Display Inversion On
    LCD_command_write( 0x11);

    //LCD_command_write( 0x13);

    rt_thread_mdelay(120);

    LCD_command_write(0x29); ///< Display on
}

void LCD_Init(uint8_t Scan_dir)
{

    //Set the resolution and scanning method of the screen
	LCD_SetAttributes(Scan_dir);

    //Set the initialization register
	LCD_InitReg();
}

void LCD_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend)
{
    //Set X and Y
	Ystart+=20; Yend+=20;

	LCD_command_write(0x2a);
	LCD_data_write(Xstart>>8);
	LCD_data_write(Xstart);
	LCD_data_write(Xend>>8);
	LCD_data_write(Xend);

	LCD_command_write(0x2b);
	LCD_data_write(Ystart>>8);
	LCD_data_write(Ystart);
	LCD_data_write(Yend>>8);
	LCD_data_write(Yend);

	LCD_command_write(0x2c);        //LCD_WriteCMD(GRAMWR);
}

void FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT)) return;
        if((x + w - 1) >= ST7789_WIDTH) w = ST7789_WIDTH - x;
        if((y + h - 1) >= ST7789_HEIGHT) h = ST7789_HEIGHT - y;

        LCD_SetWindows(x, y, x+w-1, y+h-1);
        LCD_CS_LOW();

        uint8_t data[] = { color >> 8, color & 0xFF };

        LCD_DC_HIGH();
        for(y = h; y > 0; y--) {
            for(x = w; x > 0; x--) {
                SPI_SendData(SPI1, data, sizeof(data));
            }
        }

        LCD_CS_HIGH();
        LCD_DC_LOW();
}

void DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= ST7789_WIDTH) || (y >= ST7789_HEIGHT))
        return;

    LCD_SetWindows(x, y, x, y);
    LCD_CS_LOW();

    uint8_t data[] = { color >> 8, color & 0xFF };
    rt_thread_mdelay(1);
    WriteData(data, sizeof(data));
    rt_thread_mdelay(1);

    LCD_CS_HIGH();
}

void WriteChar(uint16_t x, uint16_t y, char ch, FontDef font, uint16_t color, uint16_t bgcolor) {
    uint32_t i, b, j;

    LCD_SetWindows(x, y, x+font.width-1, y+font.height-1);
    LCD_CS_LOW();

    for(i = 0; i < font.height; i++) {
        b = font.data[(ch - 32) * font.height + i];
        for(j = 0; j < font.width; j++) {
            if((b << j) & 0x8000)  {
                uint8_t data[] = { color >> 8, color & 0xFF };
                WriteData(data, sizeof(data));
            } else {
                uint8_t data[] = { bgcolor >> 8, bgcolor & 0xFF };
                WriteData(data, sizeof(data));

            }
        }
    }
}

void WriteString(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {

    LCD_CS_LOW();

    while(*str) {
        if(x + font.width >= ST7789_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ST7789_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                str++;
                continue;
            }
        }

        if(*str == '\n')
        {
            str++;
            continue;
        }

        WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    LCD_CS_HIGH();
}

void RTC_ST7789_Time(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {

    LCD_CS_LOW();

    str = str + 11; //Remove date;

    while(*str) {
        if(x + font.width >= ST7789_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ST7789_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        if(*str == '\n')
        {
            str++;
            continue;
        }

        /* This removes year */
        if(*str == ' ')
        {
            break;
        }

        WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    LCD_CS_HIGH();
}

void RTC_ST7789_Date(uint16_t x, uint16_t y, const char* str, FontDef font, uint16_t color, uint16_t bgcolor) {

    LCD_CS_LOW();

    int i = 0; //Variable to count the amount of spaces (" ") on date

    while(*str) {
        if(x + font.width >= ST7789_WIDTH) {
            x = 0;
            y += font.height;
            if(y + font.height >= ST7789_HEIGHT) {
                break;
            }

            if(*str == ' ') {
                // skip spaces in the beginning of the new line
                str++;
                continue;
            }
        }

        if(*str == '\n')
        {
            str++;
            continue;
        }

        /* This removes year (Using RT-Thread soft RTC) */
        if(*str == ' ')
        {
            i++;
            if(i >= 4){
                i = 0;
                break;
            }
        }

        WriteChar(x, y, *str, font, color, bgcolor);
        x += font.width;
        str++;
    }

    LCD_CS_HIGH();
}

