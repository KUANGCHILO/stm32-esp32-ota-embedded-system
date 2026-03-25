#include "bootloader.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/_intsup.h>

uint8_t firmware_buffer[data_size];
FirmwareHeader_t header;
uint32_t crc;

void jump_app(void){
    //確認address是否為msp 也就是是否在sram地址範圍中
    uint32_t sp = *(uint32_t*)APP_ADDRESS;
    if (!(sp >RAM_START && sp <=RAM_END_RESERVED)) {
        uint32_t backup_sp = *(uint32_t*)SECTOR6_ADDRESS;
        log_print("App Msp Missed");
        log_display(&u8g2);
        if (backup_sp > 0x20000000U && backup_sp < 0x2000FFFFU){
            //載入備份
            log_print("Prepared reading backup");
            log_display(&u8g2);
            Get_Backup();
        }else {
            log_print("Backup Msp Missed");
            log_print("ERROR");        
            log_display(&u8g2);            
            Error_Handler();
        }
    }
        log_print("Jump to app");
        log_display(&u8g2);
        // 強制 reset I2C1 硬體
        HAL_DeInit();
        HAL_RCC_DeInit();
        
        //關閉cpu回應中斷
        __disable_irq();
        //關閉systick
        SysTick->CTRL=0;
        SysTick->LOAD=0;
        SysTick->VAL=0;
        //清除中斷
        for (uint32_t i = 0; i < 8; i++) {
            NVIC->ICER[i] = 0xFFFFFFFF;  // 把所有中斷的啟用狀態清掉
            NVIC->ICPR[i] = 0xFFFFFFFF;  // 把所有 pending 旗標清掉
        }

        //設置中斷向量表
        SCB->VTOR=APP_ADDRESS;
        //獲取app的reset_handler
        pFunction app_reset_handler = (pFunction)(*(uint32_t*)(APP_ADDRESS+4));
        //清除check_number
        *(uint8_t*)(CHECK_SRAM_ADDRESS) = 0x00;
        //啟用中斷
        
        //

        __set_MSP(*(uint32_t*)APP_ADDRESS);
        __DSB();
        __ISB();
        __enable_irq();
        //跳轉到app
        app_reset_handler();
}

////
//// 擦除sector3~5
void Flash_Erase(void){
    log_print("Preparing to erase");        
    log_display(&u8g2); 
    HAL_FLASH_Unlock();
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error;
    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS; //設定為按sector擦除
    erase_init.Sector = FLASH_SECTOR_3; //從sector開始擦
    erase_init.NbSectors = 3; //擦幾個sector
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3; //3.3v供電

    HAL_FLASHEx_Erase(&erase_init,  &sector_error);
        if (sector_error != 0xFFFFFFFF) {
        // sector_error 會回傳出錯的 sector 編號
        // 0xFFFFFFFF 代表全部成功
        log_print("Erasure failed");        
        log_display(&u8g2);
        Error_Handler();
    }
    HAL_FLASH_Lock();
    log_print("Erasing successful");        
    log_display(&u8g2);
}

////
//// 寫入sector
void Flash_Writeapp(uint32_t address,uint8_t* data,uint32_t length){
    HAL_FLASH_Unlock();

    uint32_t *p = (uint32_t*) data;
    uint32_t word_count = length / 4 ;

    for (uint32_t i = 0; i < word_count; i++) {
        //寫入 1.每次寫32bit(4bytes) 2.地址 3.資料
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, APP_ADDRESS + i*4, p[i]);
    }

    HAL_FLASH_Lock();
}
uint8_t Flash_Verify(uint32_t address,uint8_t* data,uint32_t length){
    for (uint32_t i = 0; i < length; i++){
        if(*(uint8_t*)(address+i) != data[i]){
            return 0;//驗證失敗
        }
    }
    return 1;//驗證成功
}

uint8_t Updata_Check(){
    //if(*(uint8_t*)(CHECK_SRAM_ADDRESS)!=check_number){return UPDATE_NOT_NEEDED;}
    uint8_t cmd[cmd_size]= {0};
    cmd[0] = cmd_check;
    uint8_t message[check_version_size];
    HAL_StatusTypeDef status;
    CS_LOW();
    status = HAL_SPI_Transmit(&hspi1, cmd, cmd_size, 500);
    CS_HIGH();
    if (status == HAL_TIMEOUT ) {
        return UPDATE_Connect_Fail;   
    }
    if (status != HAL_OK) {
        return UPDATE_NOT_NEEDED;   
    }
    HAL_Delay(2000);
    CS_LOW();
    status = HAL_SPI_Receive(&hspi1, message, check_version_size, HAL_MAX_DELAY);
    CS_HIGH();
    if (status == HAL_TIMEOUT ) {
        return UPDATE_Connect_Fail;   
    }
    if (status != HAL_OK) {
        return UPDATE_NOT_NEEDED;   
    }
    if (message[0]==request_update_success) {
        return UPDATE_REQUESTED;
    }
    return UPDATE_NOT_NEEDED;
}

//不用的
uint8_t* Get_Update_Program(uint32_t *out_size){
    uint8_t cmd[cmd_size]= {0};;
    cmd[0] = cmd_check;

    HAL_SPI_Transmit(&hspi1, cmd, cmd_size,500);

    //先取得檔案大小
    
    HAL_SPI_Receive(&hspi1, (uint8_t*)&header, sizeof(FirmwareHeader_t), HAL_MAX_DELAY);

    //檢查收到的header是否合理
    if (header.firmware_size == 0 || header.firmware_size > MAX_FIRMWARE_SIZE) {
        return NULL;
    }
    HAL_SPI_Receive(&hspi1,firmware_buffer,header.firmware_size,HAL_MAX_DELAY);
    *out_size = header.firmware_size;

    return firmware_buffer;
}
////
//// 擦除並取得備份區 sector 6~7
void Get_Backup(){
    Flash_Erase();
    log_print("restore from backup");        
    log_display(&u8g2);
    HAL_FLASH_Unlock();
    int once_write_size=4096;
    uint8_t this_chunk[once_write_size];
    for (uint32_t i = 0; i < 256*1024;i+=once_write_size) {
        char log_message[40];
        float progress = (float)i / (256.0f * 1024.0f) * 100.0f;
        sprintf(log_message,"Restore progress(%.f%%/100%%)",progress);
        log_print(log_message);        
        log_display(&u8g2);
        memcpy(this_chunk, (uint8_t*)SECTOR6_ADDRESS+i, once_write_size);
        uint32_t *p = (uint32_t*)this_chunk;
        for (uint32_t k = 0;k<once_write_size/4; k++) {
            HAL_StatusTypeDef ret = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_WORD, APP_ADDRESS + i + k*4, p[k]);
    
            if (ret != HAL_OK) {
                char err[40];
                sprintf(err, "ERR i=%lu k=%u SR=%08lX", i, k, FLASH->SR);
                log_print(err);
                log_display(&u8g2);
                HAL_FLASH_Lock();
                return;  // 立刻停下來看 log
            }
        }
    }
    HAL_FLASH_Lock();
    log_print("Restore successful");        
    log_display(&u8g2);
}

uint8_t Get_Update_Program_Chunk(uint32_t *out_size){
    uint8_t cmd[cmd_size]= {0};;
    cmd[0] = cmd_request_update;
    crc = 0xFFFFFFFF;

    // CS_LOW();
    // HAL_SPI_TransmitReceive(&hspi1,&cmd,(uint8_t*)&header,sizeof(FirmwareHeader_t),HAL_MAX_DELAY);
    // CS_HIGH();
    CS_LOW();
    HAL_SPI_Transmit(&hspi1, cmd, cmd_size,500);
    CS_HIGH();
    //先取得檔案大小
    HAL_Delay(10000);//下載時間
    CS_LOW();
    HAL_SPI_Receive(&hspi1, (uint8_t*)&header, sizeof(FirmwareHeader_t), HAL_MAX_DELAY);
    CS_HIGH();
    //檢查收到的header是否合理
    if (header.firmware_size == 0 || header.firmware_size > MAX_FIRMWARE_SIZE) {
        return 0;
    }
    Flash_Erase();
    HAL_FLASH_Unlock();

    uint32_t received = 0;
    uint32_t write_address = APP_ADDRESS;
    uint8_t cmd_chunk[cmd_size]={0};
    while (received<header.firmware_size) {
        uint32_t this_chunk = ((header.firmware_size-received) < CHUNK_SIZE)? (header.firmware_size-received):CHUNK_SIZE;
        this_chunk+=CRC32_BYTES;

        cmd_chunk[0] = cmd_get_update;
        cmd_chunk[1] = this_chunk >> 24;
        cmd_chunk[2] = this_chunk >> 16;
        cmd_chunk[3] = this_chunk >> 8;
        cmd_chunk[4] = this_chunk >> 0;
        cmd_chunk[5] = received >> 24;
        cmd_chunk[6] = received >> 16;
        cmd_chunk[7] = received >> 8;
        cmd_chunk[8] = received >> 0;
        CS_LOW();
        HAL_SPI_Transmit(&hspi1, cmd_chunk, cmd_size,500);
        CS_HIGH();
        HAL_Delay(1000);
        CS_LOW();
        HAL_SPI_Receive(&hspi1, firmware_buffer, data_size, HAL_MAX_DELAY);
        CS_HIGH();

        uint32_t crc_chunk = CRC32_Software_Chunk(firmware_buffer,this_chunk-CRC32_BYTES);
        uint32_t crc_received = (firmware_buffer[this_chunk-4] << 24) | (firmware_buffer[this_chunk-3] << 16) | (firmware_buffer[this_chunk-2] << 8) | (firmware_buffer[this_chunk-1] << 0);
        if (crc_chunk != crc_received) {
            continue;
        }

        char log_message[50];
        float progress = (float)received/(float)header.firmware_size*100;
        sprintf(log_message,"Updata progress(%.f%%/100%%)",progress);
        log_print(log_message);    
        log_display(&u8g2);

        CRC32_Software(firmware_buffer,this_chunk-CRC32_BYTES);

        uint32_t *p = (uint32_t*) firmware_buffer;
        uint32_t word_count = (this_chunk-CRC32_BYTES) / 4 ; //無條件進位
        
        for (uint32_t i = 0; i < word_count; i++) {
            //寫入 1.每次寫32bit(4bytes) 2.地址 3.資料
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_address, p[i]);
            write_address+=4;
        }
        uint32_t flash_msp   = *(uint32_t*)APP_ADDRESS;
        uint32_t flash_reset = *(uint32_t*)(APP_ADDRESS + 4);

        received+=this_chunk-CRC32_BYTES;
        memset(firmware_buffer, 0, sizeof(firmware_buffer));//無法跳轉原因
    }
    uint32_t flash_msp0   = *(uint32_t*)APP_ADDRESS;
    uint32_t flash_reset0 = *(uint32_t*)(APP_ADDRESS + 4);
    *out_size = header.firmware_size;
    crc = crc ^ 0xFFFFFFFF;
    HAL_FLASH_Lock();
    return 1;
}

void CRC32_Software(uint8_t *data,uint32_t length){
    for (uint32_t i =0; i<length; i++) {
        crc ^=data[i];
        for (uint8_t j=0; j<8; j++) {
            if(crc&1) crc = (crc >> 1) ^ 0xEDB88320;
            else crc >>=1;
        
        }
    }
    
}

uint32_t CRC32_Software_Chunk(uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xEDB88320;
            else         crc >>= 1;
        }
    }
    return crc ^ 0xFFFFFFFF;
}