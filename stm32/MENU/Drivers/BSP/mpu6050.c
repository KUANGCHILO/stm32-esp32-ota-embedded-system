#include "mpu6050.h"
#include "cmsis_os2.h"

static float ax, ay, az; // 加速度計數據
static float gx, gy, gz; // 陀螺儀數據
static float gx_offset = 0;
static float gy_offset = 0;
static float gz_offset = 0;

static float roll_a, pitch_a, yaw_a; // 使用加速度計計算的歐拉角數據
static float roll_g, pitch_g, yaw_g; // 使用陀螺儀計算的歐拉角數據
static float roll_Bal, pitch_Bal, yaw_Bal; // 使用互補濾波融合的歐拉角數據

static const float dt = 0.01; // 更新間隔，假設每10ms更新一次
static const float alpha = 0.95f;

static float roll_xt[2];
static float pitch_xt[2]; 
static const float F[2][2] = {{1, -dt}, 
                            {0, 1}};   
static float P_roll[2][2] = {{1, 0}, 
                            {0, 1}}; // 初始協方差矩陣     
static float P_pitch[2][2] = {{1, 0}, 
                            {0, 1}}; // 初始協方差矩陣
static const float Q[2][2] = {{0.001, 0}, 
                            {0, 0.003}}; // 過程噪聲協方差矩陣 
static const float R = 0.03; // 觀測噪聲協方差

void MPU6050_Init(I2C_HandleTypeDef *hi2c)
{
    //復位MPU6050
    uint8_t reset = DEVICE_RESET;
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &reset, 1, 500);
    osDelay(100);

    //喚醒MPU6050
    uint8_t wakeup = DEVICE_WAKEUP; //喚醒
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &wakeup, 1, 500);

    //設置加速度計
    uint8_t accel_config = ACCEL_2G; //加速度計量程±2g
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &accel_config, 1, 500);

    //設置陀螺儀
    uint8_t gyro_config = GYRO_250DPS; //陀螺儀量程±250°/s
    HAL_I2C_Mem_Write(hi2c, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &gyro_config, 1, 500);
}

void MPU6050_Read_All(I2C_HandleTypeDef *hi2c)
{
    uint8_t buf[14];

    HAL_I2C_Mem_Read(hi2c,
                     MPU6050_ADDR,
                     0x3B,
                     1,
                     buf,
                     14,
                     500);  /* 500ms timeout instead of HAL_MAX_DELAY */

    int16_t raw_ax = (buf[0] << 8) | buf[1];
    int16_t raw_ay = (buf[2] << 8) | buf[3];
    int16_t raw_az = (buf[4] << 8) | buf[5];

    int16_t raw_gx = (buf[8] << 8) | buf[9];
    int16_t raw_gy = (buf[10] << 8) | buf[11];
    int16_t raw_gz = (buf[12] << 8) | buf[13];

    ax = raw_ax / 16384.0f;
    ay = raw_ay / 16384.0f;
    az = raw_az / 16384.0f;

    gx = raw_gx / 131.0f;
    gy = raw_gy / 131.0f;
    gz = raw_gz / 131.0f;
}

////
//// MPU6050校準陀螺儀偏移的函數
////
void MPU6050_Calibrate_Gyro(I2C_HandleTypeDef *hi2c)
{
    gx_offset = 0;
    gy_offset = 0;
    gz_offset = 0;

    for(int i = 0; i < 500; i++)
    {
        MPU6050_Read_All(hi2c);

        gx_offset += gx;
        gy_offset += gy;
        gz_offset += gz;

        osDelay(10);
    }

    gx_offset /= 500;
    gy_offset /= 500;
    gz_offset /= 500;

    roll_xt[0] = 0;  roll_xt[1] = gx_offset;

    pitch_xt[0] = 0; pitch_xt[1] = gy_offset;
}

////
//// 獲取加速度計數據的函數
////
float MPU6050_Get_Accel_X(void)
{
    return ax;
}

float MPU6050_Get_Accel_Y(void)
{
    return ay;
}

float MPU6050_Get_Accel_Z(void)
{
    return az;
}

////
//// 獲取陀螺儀數據的函數
////
float MPU6050_Get_Gyro_X(void)
{
    return gx;
}

float MPU6050_Get_Gyro_Y(void)
{
    return gy;
}

float MPU6050_Get_Gyro_Z(void)
{
    return gz;
}

////
//// 歐拉角計算函數
////

////
//// 使用加速度計數據計算歐拉角的函數
////

////
//// 翻滾角計算函數
////
float MPU6050_Get_Roll_By_Accel(void)
{
    return atan2f(ay, az) * 180.0 / M_PI; // 計算翻滾角
}

////
//// 俯仰角計算函數
////
float MPU6050_Get_Pitch_By_Accel(void)
{
    return atan2f(-ax, sqrtf(ay * ay + az * az)) * 180.0 / M_PI; // 計算俯仰角
}

////
//// 偏航角計算函數
////
float MPU6050_Get_Yaw_By_Accel(void)
{
    // 加速度計無法直接計算偏航角，可能需要結合磁力計數據或使用陀螺儀數據進行積分來估算偏航角
    return 0.0;
}

////
//// 使用陀螺儀數據計算歐拉角的函數
////

////
//// 翻滾角計算函數
////
float MPU6050_Get_Roll_By_Gyro(void)
{
    return roll_g + (gx - gx_offset) * dt; // 計算翻滾角增量
}

////
//// 俯仰角計算函數
////
float MPU6050_Get_Pitch_By_Gyro(void)
{
    return pitch_g + (gy - gy_offset) * dt; // 計算俯仰角增量
}

////
//// 偏航角計算函數
////
float MPU6050_Get_Yaw_By_Gyro(void)
{
    return yaw_g + (gz - gz_offset) * dt; // 計算偏航角增量
}

////
//// 更新歐拉角的函數
////
void MPU6050_Update_Euler_Angles(void)
{
    // 使用加速度計數據計算歐拉角
    roll_a = MPU6050_Get_Roll_By_Accel();
    pitch_a = MPU6050_Get_Pitch_By_Accel();
    yaw_a = MPU6050_Get_Yaw_By_Accel();

    // 使用陀螺儀數據計算歐拉角增量
    roll_g = MPU6050_Get_Roll_By_Gyro() ;
    pitch_g = MPU6050_Get_Pitch_By_Gyro() ;
    yaw_g = MPU6050_Get_Yaw_By_Gyro() ;
}

////
//// 互補濾波器融合加速度計和陀螺儀數據的函數
////
void MPU6050_Complementary_Filter(void)
{

    // 融合加速度計和陀螺儀數據計算最終的歐拉角
    roll_Bal = alpha * roll_g + (1 - alpha) * roll_a;
    pitch_Bal = alpha * pitch_g + (1 - alpha) * pitch_a;
    yaw_Bal = yaw_g; // 由於加速度計無法提供偏航角數據，因此直接使用陀螺儀數據

    roll_g  = roll_Bal; // 更新陀螺儀角度為融合後的結果，這樣下一次計算增量時會基於融合後的角度進行
    pitch_g = pitch_Bal;
}

////
//// 獲取互補濾波融合後的翻滾角函數
////
float MPU6050_Get_Roll_Bal(void)
{
    return roll_Bal;
}

////
//// 獲取互補濾波融合後的俯仰角函數
////
float MPU6050_Get_Pitch_Bal(void)
{
    return pitch_Bal;
}   

////
//// 獲取互補濾波融合後的偏航角函數
////
float MPU6050_Get_Yaw_Bal(void)
{
    return yaw_Bal;
}

////
//// 卡爾曼濾波器融合加速度計和陀螺儀數據的函數
////
void MPU6050_Kalman_Filter(void)
{
    //第一步 預測狀態
    //xt_predict = F * xt-1 + Bu * ut
    // x = [θ , b] //狀態向量，包含角度θ和陀螺儀偏移b

    // F = [1, -dt; 
    //       0, 1] //狀態轉移矩陣，假設角度隨時間變化，陀螺儀偏移保持不變

    float roll_xt_predict[2][1] = {{roll_xt[0]*F[0][0] + roll_xt[1]*F[0][1] + gx * dt}, //gx * dt是Bu * ut
                                {roll_xt[0]*F[1][0] + roll_xt[1]*F[1][1]}};
    float pitch_xt_predict[2][1] = {{pitch_xt[0]*F[0][0] + pitch_xt[1]*F[0][1] + gy * dt}, 
                                 {pitch_xt[0]*F[1][0] + pitch_xt[1]*F[1][1]}};

    //第二步 預測協方差
    //Pt = F * Pt-1 * F' + Q
    float P_predict_roll[2][2];
    P_predict_roll[0][0] = P_roll[0][0] - P_roll[1][0] * dt - P_roll[0][1] * dt + P_roll[1][1] * dt * dt + Q[0][0];
    P_predict_roll[0][1] = P_roll[0][1] - P_roll[1][1] * dt ;
    P_predict_roll[1][0] = P_roll[1][0] - P_roll[1][1] * dt ;
    P_predict_roll[1][1] = P_roll[1][1] + Q[1][1];

    float P_predict_pitch[2][2];
    P_predict_pitch[0][0] = P_pitch[0][0] - P_pitch[1][0] * dt - P_pitch[0][1] * dt + P_pitch[1][1] * dt * dt + Q[0][0];
    P_predict_pitch[0][1] = P_pitch[0][1] - P_pitch[1][1] * dt ;
    P_predict_pitch[1][0] = P_pitch[1][0] - P_pitch[1][1] * dt ;
    P_predict_pitch[1][1] = P_pitch[1][1] + Q[1][1];

    //第三步 計算卡爾曼增益
    //K = Pt * H' * (H * Pt * H' + R)^-1
    //H = [1, 0] //觀測矩陣，假設觀測值僅包含角度θ
    float K_roll[2];
    float H_P_predict_roll; // (H * Pt * H' + R)^-1

    H_P_predict_roll = 1.0f / (P_predict_roll[0][0] + R); // H_P_predict = (H * Pt * H' + R)^-1
    K_roll[0] = P_predict_roll[0][0] * H_P_predict_roll; // K[0][0] = Pt * H' * (H * Pt * H' + R)^-1
    K_roll[1] = P_predict_roll[1][0] * H_P_predict_roll; // K[0][1] = Pt * H' * (H * Pt * H' + R)^-1
    
    float K_pitch[2];
    float H_P_predict_pitch; // (H * Pt * H' + R)^-1

    H_P_predict_pitch = 1.0f / (P_predict_pitch[0][0] + R); // H_P_predict = (H * Pt * H' + R)^-1
    K_pitch[0] = P_predict_pitch[0][0] * H_P_predict_pitch; // K[0][0] = Pt * H' * (H * Pt * H' + R)^-1
    K_pitch[1] = P_predict_pitch[1][0] * H_P_predict_pitch; // K[0][1] = Pt * H' * (H * Pt * H' + R)^-1
    
    //第四步 更新狀態
    //xt = xt_predict + K * (zt - H * xt_predict)
    float zt_roll = roll_a; // 觀測值，使用加速度計計算的翻滾角
    float zt_pitch = pitch_a; // 觀測值，使用加速度計計算的俯仰角

    float H_roll_xt_predict; // H * xt_predict
    H_roll_xt_predict = roll_xt_predict[0][0]; // H * xt_predict = xt_predict[0][0]，因為H = [1, 0]

    float H_pitch_xt_predict; // H * xt_predict
    H_pitch_xt_predict = pitch_xt_predict[0][0]; // H * xt_predict = xt_predict[0][0]，因為H = [1, 0]
    
    float K_roll_2[2]; // K * (zt - H * xt_predict)
    float K_pitch_2[2]; // K * (zt - H * xt_predict)
    K_roll_2[0] = K_roll[0] * (zt_roll - H_roll_xt_predict);
    K_roll_2[1] = K_roll[1] * (zt_roll - H_roll_xt_predict);

    K_pitch_2[0] = K_pitch[0] * (zt_pitch - H_pitch_xt_predict);
    K_pitch_2[1] = K_pitch[1] * (zt_pitch - H_pitch_xt_predict);
    
    roll_xt[0] = roll_xt_predict[0][0] + K_roll_2[0]; //roll_xt[0]是角度，roll_xt[1]是陀螺儀偏移
    roll_xt[1] = roll_xt_predict[1][0] + K_roll_2[1];

    pitch_xt[0] = pitch_xt_predict[0][0] + K_pitch_2[0]; //pitch_xt[0]是角度，pitch_xt[1]是陀螺儀偏移
    pitch_xt[1] = pitch_xt_predict[1][0] + K_pitch_2[1];
    
    //第五步 更新協方差
    //Pt = (I - K * H) * Pt

    float I_KH_roll[2][2]; // I - K * H
    I_KH_roll[0][0] = 1 - K_roll[0] ;
    I_KH_roll[0][1] = 0 ;
    I_KH_roll[1][0] = -K_roll[1];
    I_KH_roll[1][1] = 1;
    P_roll[0][0] = I_KH_roll[0][0] * P_predict_roll[0][0] ;
    P_roll[0][1] = I_KH_roll[0][0] * P_predict_roll[0][1];
    P_roll[1][0] = I_KH_roll[1][0] * P_predict_roll[0][0] + P_predict_roll[1][0];
    P_roll[1][1] = I_KH_roll[1][0] * P_predict_roll[0][1] + P_predict_roll[1][1];

    float I_KH_pitch[2][2]; // I - K * H
    I_KH_pitch[0][0] = 1 - K_pitch[0] ;
    I_KH_pitch[0][1] = 0 ;
    I_KH_pitch[1][0] = -K_pitch[1];
    I_KH_pitch[1][1] = 1;

    P_pitch[0][0] = I_KH_pitch[0][0] * P_predict_pitch[0][0] ;
    P_pitch[0][1] = I_KH_pitch[0][0] * P_predict_pitch[0][1];
    P_pitch[1][0] = I_KH_pitch[1][0] * P_predict_pitch[0][0] + P_predict_pitch[1][0];
    P_pitch[1][1] = I_KH_pitch[1][0] * P_predict_pitch[0][1] + P_predict_pitch[1][1];

}

////
//// 獲取卡爾曼濾波融合後的翻滾角函數
////
float MPU6050_Get_Roll_Kalman(void)
{
    return roll_xt[0];
}

////
//// 獲取卡爾曼濾波融合後的俯仰角函數
////
float MPU6050_Get_Pitch_Kalman(void)
{
    return -pitch_xt[0];
}

////
//// 獲取卡爾曼濾波融合後的偏航角函數
////
float MPU6050_Get_Yaw_Kalman(void)
{
    // 由於卡爾曼濾波器僅融合了翻滾角和俯仰角，因此偏航角仍然使用陀螺儀數據
    return yaw_g;
}