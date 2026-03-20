#include "menu_def.h"
#include "i2c.h"
#include "stm32f4xx_hal_rtc.h"


// MenuItem = { 標籤, 父節點 ,子選單 , 子選單數量 , 游標位置 , 觸發事件 , 數值調整}
// --- 葉節點 ---
MenuItem item_colck = { "Clock", NULL, NULL, 0, 0, action_clock,change_colck };
MenuItem item_temperature = { "Temperature", NULL, NULL, 0, 0, action_temperature,NULL};
MenuItem item_servo = { "Servo", NULL, NULL, 0, 0, action_servo, change_servo };
MenuItem item_setting = { "Setting", NULL, NULL, 0, 0, action_clock, NULL };
MenuItem item_gif = { "Gif", NULL, NULL, 0, 0, action_gif, NULL };

MenuItem item_mpu6050_Duplexer = { "Duplexer", NULL, NULL, 0, 0, action_mpu6050_duplexer, NULL };
MenuItem item_mpu6050_Kalman = { "Kalman", NULL, NULL, 0, 0, action_mpu6050_kalman, NULL};

MenuItem item_check_update = { "YES", NULL, NULL, 0, 0, action_update, NULL };
MenuItem item_no_check_update = { "NO", NULL, NULL, 0, 0, action_menu, NULL };
// --- 子選單 ---
MenuItem* mpu6050_children[]={&item_mpu6050_Duplexer,&item_mpu6050_Kalman};
MenuItem menu_mpu6050 = { "MPU6050", NULL, mpu6050_children, ARRAY_SIZE(mpu6050_children), 0, NULL, NULL };

MenuItem* update_children[]={&item_check_update,&item_no_check_update};
MenuItem menu_update = { "Are you sure you want to update?", NULL, update_children, ARRAY_SIZE(update_children), 0, NULL, NULL };
// --- 根節點 ---
MenuItem* root_children[] ={&item_colck,&item_temperature,&menu_mpu6050,&item_servo,&item_setting,&item_gif,&menu_update} ;
MenuItem menu_root = { "Main", NULL, root_children, ARRAY_SIZE(root_children), 0, NULL,NULL};

// 初始化現在位置
MenuContext ctx = {&menu_root,1,0,NULL,menu_handle_event,0,0};
////
//// 回填父節點
////
void menu_set_parents(MenuItem* node) {
    for (uint8_t i = 0; i < node->child_Count; i++) {
        node->children[i]->parent = node;
        menu_set_parents(node->children[i]);  // 遞迴往下
    }
}

////
//// 選單初始化
////
void MenuInit(){
    ctx.bottom_edge=MIN(3, menu_root.child_Count - 1);
    menu_set_parents(&menu_root);
}

////
//// 返回現在位置
////
MenuContext GetMenuContext(){
    return ctx;
}

////
//// 時鐘觸發事件
////
void action_clock(void) {
    ctx.active_screen = screen_clock;
    ctx.handle_event  = clock_handle_event;
    ctx.current=&item_colck;
    ctx.current->selected_index=0;
}

////
//// 編輯時鐘
////
void change_colck(int8_t dir){
    int val;
    switch (ctx.current->selected_index) {
        case 0://年
            val = (int)tempDate.Year;
            val=(val + dir > 99 ? 0 :val + dir);
            val=(val< 0 ? 99 : val);
            tempDate.Year=val;
            break;
        case 1://月
            val = (int)tempDate.Month;
            val=(val + dir > 12 ? 1 :val + dir);
            val=(val < 1 ? 12 : val);
            tempDate.Month=val;
            break;
        case 2://日
            val = (int)tempDate.Date;
            val=(val+ dir > 31 ? 1 :val + dir);
            val=(val < 1 ? 31 : val);
            tempDate.Date=val;
            break;
        case 3://時
            val = (int)tempTime.Hours;
            val=(val + dir > 23 ? 0 :val + dir);
            val=(val < 0 ? 23 : val);
            tempTime.Hours=val;
            break;           
        case 4://分
            val = (int)tempTime.Minutes;
            val=(val + dir > 59 ? 0 :val + dir);
            val=(val < 0 ? 59 : val);
            tempTime.Minutes=val;
            break;  
        case 5://秒
            val = (int)tempTime.Seconds;
            val=(val + dir > 60 ? 0 :val + dir);
            val=(val < 0 ? 59 : val);
            tempTime.Seconds=val;
            break; 
        case 6://星期
            val = (int)tempDate.WeekDay;
            val =(val + dir > 7 ? 1 :val + dir);
            val =(val  < 1 ? 7 : val );
            tempDate.WeekDay=val;
            break;         
    }
}

////
//// 回到menu
////
void action_menu(){
    ctx.edit_mode=0;
    ctx.need_redraw=1;
    ctx.bottom_edge=3;
    ctx.handle_event=menu_handle_event;
    ctx.top_edge=0;
    ctx.active_screen=NULL;
    ctx.current=&menu_root;
}

////
//// 溫濕度觸發事件
////
void action_temperature(void) {
    ctx.active_screen = screen_temperature;
    ctx.handle_event  = engine_handle_event;
    ctx.current=&item_temperature;
    ctx.current->selected_index=0;
    HTU21D_Init();
}

////
//// 舵機觸發事件
////
int servo_degree = 0;
void action_servo(void) {
    ctx.active_screen = screen_servo;
    ctx.handle_event  = servo_handle_event;
    ctx.current=&item_servo;
    ctx.current->selected_index=0;
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HTU21D_Init();
    int compare = (0 * 2000 / 180) + 500;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
}

////
//// 編輯舵機
////
void change_servo(int8_t dir){
    switch (ctx.current->selected_index){
        case 3:
            if(servo_degree+10<141){
                servo_degree+=10;
                int compare = (servo_degree * 2000 / 180) + 500;
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
            }
            break; 
        case 2:
            if(servo_degree<141){
                servo_degree++;
                int compare = (servo_degree * 2000 / 180) + 500;
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
            }
            break; 
        case 1:
            if(servo_degree>0){
                servo_degree--;
                int compare = (servo_degree * 2000 / 180) + 500;
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
            }
            break; 
        case 0:
            if(servo_degree-9>0){
                servo_degree-=10;
                int compare = (servo_degree * 2000 / 180) + 500;
                __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, compare);
            }
            break; 
    }
}

////
//// mpu6050_互補濾波觸發事件
////
void action_mpu6050_duplexer(void) {
    ctx.active_screen = screen_mpu6050_duplexer;
    ctx.handle_event  = engine_handle_event;
    ctx.current=&item_mpu6050_Duplexer;
    ctx.current->selected_index=0;
    MPU6050_Init(&hi2c1);
}

////
//// mpu6050卡爾曼濾波觸發事件
////
void action_mpu6050_kalman(void) {
    ctx.active_screen = screen_mpu6050_kalman;
    ctx.handle_event  = engine_handle_event;
    ctx.current=&item_mpu6050_Kalman;
    ctx.current->selected_index=0;
    MPU6050_Init(&hi2c1);
}

////
//// gif觸發事件
////
void action_gif(void) {
    ctx.active_screen = screen_gif_capoo;
    ctx.handle_event  = engine_handle_event;
    ctx.current=&item_gif;
    //ctx.current->selected_index=0;
}

////
//// 更新觸發事件
////
void action_update(void){
    uint32_t sp = *(uint32_t*)BOOTLOADER_ADDRESS;
    if (sp >0x20000000U && sp <0x2000FFFFU){
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
        SCB->VTOR=BOOTLOADER_ADDRESS;
        //獲取app的reset_handler
        pFunction bootloader_reset_handler = (pFunction)(*(uint32_t*)(BOOTLOADER_ADDRESS+4));
        //設置msp
        __set_MSP(*(uint32_t*)BOOTLOADER_ADDRESS);
        //清除check_number
        *(uint8_t*)(CHECK_SRAM_ADDRESS) = check_number;
        //跳轉到bootloader
        bootloader_reset_handler();
    }
}