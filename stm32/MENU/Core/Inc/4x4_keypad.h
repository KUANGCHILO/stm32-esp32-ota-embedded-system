#ifndef INC_4x4_KEYPAD_H
#define INC_4x4_KEYPAD_H

#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "main.h"

#define ALL_ROWS (key_R1_Pin|key_R2_Pin|key_R3_Pin|key_R4_Pin)

char Keypad_GetKey(void);
char Keypad_GetKey_IT(uint16_t GPIO_Pin);
extern volatile char keypad_key_pressed;  // 中斷檢測到的按鍵
#endif /* INC_4x4_KEYPAD_H */