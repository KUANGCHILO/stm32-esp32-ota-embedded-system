#include <ESP32DMASPISlave.h>

#include <WiFi.h>
#include <HTTPClient.h>
#include "transmit.h"

const char* ssid = WiFi_ssid;
const char* password = WiFi_password;

uint8_t *tx_buf_Check_Version;
uint8_t *tx_buf_Header;
uint8_t *tx_buf_update;
uint8_t *rx_buf_Check_Version;
uint8_t *rx_buf_Header;
uint8_t *rx_buf_update;
uint8_t *tx_buf;
uint8_t *rx_buf;
uint32_t this_chunk=0;
uint32_t received=0;

FirmwareHeader_t message;

ESP32DMASPI::Slave slave;

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_SCK  18
#define VSPI_CS    5

#define cmd_size 12
#define check_version_size 4
#define data_size (4080)

#define header_size sizeof(FirmwareHeader_t)

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

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("未連接");
  }

  Serial.println("\n已連線！IP: " + WiFi.localIP().toString());
  // #region agent log
  debugLog("before-fix", "H4", "update.ino:setup", "wifi_connected", "{\"status\":1}");
  // #endregion

  // 初始化 SPI 從機
  slave.setDataMode(SPI_MODE0);           // default: SPI_MODE0
  slave.setMaxTransferSize(1024);  // default: 4092 bytes
  slave.setQueueSize(1);         // default: 1
  


  LittleFS.begin(true);

  tx_buf = slave.allocDMABuffer(cmd_size);
  rx_buf = slave.allocDMABuffer(cmd_size);
  tx_buf_Check_Version = slave.allocDMABuffer(check_version_size);
  tx_buf_Header=slave.allocDMABuffer(sizeof(FirmwareHeader_t));
  tx_buf_update=slave.allocDMABuffer(data_size);
  rx_buf_Check_Version = slave.allocDMABuffer(check_version_size);
  rx_buf_Header=slave.allocDMABuffer(sizeof(FirmwareHeader_t));
  rx_buf_update=slave.allocDMABuffer(data_size);
  // #region agent log
  debugLog("before-fix", "H5", "update.ino:setup", "dma_alloc", String("{\"tx\":") + (tx_buf ? "1" : "0") +
    ",\"rx\":" + (rx_buf ? "1" : "0") +
    ",\"check\":" + (tx_buf_Check_Version ? "1" : "0") +
    ",\"header\":" + (tx_buf_Header ? "1" : "0") +
    ",\"update\":" + (tx_buf_update ? "1" : "0") + "}");
  // #endregion
  slave.begin(VSPI, VSPI_SCK, VSPI_MISO, VSPI_MOSI, VSPI_CS);

}

void printHex(const char* label, uint8_t* buf, int len) {
    Serial.printf("[%s] %d bytes: ", label, len);
    for (int i = 0; i < len; i++) {
        Serial.printf("%02X ", buf[i]);
    }
    Serial.println();
}

void loop() {
    memset(tx_buf, 0, cmd_size);
    memset(rx_buf, 0, cmd_size);

    // 加 timeout，超時不崩潰
    bool received_data = slave.transfer(tx_buf, rx_buf, cmd_size, 60000);

    if (!received_data) {
        Serial.println("等待指令逾時...");
        // #region agent log
        debugLog("before-fix", "H4", "update.ino:loop", "transfer_timeout", "{\"timeout_ms\":60000}");
        // #endregion
        return;  // 重新等待
    }

    printHex("收到指令", rx_buf, cmd_size);

    switch (rx_buf[0]) {
    case cmd_check_version:
        // #region agent log
        debugLog("before-fix", "H3", "update.ino:cmd_check_version", "enter_check_version", "{\"cmd\":1}");
        // #endregion
        tx_buf_Check_Version[0] = Check_Version();
        printHex("傳出 Check_Version", tx_buf_Check_Version, check_version_size);
        slave.transfer(tx_buf_Check_Version, rx_buf_Check_Version, check_version_size,60000);
        memset(tx_buf_Check_Version, 0, check_version_size);
        break;

    case cmd_download_update:
        Serial.println("[Download] 開始下載...");
        // #region agent log
        debugLog("before-fix", "H3", "update.ino:cmd_download_update", "enter_download", "{\"cmd\":2}");
        // #endregion
        message = Download_Update();
        memcpy(tx_buf_Header, &message, sizeof(FirmwareHeader_t));
        printHex("傳出 Header", tx_buf_Header, sizeof(tx_buf_Header));
        Serial.printf("[Download] firmware_size=%u, crc=0x%08X\n",
            message.firmware_size, message.crc);
        slave.transfer(tx_buf_Header, rx_buf_Header,header_size ,60000);
        memset(tx_buf_Header, 0, header_size);
        break;

    case cmd_transmit_update:
        this_chunk = ((uint32_t)rx_buf[1] << 24) | ((uint32_t)rx_buf[2] << 16)
                   | ((uint32_t)rx_buf[3] << 8)  |  (uint32_t)rx_buf[4];
        received   = ((uint32_t)rx_buf[5] << 24) | ((uint32_t)rx_buf[6] << 16)
                   | ((uint32_t)rx_buf[7] << 8)  |  (uint32_t)rx_buf[8];
        // #region agent log
        debugLog("before-fix", "H1", "update.ino:cmd_transmit_update", "parsed_chunk_and_offset",
          String("{\"this_chunk\":") + this_chunk + ",\"received\":" + received + ",\"max_data_size\":" + data_size + "}");
        // #endregion
        Serial.printf("[Transmit] this_chunk=%u, received=%u\n", this_chunk, received);
        memset(tx_buf_update, 0, data_size);
        Transmit_Update(this_chunk, received, tx_buf_update);
        printHex("傳出 Update 前8bytes", tx_buf_update, 8);  // 只印前8個，避免太長
        Serial.printf("[Transmit] 傳出 %u bytes\n", data_size);
        slave.transfer(tx_buf_update, rx_buf_update, data_size,60000);
        memset(tx_buf_update, 0, data_size);
        break;

    default:
        Serial.printf("[Warning] 未知指令：0x%02X\n", rx_buf[0]);
        break;
    }
}
