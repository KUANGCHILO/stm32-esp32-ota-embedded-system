#ifndef MENU_DEF_H_
#define MENU_DEF_H_

#include "menu.h"
#include "menu_render.h"
#include "menu_engine.h"
#include "mpu6050.h"

#define BOOTLOADER_ADDRESS (0x8000000)
#define CHECK_SRAM_ADDRESS (0x20017FF8U) //sram範圍 0x20000000 ~ 0x20017FFF 取最後8 bytes
#define check_number (0x06)

typedef void (*pFunction)(void);

MenuContext GetMenuContext();
void MenuInit();
void action_clock();
void action_menu();
void change_colck(int8_t dir);
void action_temperature(void);
void change_servo(int8_t dir);
void action_servo(void);
void action_mpu6050_kalman(void);
void action_mpu6050_duplexer(void);
void action_gif(void);
void action_update(void);

extern RTC_TimeTypeDef tempTime;
extern RTC_DateTypeDef tempDate;
extern TIM_HandleTypeDef htim2;

extern int servo_degree;

#endif