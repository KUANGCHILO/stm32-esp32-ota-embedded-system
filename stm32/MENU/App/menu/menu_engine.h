#ifndef MENU_ENGINE_H_
#define MENU_ENGINE_H_

#include "menu.h"
#include "menu_def.h"
#include "rtc.h"

extern RTC_TimeTypeDef tempTime;
extern RTC_DateTypeDef tempDate;

void menu_handle_event(MenuContext* ctx, ButtonEvent evt);
void clock_handle_event(MenuContext* ctx, ButtonEvent evt);
void engine_handle_event(MenuContext* ctx, ButtonEvent evt);
void servo_handle_event(MenuContext* ctx, ButtonEvent evt);

#endif