#include "scroll.h"

char log_buffer[MAX_LINES][MAX_LINE_LEN];
uint8_t log_count = 0;   // 目前總共有幾行

// 新增一行，超過 MAX_LINES 就捨棄最舊的
void log_print(const char *str)
{
    if (log_count < MAX_LINES) {
        strncpy(log_buffer[log_count], str, MAX_LINE_LEN - 1);
        log_buffer[log_count][MAX_LINE_LEN - 1] = '\0';
        log_count++;
    } else {
        // 全部往上移一行
        for (uint8_t i = 0; i < MAX_LINES - 1; i++) {
            memcpy(log_buffer[i], log_buffer[i + 1], MAX_LINE_LEN);
        }
        strncpy(log_buffer[MAX_LINES - 1], str, MAX_LINE_LEN - 1);
        log_buffer[MAX_LINES - 1][MAX_LINE_LEN - 1] = '\0';
    }
}

// 顯示最後 VISIBLE_LINES 行
void log_display(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);
    u8g2_SetFont(u8g2, u8g2_font_tom_thumb_4x6_tr);   // 4x6 字體

    // 決定從哪行開始顯示
    uint8_t start = (log_count > VISIBLE_LINES) ? (log_count - VISIBLE_LINES) : 0;

    for (uint8_t i = 0; i < VISIBLE_LINES && (start + i) < log_count; i++) {
        u8g2_DrawStr(u8g2, 0, (i + 1) * LINE_HEIGHT, log_buffer[start + i]);
    }

    u8g2_SendBuffer(u8g2);
}