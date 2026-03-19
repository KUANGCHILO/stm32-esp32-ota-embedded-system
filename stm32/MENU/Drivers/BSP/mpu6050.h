#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_

#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "math.h"
#include "stm32f4xx_hal_i2c.h"
#include <stdint.h>

//// MPU6050 I2C address and register definitions
#define MPU6050_ADDR (0x68 << 1) // I2C address of the MPU6050
#define PWR_MGMT_1_REG 0x6B // Power management register
#define ACCEL_CONFIG_REG 0x1C // Accelerometer configuration register
#define GYRO_CONFIG_REG 0x1B // 陀螺儀配置寄存器

#define ACCEL_XOUT_H_REG 0x3B // 加速度計X軸高位寄存器
#define ACCEL_XOUT_L_REG 0x3C // 加速度計X軸低位寄存器
#define ACCEL_YOUT_H_REG 0x3D // 加速度計Y軸高位寄存器
#define ACCEL_YOUT_L_REG 0x3E // 加速度計Y軸低位寄存器
#define ACCEL_ZOUT_H_REG 0x3F // 加速度計Z軸高位寄存器
#define ACCEL_ZOUT_L_REG 0x40 // 加速度計Z軸低位寄存器

#define GYRO_XOUT_H_REG 0x43 // 陀螺儀X軸高位寄存器
#define GYRO_XOUT_L_REG 0x44 // 陀螺儀X軸低位寄存器
#define GYRO_YOUT_H_REG 0x45 // 陀螺儀Y軸高位寄存器
#define GYRO_YOUT_L_REG 0x46 // 陀螺儀Y軸低位寄存器
#define GYRO_ZOUT_H_REG 0x47 // 陀螺儀Z軸高位寄存器
#define GYRO_ZOUT_L_REG 0x48 // 陀螺儀Z軸低位寄存器

//// CMD definitions
#define DEVICE_RESET 0X80 //復位
#define DEVICE_WAKEUP 0X00 //喚醒

#define ACCEL_2G 0x00 //加速度計量程±2g

#define GYRO_250DPS 0x00 //陀螺儀量程±250°/s
//// Function prototypes
void MPU6050_Init(I2C_HandleTypeDef *hi2c);
void MPU6050_Read_All(I2C_HandleTypeDef *hi2c);
void MPU6050_Calibrate_Gyro(I2C_HandleTypeDef *hi2c);
// void MPU6050_Update(I2C_HandleTypeDef *hi2c);
// float MPU6050_Get_Accel_X(void);
// float MPU6050_Get_Accel_Y(void);
// float MPU6050_Get_Accel_Z(void);
// float MPU6050_Get_Gyro_X(void);
// float MPU6050_Get_Gyro_Y(void);
// float MPU6050_Get_Gyro_Z(void);
float MPU6050_Get_Roll_By_Accel(void);
float MPU6050_Get_Pitch_By_Accel(void);
float MPU6050_Get_Yaw_By_Accel(void);
float MPU6050_Get_Roll_By_Gyro(void);
float MPU6050_Get_Pitch_By_Gyro(void);
float MPU6050_Get_Yaw_By_Gyro(void);
void MPU6050_Update_Euler_Angles(void);
void MPU6050_Complementary_Filter(void);
float MPU6050_Get_Roll_Bal(void);
float MPU6050_Get_Pitch_Bal(void);
float MPU6050_Get_Yaw_Bal(void);
void MPU6050_Kalman_Filter(void);
float MPU6050_Get_Roll_Kalman(void);
float MPU6050_Get_Pitch_Kalman(void);
float MPU6050_Get_Yaw_Kalman(void);

#endif /* INC_MPU6050_H_ */