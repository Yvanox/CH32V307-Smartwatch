/*********************************************** *******************************
* File Name          : fonts.h
* Author             : ElHombreSoqo
* Version            : V1.0.0
* Date               : 07 Jun 2022
* Description        : Font definitions.
*******************************************************************************/

#ifndef __FONTS_H__
#define __FONTS_H__

#include <stdint.h>

typedef struct {
    const uint8_t width;
    uint8_t height;
    const uint16_t *data;
} FontDef;


extern FontDef Font_7x10;
extern FontDef Font_11x18;
extern FontDef Font_16x26;
extern FontDef Clock_font;

#endif // __FONTS_H__
