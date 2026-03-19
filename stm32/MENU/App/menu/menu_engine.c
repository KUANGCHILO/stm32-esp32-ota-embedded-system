#include "menu_engine.h"


////
//// 選單狀態機
//// MenuContext 當前節點 ButtonEvent 輸入按鍵
void menu_handle_event(MenuContext* ctx, ButtonEvent evt){
    MenuItem* cur = ctx ->current;
    switch(evt){
        case BTN_UP:
            if(cur -> selected_index > 0){
                cur -> selected_index--;
                ctx ->top_edge = MIN(ctx ->top_edge, cur->selected_index);
                ctx ->bottom_edge = MIN(ctx ->top_edge+3,ctx->current->child_Count-1);
                break;
            }
            cur -> selected_index = cur -> child_Count -1 ;
            ctx ->bottom_edge = cur->child_Count-1;
            ctx ->top_edge =MAX(0, ctx ->bottom_edge-3);
            break;
        case BTN_DOWN:
            if(cur -> selected_index < cur -> child_Count -1){
                cur -> selected_index++;
                ctx ->bottom_edge = MAX(ctx ->bottom_edge,cur->selected_index);
                ctx ->top_edge = MAX(0, ctx->bottom_edge - 3);             
                break;
            }
            ctx ->top_edge =0;
            ctx ->bottom_edge =MIN(3, ctx->current->child_Count-1);
            cur -> selected_index = 0 ;
            break;
        case BTN_RIGHT: //進入選單
            if(cur->children[cur ->selected_index] ->children){
                ctx ->current = cur ->children[cur ->selected_index];
                ctx->top_edge    = 0;
                ctx->bottom_edge = MIN(3, cur->children[cur ->selected_index] ->child_Count - 1);
                cur->selected_index = 0; 
            }else if(cur->children[cur ->selected_index] -> on_enter){
                ctx->active_screen = cur->children[cur ->selected_index] ->on_enter;
            }
            break;
        case BTN_LEFT: //返回上層
            if(cur->parent){
                ctx ->current = cur ->parent;
                ctx ->active_screen = NULL;
                ctx ->edit_mode = 0;
                ctx->top_edge    = 0;
                ctx->bottom_edge = MIN(3, ctx->current->child_Count - 1);
                break;
            }
            break;
        case BTN_ENTER:
            // BTN_ENTER 處理
            break;
        default:
            return;
    }
    ctx->need_redraw = 1;
}

////
//// 時鐘狀態機
////
RTC_TimeTypeDef tempTime;
RTC_DateTypeDef tempDate;
void clock_handle_event(MenuContext* ctx, ButtonEvent evt){
    MenuItem* cur = ctx ->current;
    if (!(ctx->edit_mode)) {
        switch (evt) {
            case BTN_LEFT:
                action_menu();
                break;
            case BTN_ENTER:
                ctx->edit_mode=1;
                HAL_RTC_GetDate(&hrtc, &tempDate, RTC_FORMAT_BIN);
                HAL_RTC_GetTime(&hrtc, &tempTime, RTC_FORMAT_BIN);
                break;
            default:
                return;
        }
        return;    
    }
    if (ctx->edit_mode) {
        switch(evt){
        case BTN_ENTER:
            HAL_RTC_SetDate(&hrtc, &tempDate, RTC_FORMAT_BIN);
            HAL_RTC_SetTime(&hrtc, &tempTime, RTC_FORMAT_BIN);
            ctx->edit_mode=0;
            break;
        case BTN_UP:
            cur->on_change(1);
            break;
        case BTN_DOWN:
            cur->on_change(-1);
            break;
        case BTN_LEFT:
            cur->selected_index = (cur->selected_index-1+7)%7;
            break;
        case BTN_RIGHT:
            cur->selected_index = (cur->selected_index+1)%7;
            break;
        default:
            return;
        }
    }
}

////
//// 傳感器狀態機(只有返回功能)
////
void engine_handle_event(MenuContext* ctx, ButtonEvent evt){
    switch (evt) {
        case BTN_LEFT:
            action_menu();
            break;
    }
    return;
}

////
//// 舵機狀態機
////
void servo_handle_event(MenuContext* ctx, ButtonEvent evt){
    MenuItem* cur = ctx ->current;
    if (!(ctx->edit_mode)) {
        switch (evt) {
            case BTN_LEFT:
                action_menu();
                break;
            case BTN_ENTER:
                ctx->edit_mode=1;
                break;
            default:
                return;
        }
        return;    
    }
    if (ctx->edit_mode) {
        switch(evt){
        case BTN_ENTER:
            ctx->edit_mode=0;
            break;
        case BTN_UP:
            cur->on_change(1);
            break;
        case BTN_DOWN:
            cur->on_change(-1);
            break;
        case BTN_LEFT:
            cur->selected_index = (cur->selected_index-1+4)%4;
            break;
        case BTN_RIGHT:
            cur->selected_index = (cur->selected_index+1)%4;
            break;
        default:
            return;
        }
    }
}