#include "HTU21D.h"
#include "cmsis_os2.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_i2c.h"
#include <stdint.h>

#define HTU21D_ADDR (0x40 << 1)

void HTU21D_Init(void)
{
    uint8_t cmd = 0xFE;
    HAL_I2C_Master_Transmit(&hi2c1, HTU21D_ADDR, &cmd, 1, 100);
    osDelay(20);
}

uint8_t HTU21D_ReadUserReg(void)
{
    uint8_t cmd = 0xE7;
    uint8_t readBuffer = 0x00; // 初始化為 0
    HAL_StatusTypeDef status;

    // 1. 發送讀取暫存器的指令
    status = HAL_I2C_Master_Transmit(&hi2c1, HTU21D_ADDR, &cmd, 1, 100);
    if (status != HAL_OK) return 0xEE; // 傳送失敗回傳錯誤碼 0xEE

    osDelay(10); 

    // 2. 讀取 1 byte 數據
    status = HAL_I2C_Master_Receive(&hi2c1, HTU21D_ADDR, &readBuffer, 1, 100);
    if (status != HAL_OK) return 0xFF; // 接收失敗回傳錯誤碼 0xFF

    return readBuffer;
}

float HTU21D_ReadTemperature(void)
{
    uint8_t cmd = 0xE3;
    uint8_t readBuffer[3];
    HAL_I2C_Master_Transmit(&hi2c1, HTU21D_ADDR, &cmd, 1, 100);
    osDelay(50);
    HAL_I2C_Master_Receive(&hi2c1, HTU21D_ADDR, readBuffer, 3, 100);
    
    uint16_t rawTemp = (readBuffer[0] << 8) | readBuffer[1];
    rawTemp &= ~(0x0003); // Clear status bits
    return -46.85 + (175.72 * rawTemp / 65536.0);
}

float HTU21D_ReadHumidity(void)
{
    uint8_t cmd = 0xE5;
    uint8_t readBuffer[3];
    HAL_I2C_Master_Transmit(&hi2c1, HTU21D_ADDR, &cmd, 1, 100);
    osDelay(50);
    HAL_I2C_Master_Receive(&hi2c1, HTU21D_ADDR, readBuffer, 3, 100);
    
    uint16_t rawHum = (readBuffer[0] << 8) | readBuffer[1];
    rawHum &= ~(0x0003); // Clear status bits
    return -6.0 + (125.0 * rawHum / 65536.0);
}