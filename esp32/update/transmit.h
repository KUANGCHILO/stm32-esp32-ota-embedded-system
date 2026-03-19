#ifndef _TRANSMIT_H
#define _TRANSMIT_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>
#include <WiFiClientSecure.h>

#define cmd_check_version (0x01) //檢查更新
#define cmd_download_update (0x02) //下載更新檔案
#define cmd_transmit_update (0x03) //傳送資料指令後送this_chunk開始到CHUNK_SIZE-1(512bytes)處檔案

#define UPDATE_NOT_NEEDED  0
#define UPDATE_REQUESTED   1
#define UPDATE_Connect_Fail   2

// #define WiFi_ssid "OPPO A72"
// #define WiFi_password "99999999"

#define WiFi_ssid "250-2N6F-2.4G"
#define WiFi_password "0911960009"

typedef struct {
    uint32_t firmware_size;   // ESP32 告訴 STM32：我要傳多少 bytes
    uint32_t crc;             // 整包的 CRC，用來最後驗證
} FirmwareHeader_t;
uint8_t Check_Version();
FirmwareHeader_t Download_Update();
void Transmit_Update(uint32_t this_chunk,uint32_t received,uint8_t *data);



#endif