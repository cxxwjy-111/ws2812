#ifndef __WS2812B_H__
#define __WS2812B_H__

#include "Config.h"
#include "GPIO.h"

#define		WS_GPIO			GPIO_P3
#define		WS_GPIO_PIN		GPIO_Pin_5

#define		RGB_H		P35 = 1
#define 	RGB_L		P35 = 0

#define 	FLOWING_WIDTH		20

void HSVtoRGB(double h, double s, double v, u8* r, u8* g, u8* b);
void HSVtoRGB24(double h, double s, double v, u32* rgb);

void WS_init(u16 total_count);

//点亮第N个灯, 其他熄灭
void WS_light_on_RGB(u16 n, u8 R, u8 G, u8 B);

//点亮指定开始结束位置的灯，其他熄灭, 范围为[start, end) 不包含结束位置
void WS_light_on_range_RGB(u16 start, u16 end, u8 R, u8 G, u8 B);

// 关闭所有灯
void WS_light_off_all();

typedef enum {
	HALF_FORWARD,	// 单向前进
	HALF_BACKWARD,	// 单向后退
	FULL,	// 双向
} FLOWING_MODE;

// 流水灯：
// 指定颜色，饱和度，
// 指定宽度，指定模式
void WS_light_on_flowing_HSV(
	double hue, double sat,
	u16 pos,
	FLOWING_MODE mode 
);
		

//结束
void WS_Send_Reset();




#endif