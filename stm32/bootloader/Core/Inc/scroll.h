#ifndef _SCROLL_H
#define _SCROLL_H

#define MAX_LINES      20
#define VISIBLE_LINES   8
#define LINE_HEIGHT     8
#define MAX_LINE_LEN   22   // 128px / 約6px per char

#include <stdint.h>
#include <string.h>
#include "u8g2.h"

void log_print(const char *str);
void log_display(u8g2_t *u8g2);

#endif