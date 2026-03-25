#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include <stdint.h>
#include "main.h"
#include "spi.h"
#include "stm32f401xe.h"
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include "stm32f4xx_hal_spi.h"
#include <stdint.h>
#include <string.h>
#include "scroll.h"

#define APP_ADDRESS (0x0800C000U)
#define SECTOR2_ADDRESS (0x08008000)
#define SECTOR6_ADDRESS (0x08040000)
#define CHECK_SRAM_ADDRESS (0x20017FF8U) //sram範圍 0x20000000 ~ 0x20017FFF 取最後8 bytes

#define RAM_START         0x20000000U
#define RAM_END_RESERVED  0x20017FF8U

#define UPDATE_NOT_NEEDED  0
#define UPDATE_REQUESTED   1
#define UPDATE_Connect_Fail   2

#define cmd_check (0x01)
#define cmd_request_update (0x02)
#define cmd_get_update (0x03)
#define ARK (0x69)

#define request_update_success (0x01)
#define request_connect_Fail (0x02)
#define request_updata_not_needed (0x00)
#define check_number (0x06)

#define MAX_FIRMWARE_SIZE  (256 * 1024) //收到bin最大256kb
#define CHUNK_SIZE  (4080-4) //一次只收2048bytes
#define CRC32_BYTES (4)

#define cmd_size 12
#define check_version_size 4
#define data_size (CHUNK_SIZE+CRC32_BYTES)

#define CS_LOW()  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;
typedef void (*pFunction)(void);

void jump_app(void);
void Flash_Erase(void);
void Flash_Writeapp(uint32_t address,uint8_t* data,uint32_t length);
uint8_t Flash_Verify(uint32_t address,uint8_t* data,uint32_t length);
uint8_t Updata_Check(void);
uint8_t* Get_Update_Program(uint32_t *out_size);
void Get_Backup();
uint8_t Get_Update_Program_Chunk(uint32_t *out_size);
void CRC32_Software(uint8_t *data,uint32_t length);
uint32_t CRC32_Software_Chunk(uint8_t *data, uint32_t length);

typedef struct {
    uint32_t firmware_size;   // ESP32 告訴 STM32：我要傳多少 bytes
    uint32_t crc;             // 整包的 CRC，用來最後驗證
} FirmwareHeader_t;

extern uint8_t firmware_buffer[data_size];
extern FirmwareHeader_t header;
extern uint32_t crc;
extern u8g2_t u8g2;



#endif