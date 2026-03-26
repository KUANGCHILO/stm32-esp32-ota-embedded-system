# STM32 + ESP32 OTA 嵌入式系統專案

這是一個為了面試展示與嵌入式學習而完成的多板協作專案。專案結合了 STM32 主程式、STM32 自製 Bootloader，以及 ESP32 OTA 中介模組，完整展示了韌體更新流程、即時系統任務切分、周邊整合與嵌入式 UI 設計能力。

## 專案簡介

整個系統主要分成三個部分：

1. `stm32/MENU`
   STM32F401 的主應用程式，負責 OLED 選單、矩陣鍵盤操作、感測器讀取、伺服馬達控制與姿態顯示。
2. `stm32/bootloader`
   自製 Bootloader，負責檢查更新、透過 SPI 接收韌體、進行 CRC 驗證、寫入 Flash，最後跳轉到主程式。
3. `esp32/update`
   ESP32 OTA 中介模組，負責連接 Wi-Fi、從 GitHub 下載版本資訊與韌體、儲存在 LittleFS 中，並透過 SPI DMA 把更新資料提供給 STM32。

## 核心功能

- 自製 STM32 OTA 更新流程
- 以遠端 `version.txt` 進行版本比對
- 韌體分段傳輸與 CRC32 驗證
- STM32 Bootloader 的 Flash 擦除、寫入、跳轉與備份還原邏輯
- 基於 FreeRTOS 的主程式架構
- OLED 圖形選單介面
- 4x4 Keypad 使用者輸入
- RTC 時鐘頁面與時間編輯
- HTU21D 溫濕度顯示
- PWM 伺服馬達角度控制
- MPU6050 姿態顯示，包含互補濾波與 Kalman 濾波
- OLED 動畫播放

## 系統架構

```text
GitHub 韌體與版本檔
        |
        v
ESP32 OTA 中介
  - Wi-Fi
  - HTTP/HTTPS 下載
  - LittleFS 儲存
  - SPI DMA Slave
        |
        v
STM32 Bootloader
  - 檢查更新
  - 分段接收
  - CRC32 驗證
  - Flash 寫入
  - 跳轉主程式
        |
        v
STM32 主應用
  - FreeRTOS 任務
  - OLED UI
  - Keypad 輸入
  - 感測器與致動器
```

## OTA 更新流程

1. STM32 Bootloader 先向 ESP32 詢問是否有新版本。
2. ESP32 從 GitHub 讀取遠端版本號，並與 LittleFS 中的本地版本比較。
3. 若需要更新，ESP32 會下載 STM32 的韌體 `.bin` 檔並暫存。
4. STM32 向 ESP32 取得韌體標頭資訊，包含大小與 CRC32。
5. Bootloader 透過 SPI 分段接收韌體資料。
6. 每個資料區塊都會先做 CRC 驗證，再寫入 STM32 Flash。
7. 韌體寫入成功後，Bootloader 跳轉到主應用程式。
8. 若更新異常，系統也保留了備份還原機制。

## 主程式設計

STM32 主程式採用輕量化的嵌入式 UI 架構，主要組成如下：

- `freertos.c`
  建立按鍵任務與畫面渲染任務。
- `App/menu`
  負責選單定義、事件處理與畫面切換。
- `Drivers/BSP`
  封裝 OLED、HTU21D、MPU6050 與 3D 姿態繪圖等板級驅動。

目前選單內容包含：

- 時鐘
- 溫濕度
- MPU6050 互補濾波顯示
- MPU6050 Kalman 濾波顯示
- 伺服馬達控制
- GIF 動畫
- 更新入口

## 使用技術

- C / C++
- STM32CubeMX
- STM32 HAL
- FreeRTOS
- CMake
- ESP32 Arduino Framework
- Wi-Fi / HTTPClient / WiFiClientSecure
- LittleFS
- U8g2 圖形函式庫
- SPI、I2C、UART、RTC、PWM

## 資料夾結構

```text
project/
├─ esp32/
│  └─ update/        # ESP32 OTA 中介模組
├─ stm32/
│  ├─ bootloader/    # STM32 自製 Bootloader
│  └─ MENU/          # STM32 主應用程式
└─ README.md
```

## 建置說明

此專案包含 STM32CubeMX 產生的工程，以及 CMake 建置設定：

- `stm32/MENU` 會建出主程式，並產生 `MENU.bin`
- `stm32/bootloader` 會建出 Bootloader，並產生 `bootloader.bin`
- `esp32/update` 是 ESP32 Arduino 草稿，用來負責 OTA 傳輸

目前 ESP32 程式中的 OTA URL 指向 GitHub 上的韌體與版本檔，因此更新行為會依照遠端發布內容而定。

## 備註

為了方便調試：

- WiFi密碼以明文放置於程式碼中，實際情況需放置在LittleFS中，並依據情況選擇是否加密
- version.txt在更新成功後沒有更新本地文件，實際情況在stm32完成更新後需回傳一指令讓esp32更新version.txt