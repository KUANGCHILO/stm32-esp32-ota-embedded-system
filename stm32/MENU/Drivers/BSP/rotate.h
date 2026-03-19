#ifndef _ROTATE_H
#define _ROTATE_H

#include "mpu6050.h"
#include "oled.h"
#include "u8g2.h"
#include "math.h"
#include "usart.h"
#include "stdio.h"

extern u8g2_t u8g2;
extern UART_HandleTypeDef huart2;

void rotate_and_project(float roll, float pitch, float yaw);
void drawCube(float roll, float pitch, float yaw);

#endif /* _ROTATE_H */