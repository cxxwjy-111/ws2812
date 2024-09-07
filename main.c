#include "Config.h"
#include "GPIO.h"
#include "Delay.h"
#include "WS2812B.h"
#include "stdlib.h"
#include "Exti.h"
#include "UART.h"

#define TOTAL_LIGHT_COUNT 16

/******************** INT???? ********************/
void	Exti_config(void)
{
    EXTI_InitTypeDef	Exti_InitStructure;							//??????

    Exti_InitStructure.EXTI_Mode      = EXT_MODE_RiseFall;//????,   EXT_MODE_RiseFall,EXT_MODE_Fall
    Ext_Inilize(EXT_INT0, &Exti_InitStructure);				//?????
    NVIC_INT0_Init(ENABLE, Priority_0);		//?????, ENABLE/DISABLE; ?????(?????) Priority_0,Priority_1,Priority_2,Priority_3
}

void UART_config(void) {
    // >>> ??????? NVIC.c, UART.c, UART_Isr.c <<<
    COMx_InitDefine		COMx_InitStructure;					//??????
    COMx_InitStructure.UART_Mode      = UART_8bit_BRTx;	//??, UART_ShiftRight,UART_8bit_BRTx,UART_9bit,UART_9bit_BRTx
    COMx_InitStructure.UART_BRT_Use   = BRT_Timer1;			//????????????, BRT_Timer1, BRT_Timer2 (???: ????2??????BRT_Timer2)
    COMx_InitStructure.UART_BaudRate  = 115200ul;			//??????, ??? 110 ~ 115200
    COMx_InitStructure.UART_RxEnable  = ENABLE;				//????????,   ENABLE??DISABLE
    COMx_InitStructure.BaudRateDouble = DISABLE;			//????????, ENABLE??DISABLE
    UART_Configuration(UART1, &COMx_InitStructure);		//?????????1 UART1,UART2,UART3,UART4

    NVIC_UART1_Init(ENABLE,Priority_1);		//?????, ENABLE/DISABLE; ?????(?????) Priority_0,Priority_1,Priority_2,Priority_3
    UART1_SW(UART1_SW_P30_P31);		// ???????, UART1_SW_P30_P31,UART1_SW_P36_P37,UART1_SW_P16_P17,UART1_SW_P43_P44
}

u8 cur_mode = 0;
u8 pre_mode = 0;

u8 is_mode_changed() {
    return cur_mode != pre_mode;
}

void light_on()
{
    u8 R, G, B;
    u8 n;

    // ???????
    R = 0x56, G = 0x12, B = 0x03;

    for (n = 0; n <= TOTAL_LIGHT_COUNT; n++)
    {

        // ?????n????
        //        WS_light_on_range_RGB(0, n, R, G, B);

        // ??????n????
        WS_light_on_RGB(n, R, G, B);

        delay_ms(40);

        if (is_mode_changed()) {
            pre_mode = cur_mode;
            break;
        }

    }

    // ???????
    WS_light_off_all();
}

/**
HSV??????? https://robot.czxy.com/docs/opencv/02_opencv_advance1/02_hsv/
H(hue) 				???	 [0, 360)  ???????????????????????? ????????????
S(saturation) ??????0%-100%????????????????????????????????????????????
V(Value??			????  ??0%-100%?????????????,??????? 5%???????
*/

void light_breathe(double init_value)
{
    static double step = 2;
    static double hue = 0.0;
    static double saturation = 1.0;
    double value = init_value;
    u8 n = 100;

    u8 R, G, B;
    // ?????????
    HSVtoRGB(hue, saturation, value, &R, &G, &B);

    // ?????n????
    WS_light_on_range_RGB(0, n, R, G, B);

    delay_ms(5);

    hue += step;
    if (hue >= 360)
    {
        step = -2;
    }
    else if (hue <= 0)
    {
        step = 2;
    }

    		// saturation += (step * 0.01);
    		// if (saturation >= 1.0){
    		// 	step = -1;
    		// }else if (saturation <= 0){
    		// 	step = 1;
    		// }

       value += (step * 0.01);
       if (value >= 1.0){
           step = -1;
       }
       else if (value <= 0) {
           step = 1;
       }


    if (is_mode_changed()) {
        pre_mode = cur_mode;
    }
}

void light_shoot(u8 delay)
{
    static double hue = 0.0;
    double sat = 1.0;
    int i = 0;

    for (i = 0; i < (TOTAL_LIGHT_COUNT); i++)
    {
        WS_light_on_flowing_HSV(hue, sat, i, HALF_FORWARD);
        delay_ms(delay);

        if (is_mode_changed()) {
            pre_mode = cur_mode;
            break;
        }
    }

    hue += 20.0;

    for (i = (TOTAL_LIGHT_COUNT-1); i >=0 ; i--)
    {
        WS_light_on_flowing_HSV(hue, sat, i, HALF_BACKWARD);
        delay_ms(delay);

        if (is_mode_changed()) {
            pre_mode = cur_mode;
            break;
        }
    }

    hue += 20.0;

    if (hue >= 360) {
        hue = 0;
    }

}

void light_shoot1(u8 delay)              //?????
{
    static double hue = 0.0;
    double sat = 1.0;
    int i = 0;

    for (i =  0; i < (TOTAL_LIGHT_COUNT/2); i++)
    {
        WS_light_on_flowing_HSV(hue, sat, i, HALF_FORWARD);
        delay_ms(delay);

        if (is_mode_changed()) {
            pre_mode = cur_mode;
            break;
        }
    }

    hue += 10.0;

    for (i = (TOTAL_LIGHT_COUNT); i >=(TOTAL_LIGHT_COUNT/2) ; i--)
    {
        WS_light_on_flowing_HSV(hue, sat, i, HALF_BACKWARD);
        delay_ms(delay);

        if (is_mode_changed()) {
            pre_mode = cur_mode;
            break;
        }
    }

    hue += 10.0;

    if (hue >= 360) {
        hue = 0;
    }

}


void ext_int0_call() {

    if (P32) {
        ++cur_mode;
        cur_mode %= 9;

        printf("cur_mode:%d \n", (int)cur_mode);
    }

}

void main()
{

    P3_MODE_IO_PU(GPIO_Pin_2);

    Exti_config();

    UART_config();

    EA = 1;

    WS_init(TOTAL_LIGHT_COUNT);

    WS_Send_Reset();

    while (1)
    {

        switch(cur_mode) {
        case 0:
            // light_breathe(0.10);
            WS_light_on_RGB(0, 255, 0, 0);
            break;
        case 1:
            // light_breathe(0.20);
            WS_light_on_RGB(0, 0, 255, 0);
            break;
        case 2:
            // light_breathe(0.40);
            WS_light_on_RGB(0, 0, 0, 255);
            break;
        case 3:
            break;
        case 4:
            light_breathe(1.00);
            break;
        case 5:
            light_shoot(20);
            break;
        case 6:
            light_shoot1(20);
            break;
        case 7:
            light_shoot1(30);
            break;
        case 8:
            light_on();
            break;
        default:
            light_on();
            break;
        }
        pre_mode = cur_mode;
        delay_ms(20);
    }
}
