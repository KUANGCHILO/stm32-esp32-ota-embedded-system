#include <WiFi.h>
#include <HTTPClient.h>
#include <ESP32SPISlave.h>
#include <Wire.h>
#include "transmit.h"

const char* ssid = WiFi_ssid;
const char* password = WiFi_password;

uint8_t tx_buf_Check_Version[1] = {0};
uint8_t tx_buf_Header[sizeof(FirmwareHeader_t)] = {0};
uint8_t tx_buf_update[530] = {0};
uint8_t tx_buf[32] = {0};
uint8_t rx_buf[32] = {0};
uint32_t this_chunk=0;
uint32_t received=0;

FirmwareHeader_t message;

ESP32SPISlave slave;

#define VSPI_MOSI 23
#define VSPI_MISO 19
#define VSPI_SCK  18
#define VSPI_CS    5

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

  // 初始化 SPI 從機
  slave.setDataMode(SPI_MODE0);
  slave.begin(VSPI, VSPI_SCK, VSPI_MISO, VSPI_MOSI, VSPI_CS);

    // 準備要回傳給主機的資料
  memset(tx_buf, 0, sizeof(tx_buf));
  LittleFS.begin(true);
}

void loop() {
  // put your main code here, to run repeatedly:
    // if (slave.remained() == 0) {
    //     slave.queue(rx_buf, tx_buf, BUFFER_SIZE);
    // }
    memset(tx_buf, 0, sizeof(tx_buf));
    memset(rx_buf, 0, sizeof(rx_buf));
    slave.transfer(tx_buf,rx_buf,sizeof(rx_buf));
    // 有收到資料時處理

    switch (rx_buf[0]) {
    case cmd_check_version:
      tx_buf_Check_Version[0] = Check_Version();
      slave.transfer(tx_buf_Check_Version,rx_buf,sizeof(rx_buf));
      memset(tx_buf_Check_Version, 0, sizeof(tx_buf_Check_Version));
      break;
    case cmd_download_update:
      message=Download_Update();
      memcpy(tx_buf_Header, &message, sizeof(FirmwareHeader_t));
      slave.transfer(tx_buf_Header,rx_buf,sizeof(rx_buf));
      memset(tx_buf_Header, 0, sizeof(tx_buf_Header));
      break;
    case cmd_transmit_update:
      this_chunk = (rx_buf[1] << 24) | (rx_buf[2] << 16) | (rx_buf[3] << 8) | (rx_buf[4] << 0);
      received = (rx_buf[5] << 24) | (rx_buf[6] << 16) | (rx_buf[7] << 8) | (rx_buf[8] << 0);
      Transmit_Update(this_chunk,received,tx_buf_update);
      slave.transfer(tx_buf_update,rx_buf,this_chunk);
      memset(tx_buf_update, 0, sizeof(tx_buf_update));
      break;
    }
    memset(rx_buf, 0, sizeof(rx_buf));  // 清空接收緩衝
    // slave.pop();
}
