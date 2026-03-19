#ifndef MENU_RENDER_H_
#define MENU_RENDER_H_

#include "menu.h"
#include "menu_engine.h"
#include <stdint.h>
#include <string.h>
#include "HTU21D.h"
#include "rotate.h"
#include "assets/images.h"

//char weeks[7][10] = {"","Sunday","Monday","Thesday","Wednesday","Thursday","Friday","Saturday"};

extern RTC_TimeTypeDef tempTime;
extern RTC_DateTypeDef tempDate;
extern uint8_t* image_capoo[21];

void menu_render(MenuContext* ctx);
void screen_temperature();
void screen_clock();
void screen_servo();
void screen_mpu6050_kalman();
void screen_mpu6050_duplexer();
void screen_gif_capoo();

#endif
