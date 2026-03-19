#include "menu_render.h"
#include "u8g2.h"
#include <stdint.h>
#include <stdio.h>

const char weeks[8][12] = {      // ← 定義放在 .c
    "", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday", "Sunday"
};

////
//// 主選單渲染
////
void menu_render(MenuContext* ctx){
    u8g2_t* u8g2 = oled_get();
    u8g2_SetFont(u8g2, u8g2_font_6x10_tr);
    MenuItem* cur = ctx ->current;
    u8g2_ClearBuffer(u8g2);

    //標題
    u8g2_DrawStr(u8g2, 0, 10, cur ->label);

    //分隔線
    u8g2_DrawHLine(u8g2, 0, 12, 128);

    int k =0;
    //畫子節點列表
    for(int i = ctx ->top_edge;i< ctx->bottom_edge+1;i++ ){
        if(i==cur->selected_index){
            u8g2_DrawBox(u8g2, 0, 14+k*12, 128, 12);
            u8g2_SetDrawColor(u8g2, 0);
        }
        u8g2_DrawStr(u8g2, 2, 23+k*12, cur ->children[i]->label);
        u8g2_SetDrawColor(u8g2, 1);
        k++;
    }
    u8g2_SendBuffer(u8g2);
}

////
//// 時鐘畫面渲染
////
void screen_clock(){
    u8g2_t* u8g2 = oled_get();
    u8g2_ClearBuffer(u8g2);

    char date[25];
    char time[25];
    RTC_TimeTypeDef gTime;
    RTC_DateTypeDef gDate;
    if(ctx.edit_mode){
        gTime = tempTime;
        gDate = tempDate;
    }else {
        HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BIN);
        HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BIN);
    }
    // sprintf(date, "20%02d/%02d/%02d",
    //    gDate.Year,
    //    gDate.Month,
    //    gDate.Date);
    // sprintf(time, "%02d:%02d:%02d",
    //    gTime.Hours,
    //    gTime.Minutes,
    //    gTime.Seconds);
    // const char* weekday = weeks[gDate.WeekDay];
    // u8g2_DrawStr(u8g2, 14, 12, date);
    // u8g2_DrawStr(u8g2, 16,35 , time);
    // uint8_t x_week=(128-strlen(weekday)*8)/2;
    // u8g2_DrawStr(u8g2, x_week,55 , weekday);
    u8g2_SetFont(u8g2, u8g2_font_10x20_tr);

    char year[5], month[8], day[8];
    char hours[8], minutes[8], seconds[8];

    sprintf(year,    "%02d", gDate.Year);
    sprintf(month,   "%02d", gDate.Month);
    sprintf(day,     "%02d", gDate.Date);
    sprintf(hours,   "%02d", gTime.Hours);
    sprintf(minutes, "%02d", gTime.Minutes);
    sprintf(seconds, "%02d", gTime.Seconds);

    // x 座標（每個字元 10px，'/' 也是 10px）
    // 20YY / MM / DD 100px
    u8g2_DrawStr(u8g2,  14, 20, "20");
    u8g2_DrawStr(u8g2, 34, 20, year);
    u8g2_DrawStr(u8g2, 54, 20, "/");
    u8g2_DrawStr(u8g2, 64, 20, month);
    u8g2_DrawStr(u8g2, 84, 20, "/");
    u8g2_DrawStr(u8g2, 94, 20, day);

    // HH : MM : SS 80px
    u8g2_DrawStr(u8g2,  24, 45, hours);
    u8g2_DrawStr(u8g2, 44, 45, ":");
    u8g2_DrawStr(u8g2, 54, 45, minutes);
    u8g2_DrawStr(u8g2, 74, 45, ":");
    u8g2_DrawStr(u8g2, 84, 45, seconds);

    // ── 星期 ──────────────────────────────
    u8g2_SetFont(u8g2, u8g2_font_6x10_tr);
    const char* weekday = weeks[gDate.WeekDay];
    uint8_t x_week = (128 - strlen(weekday) * 6) / 2;
    u8g2_DrawStr(u8g2, x_week, 60, weekday);

    // ── 編輯模式光標 ──────────────────────
    if (ctx.edit_mode) {
        u8g2_SetFont(u8g2, u8g2_font_10x20_tr);

        // selected_index 對應的欄位底線位置
        // x=起始, y=底線y座標, w=寬度
        const uint8_t cursor_x[] = { 14, 64, 94,  24, 54, 84,  x_week };
        const uint8_t cursor_y[] = { 22, 22, 22, 47, 47, 47, 62 };
        const uint8_t cursor_w[] = { 40, 20, 20, 20, 20, 20, strlen(weekday) * 6};
        //                           年   月  日  時  分  秒  星期

        uint8_t idx = ctx.current->selected_index;
        if (idx < 7) {
            u8g2_DrawHLine(u8g2, cursor_x[idx], cursor_y[idx], cursor_w[idx]);
        }
    }
    u8g2_SendBuffer(u8g2);
}

////
//// 溫濕度畫面渲染
////
const uint8_t image_temperature[] ={
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x01, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x00, 0x38, 0x06, 0x00, 0x00, 0x1c, 0x0c, 0x00, 
0x00, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0x0c, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 
0x00, 0xcc, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 0x00, 0xcc, 0x0c, 0x00, 
0x00, 0xcc, 0x0c, 0x00, 0x00, 0xce, 0x1c, 0x00, 0x00, 0xe7, 0x39, 0x00, 0x00, 0xe3, 0x33, 0x00, 0x00, 0xf3, 0x33, 0x00, 0x00, 0xf3, 0x33, 0x00, 
0x00, 0xf3, 0x33, 0x00, 0x00, 0xe7, 0x39, 0x00, 0x00, 0x0e, 0x18, 0x00, 0x00, 0x1c, 0x0e, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x00, 0xf0, 0x03, 0x00, 
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};
const uint8_t image_humidity[] ={
0x00, 0x80, 0x01, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 
0x00, 0xf8, 0x1f, 0x00, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0xfe, 0x7f, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x80, 0xff, 0xff, 0x01, 
0xc0, 0x87, 0xff, 0x03, 0xc0, 0x03, 0xff, 0x03, 0xe0, 0x33, 0xcf, 0x07, 0xe0, 0x33, 0xc7, 0x07, 0xf0, 0x03, 0xe3, 0x0f, 0xf0, 0xc7, 0xf1, 0x0f, 
0xf0, 0xff, 0xf8, 0x0f, 0xf0, 0x7f, 0xfc, 0x0f, 0xf0, 0x3f, 0xfe, 0x0f, 0xf0, 0x1f, 0xff, 0x0f, 0xf0, 0x8f, 0xe3, 0x0f, 0xf0, 0xc7, 0xe1, 0x0f, 
0xe0, 0xe3, 0xcc, 0x07, 0xe0, 0xf3, 0xcc, 0x07, 0xc0, 0xff, 0xc0, 0x03, 0x80, 0xff, 0xe1, 0x03, 0x80, 0xff, 0xff, 0x01, 0x00, 0xfe, 0xff, 0x00, 
0x00, 0xfc, 0x3f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 
};
void screen_temperature(){
    u8g2_t* u8g2 = oled_get();
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFont(u8g2, u8g2_font_10x20_tr);
    float temp_val  = HTU21D_ReadTemperature();
    float humi_val  = HTU21D_ReadHumidity();

    char temperature[16];
    char humidity[16];

    sprintf(temperature,"%2.2f°C",temp_val);
    sprintf(humidity,"%2.2f%%",humi_val);
    //u8g2_DrawXBMP(u8g2,18,0,32,32,image_humidity);
    u8g2_DrawXBMP(u8g2,18,0,32,32,image_temperature);
    u8g2_DrawStr(u8g2,  18+32, 26, temperature);

    u8g2_DrawXBMP(u8g2,18,32,32,32,image_humidity);
    u8g2_DrawStr(u8g2,  18+32, 56, humidity);

    u8g2_SendBuffer(u8g2);
}

////
//// 舵機渲染
////
extern int servo_degree;
void screen_servo(){
    u8g2_t* u8g2 = oled_get();
    u8g2_ClearBuffer(u8g2);

    uint8_t degree_x = 64-15;

    char degree_str[8];
    sprintf(degree_str, "%d", servo_degree);
    u8g2_SetFont(u8g2, u8g2_font_10x20_tr);
    u8g2_DrawStr(u8g2,  degree_x, 44, degree_str);

    char plus_1[]="+1";
    char plus_10[]="+10";
    char sub_1[]="-1";
    char sub_10[]="-10";

    uint8_t plus_1_x = degree_x+30+5;
    uint8_t plus_10_x = plus_1_x+5+14;
    uint8_t sub_1_x = degree_x-5-14;
    uint8_t sub_10_x = sub_1_x-10-14;


    u8g2_SetFont(u8g2, u8g2_font_7x13_tf);
    u8g2_DrawStr(u8g2,  plus_1_x, 41, plus_1);
    u8g2_DrawStr(u8g2,  sub_1_x, 41, sub_1);

    u8g2_SetFont(u8g2, u8g2_font_6x10_tf );
    u8g2_DrawStr(u8g2,  plus_10_x, 39, plus_10);
    u8g2_DrawStr(u8g2,  sub_10_x, 39, sub_10);

    
    // ── 編輯模式光標 ──────────────────────
    if (ctx.edit_mode) {
        u8g2_SetFont(u8g2, u8g2_font_10x20_tr);

        // selected_index 對應的欄位底線位置
        // x=起始, y=底線y座標, w=寬度
        const uint8_t cursor_x[] = { sub_10_x, sub_1_x, plus_1_x,  plus_10_x};
        const uint8_t cursor_y[] = { 41, 41, 43, 43 };
        const uint8_t cursor_w[] = { 14, 18, 18, 14};
        //                          -10  -1  +1  +10

        uint8_t idx = ctx.current->selected_index;
        if (idx < 7) {
            u8g2_DrawHLine(u8g2, cursor_x[idx], cursor_y[idx], cursor_w[idx]);
        }
    }

    u8g2_SendBuffer(u8g2);
}

////
//// mpu6050互補濾波渲染
////
void screen_mpu6050_duplexer(){
    MPU6050_Read_All(&hi2c1);
    MPU6050_Update_Euler_Angles();
    MPU6050_Complementary_Filter();

    float roll,pitch,yaw;
    roll=MPU6050_Get_Roll_Bal();
    pitch = MPU6050_Get_Pitch_Bal();
    yaw = MPU6050_Get_Yaw_Bal();

    u8g2_ClearBuffer(&u8g2);
    drawCube(roll, pitch, yaw);
    u8g2_SendBuffer(&u8g2);
}

////
//// mpu6050卡爾曼濾波渲染
////
void screen_mpu6050_kalman(){
    float roll,pitch,yaw;
    MPU6050_Read_All(&hi2c1);
    MPU6050_Update_Euler_Angles();
    MPU6050_Kalman_Filter();

    roll = MPU6050_Get_Roll_Kalman();
    pitch = MPU6050_Get_Pitch_Kalman();
    yaw = MPU6050_Get_Yaw_Kalman();

    u8g2_ClearBuffer(&u8g2);
    drawCube(roll, pitch, yaw);
    u8g2_SendBuffer(&u8g2);
}

////
//// gif渲染
////
void screen_gif_capoo(){
    u8g2_t* u8g2 = oled_get();
    for (int i =0;i<21; i++) {
        u8g2_ClearBuffer(u8g2);
        u8g2_DrawXBMP(u8g2,0,0,68,64,image_capoo[i]);
        u8g2_SendBuffer(u8g2);
        osDelay(50);
    }
}