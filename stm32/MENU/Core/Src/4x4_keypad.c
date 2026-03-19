#include "4x4_keypad.h"
#include "cmsis_os2.h"
#include "main.h"
#include <stdint.h>

volatile char keypad_key_pressed = 0;  // 保存檢測到的按鍵

static const char keymap[4][4] = {
        {'1','2','3','A'},
        {'4','5','6','B'},
        {'7','8','9','C'},
        {'*','0','#','D'}
    };
static const uint16_t row_pins[4] = {key_R1_Pin, key_R2_Pin, key_R3_Pin, key_R4_Pin};
static const uint16_t col_pins[4] = {key_C1_Pin, key_C2_Pin, key_C3_Pin, key_C4_Pin};
char Keypad_GetKey(void) {
    for(int row=0; row<4; row++)
    {
        // 所有 row 設高
        GPIOB->ODR |= (ALL_ROWS);
        osDelay(2);

        // 當前 row 拉低
        GPIOB->ODR &= ~row_pins[row];
        osDelay(2);

        for(int col=0; col<4; col++)
        {
            if(!(GPIOC->IDR & col_pins[col]))
            {
                osDelay(20); // debounce
                while(!(GPIOC->IDR & col_pins[col]))
                {
                    osDelay(1);
                }
                GPIOB->ODR &= ~ALL_ROWS;
                return keymap[row][col];
            }
        }
        GPIOB->ODR &= ~ALL_ROWS;
    }

    return 0;
}

char Keypad_GetKey_IT(uint16_t GPIO_Pin) {
    int col_idx = __builtin_ctz(GPIO_Pin);
    
    for (int row = 0; row < 4; row++) {
        // 所有 Row 拉高（讓其他 Row 不干擾）
        GPIOB->ODR |= ALL_ROWS;
        // 只有當前 Row 拉低
        GPIOB->ODR &= ~row_pins[row];
        
        __NOP(); __NOP(); __NOP(); __NOP();
        
        // 如果這個 Col 還是 LOW，就是這個 Row
        if (!(GPIOC->IDR & GPIO_Pin)) {
            GPIOB->ODR &= ~ALL_ROWS;  // 恢復全部 LOW
            return keymap[row][col_idx];
        }
    }
    
    GPIOB->ODR &= ~ALL_ROWS;  // 恢復全部 LOW
    return 0;
}