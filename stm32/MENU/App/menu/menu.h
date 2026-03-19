#ifndef MENU_H_
#define MENU_H_

#include <stdlib.h>    // 包含 NULL 和其他常用定義
#include "oled.h"
#include "rtc.h"
#include "stdio.h"
#include <stddef.h>
#include <stdint.h>
#include "u8g2.h"

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#define MIN(A,B) ((A) <= (B) ? (A):(B))
#define MAX(A,B) ((A) > (B) ? (A):(B))

// 向前聲明
typedef struct MenuContext MenuContext;

////
//// 畫面函數類別
////
typedef void (*ScreenFunc)(void);

typedef enum{
    BTN_UP = '5',
    BTN_DOWN = '8', 
    BTN_LEFT = '7', 
    BTN_RIGHT = '9', 
    BTN_ENTER = '#'
}ButtonEvent;

////
//// 選單物件
////
typedef struct MenuItem{
    const char* label;
    struct MenuItem* parent; //上一層
    struct MenuItem** children; //下一層 指標陣列
    uint8_t child_Count; //子選單數量
    uint8_t selected_index; //游標位置
    ScreenFunc on_enter; //觸發事件 當=null時代表有子選單
    void (*on_change)(int8_t dir); //當有需要調整的數值時 用於數值調整
}MenuItem;

////
//// 選單狀態
////
struct MenuContext{
    MenuItem*  current;    // 目前所在節點
    uint8_t    need_redraw; //=0時不重繪oled(只有在事件發生時重繪 eg.按下按鈕)
    uint8_t   edit_mode;; // 編輯模式 1=啟用
    ScreenFunc active_screen; // 目前顯示的畫面函式
    void (*handle_event)(MenuContext* ctx, ButtonEvent evt);
    int top_edge; //當前選單最上方index
    int bottom_edge; //當前選單最下方index
};

extern MenuContext ctx;

#endif