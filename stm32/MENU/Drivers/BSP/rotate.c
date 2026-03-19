#include "rotate.h"
#include "u8g2.h"
#include <math.h>

// 立方體的12條邊 (頂點索引對)
const uint8_t edges[12][2] = {
  {0,1},{1,2},{2,3},{3,0},  // 後面
  {4,5},{5,6},{6,7},{7,4},  // 前面
  {0,4},{1,5},{2,6},{3,7}   // 連接邊
};

// 原始頂點 (邊長2，中心在原點)
const float vertices[8][3] = {
  {-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1},
  {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1}
};

int16_t projected[8][2]; // 投影後的頂點坐標

////
//// 旋轉立方體並投影到2D平面上的函數
////
void rotate_and_project(float roll, float pitch, float yaw) {
    //算出旋轉矩陣的sin cos值

    // float roll_rad = roll * M_PI / 180.0f;
    // float pitch_rad = pitch * M_PI / 180.0f;
    float roll_rad = -pitch * M_PI / 180.0f; //因應實際情況調整
    float pitch_rad = roll * M_PI / 180.0f;
    float yaw_rad = yaw * M_PI / 180.0f;

    char message[20];
    //int message_length = sprintf(message, " %.2f, %.2f, %.2f\r\n", roll_rad, pitch_rad, yaw_rad);
    //HAL_UART_Transmit(&huart2, (uint8_t*)message, message_length, 50);  /* 50ms timeout instead of HAL_MAX_DELAY */
    sprintf(message, " roll:%.2f\r\n", roll);
    u8g2_SetFont(&u8g2, u8g2_font_5x8_tf); // Choose a suitable font
    u8g2_DrawUTF8(&u8g2,0,8,message);

    sprintf(message, " pitch:%.2f\r\n", pitch);
    u8g2_DrawUTF8(&u8g2,0,17,message);

    sprintf(message, " yaw:%.2f\r\n", yaw);
    u8g2_DrawUTF8(&u8g2,0,26,message);

    float cos_roll = cos(roll_rad); //這用的弧度
    float sin_roll = sin(roll_rad);

    float cos_pitch = cos(pitch_rad);
    float sin_pitch = sin(pitch_rad);

    float cos_yaw = cos(yaw_rad);
    float sin_yaw = sin(yaw_rad);

    //對每個頂點進行旋轉和投影
    for(int i =0; i<8; i++){
        float x = vertices[i][1];
        float y = vertices[i][0];
        float z = vertices[i][2];

        // 旋轉Rx
        float y1 = y*cos_roll - z*sin_roll;
        float z1 = y*sin_roll + z*cos_roll;

        // 旋轉Ry
        float x2 = x*cos_pitch + z1*sin_pitch;
        float z2 = -x*sin_pitch + z1*cos_pitch;

        // 旋轉Rz
        float x3 = x2*cos_yaw - y1*sin_yaw;
        float y3 = x2*sin_yaw + y1*cos_yaw;
        float z3 = z2;

        // 投影到2D平面 (簡單的透視投影)
        float distance = 5.0f; // 相機距離
        int k = 15; // 放大倍數
        projected[i][0] = (int16_t)(x3 * (distance / (distance + z3)) * k) + 96; // 投影後的X坐標，放大k倍並平移到中心
        projected[i][1] = (int16_t)(y3 * (distance / (distance + z3)) * k) + 32; // 投影後的Y坐標，放大k倍並平移到中心
    }
}

////
//// 繪製立方體邊緣的函數
////
void drawCube(float roll,float pitch,float yaw){
    rotate_and_project(roll, pitch, yaw);

    for(int i=0; i<12; i++){
        int x1 = projected[edges[i][0]][0];
        int y1 = projected[edges[i][0]][1];
        int x2 = projected[edges[i][1]][0];
        int y2 = projected[edges[i][1]][1];

        u8g2_DrawLine(&u8g2, x1, y1, x2, y2);
    }
}