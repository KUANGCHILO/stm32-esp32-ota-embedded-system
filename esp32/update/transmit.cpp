#include <stdint.h>
#include "transmit.h"
#include <Arduino.h>

static void debugLog(const char* runId, const char* hypothesisId, const char* location, const char* message, const String& dataJson) {
    String line = String("{\"sessionId\":\"f656c2\",\"runId\":\"") + runId +
                  "\",\"hypothesisId\":\"" + hypothesisId +
                  "\",\"location\":\"" + location +
                  "\",\"message\":\"" + message +
                  "\",\"data\":" + dataJson +
                  ",\"timestamp\":" + String((unsigned long)millis()) + "}";
    Serial.println(line);
    File lf = LittleFS.open("debug-f656c2.log", "a");
    if (lf) {
        lf.println(line);
        lf.close();
    }
}

uint32_t crc = 0xFFFFFFFF;

const char* versionUrl = "https://raw.githubusercontent.com/KUANGCHILO/stm32-esp32-ota-embedded-system/refs/heads/main/stm32/MENU/version.txt";
const char* updateUrl = "https://raw.githubusercontent.com/KUANGCHILO/stm32-esp32-ota-embedded-system/refs/heads/main/stm32/MENU/build/Debug/MENU.bin";
const char* update_file_path = "/MENU.bin";
const char* version_file_path = "/version.txt";
String readVersion() {
    File f = LittleFS.open(version_file_path, "r");
    if (!f) return "unknown";
    String ver = f.readStringUntil('\n');
    f.close();
    ver.trim();  // 去掉換行符
    return ver;
}

String getRemoteVersion() {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.begin(client, versionUrl);

    String remoteVer = "unknown";
    int code = http.GET();
    // #region agent log
    debugLog("before-fix", "H3", "transmit.cpp:getRemoteVersion", "version_http_response",
        String("{\"http_code\":") + code + "}");
    // #endregion
    if (code == 200) {
        remoteVer = http.getString();
        remoteVer.trim();  // 去掉換行空白
    }
    http.end();
    return remoteVer;
}

uint8_t Check_Version()
{
    String remote = getRemoteVersion();
    if (remote== "unknown") {
        return request_connect_Fail;
    }
    if (remote != readVersion()) {
        Serial.println("有新版本！");
        return request_update_success;
    } else {
        return request_updata_not_needed;
    }
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

FirmwareHeader_t Download_Update(){
    crc = 0xFFFFFFFF;
    FirmwareHeader_t result= {0};
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    
    http.begin(client,updateUrl);
    //http.setInsecure();  // 跳過 SSL 驗證
    
    int httpCode = http.GET();
    // #region agent log
    debugLog("before-fix", "H3", "transmit.cpp:Download_Update", "firmware_http_response",
        String("{\"http_code\":") + httpCode + ",\"content_length\":" + http.getSize() + "}");
    // #endregion
    if (httpCode == 200) {
        // 開檔案準備寫入
        File file = LittleFS.open(update_file_path, "w");
        // #region agent log
        debugLog("before-fix", "H2", "transmit.cpp:Download_Update", "open_update_file_for_write",
            String("{\"file_open_ok\":") + (file ? "1" : "0") + "}");
        // #endregion
        
        // 串流寫入，不會一次塞爆 RAM
        WiFiClient* stream = http.getStreamPtr();
        uint8_t buf[256];
        int total = 0;

        while (http.connected() && total < http.getSize()) {
            int len = stream->read(buf, sizeof(buf));
            if (len > 0) {
                file.write(buf, len);
                total += len;
                Serial.printf("已下載：%d bytes\n", total);
                CRC32_Software(buf,len);
            }
        }

        result.firmware_size=(uint32_t)total;
        result.crc=crc ^ 0xFFFFFFFF;

        file.close();
        Serial.println("下載完成！");
    }
    http.end();
    return result;
}

uint32_t CRC32_Software_Chunk(uint8_t *data, uint32_t length)
{
    uint32_t crc_chunk = 0xFFFFFFFF;
    for (uint32_t i = 0; i < length; i++) {
        crc_chunk ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc_chunk & 1) crc_chunk = (crc_chunk >> 1) ^ 0xEDB88320;
            else         crc_chunk >>= 1;
        }
    }
    return crc_chunk ^ 0xFFFFFFFF;
}

void Transmit_Update(uint32_t this_chunk,uint32_t received,uint8_t *data){
    File file = LittleFS.open(update_file_path, "r");
    // #region agent log
    debugLog("before-fix", "H1", "transmit.cpp:Transmit_Update", "transmit_entry",
        String("{\"this_chunk\":") + this_chunk + ",\"received\":" + received +
        ",\"file_open_ok\":" + (file ? "1" : "0") + "}");
    // #endregion
    // #region agent log
    debugLog("before-fix", "H1", "transmit.cpp:Transmit_Update", "chunk_bounds_check",
        String("{\"chunk_lt_4\":") + ((this_chunk < 4) ? "1" : "0") +
        ",\"chunk_gt_528\":" + ((this_chunk > 528) ? "1" : "0") + "}");
    // #endregion
    file.seek(received);
    file.read(data,this_chunk-4);
    uint32_t crc = CRC32_Software_Chunk(data,this_chunk-4);
    uint8_t crc_0 = crc >> 0;
    uint8_t crc_1 = crc >> 8;
    uint8_t crc_2 = crc >> 16;
    uint8_t crc_3 = crc >> 24;
    data[this_chunk-4] = crc_3;
    data[this_chunk-3] = crc_2;
    data[this_chunk-2] = crc_1;
    data[this_chunk-1] = crc_0;
}