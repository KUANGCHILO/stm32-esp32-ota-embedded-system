#ifndef INC_HTU21D_H
#define INC_HTU21D_H

#include "i2c.h"
void HTU21D_Init(void);
uint8_t HTU21D_ReadUserReg(void);
float HTU21D_ReadTemperature(void);
float HTU21D_ReadHumidity(void);
#endif  /* HTU21D.h */