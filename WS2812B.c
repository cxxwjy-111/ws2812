#include "WS2812B.h"
#include <math.h>

u16 total_count;

void Delay100us(void)	//@24.000MHz
{
    unsigned char data i, j;

    i = 4;
    j = 27;
    do
    {
        while (--j);
    } while (--i);
}

static void GPIO_config(void) {
    GPIO_InitTypeDef	GPIO_InitStructure;		//结构定义
    GPIO_InitStructure.Pin  = WS_GPIO_PIN;		//指定要初始化的IO,
    GPIO_InitStructure.Mode = GPIO_PullUp;	//指定IO的输入或输出方式,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
    GPIO_Inilize(WS_GPIO, &GPIO_InitStructure);//初始化
}


#define DALY_300NS()		 NOP6()
#define DALY_1080NS()		 NOP25()

//传输0/1的信号
#define		RGB_Send1()		  \
    RGB_H;								\
    DALY_1080NS();				\
    RGB_L;								\
    DALY_300NS();					\

#define		RGB_Send0()			\
    RGB_H;								\
    DALY_300NS();					\
    RGB_L;								\
    DALY_1080NS();				\



#define SEND_AND_MOVE(c)			\
		if ((c) & 0x80){					\
			RGB_Send1();						\
		}else {										\
			RGB_Send0();						\
		}													\
		(c) <<= 1;								\


double fmax(double a, double b) {
    return a > b ? a : b;
}

double fmin(double a, double b) {
    return a < b ? a : b;
}

/**

HSV颜色模式： https://robot.czxy.com/docs/opencv/02_opencv_advance1/02_hsv/
H(hue) 				色相	 [0, 360)  色彩的相貌或者图像的颜色，比如 红橙黄绿青蓝紫
S(saturation) 饱和度（0%-100%）颜色的深浅、浓度、鲜艳程度。如红色可以分为深红、洋红、浅红等。
V(Value）			明度  （0%-100%）表示颜色的强度,明暗度。
*/
void HSVtoRGB(double h, double s, double v, u8* r, u8* g, u8* b) {
    double r1, g1, b1;

    double c = v * s;
    double x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    double m = v - c;

    h = fmod(h, 360.0); // Ensure h is in the range [0, 360)
    s = fmax(0.0, fmin(1.0, s)); // Clamp s to [0, 1]
    v = fmax(0.0, fmin(1.0, v)); // Clamp v to [0, 1]


    if (h >= 0 && h < 60) {
        r1 = c, g1 = x, b1 = 0;
    } else if (h >= 60 && h < 120) {
        r1 = x, g1 = c, b1 = 0;
    } else if (h >= 120 && h < 180) {
        r1 = 0, g1 = c, b1 = x;
    } else if (h >= 180 && h < 240) {
        r1 = 0, g1 = x, b1 = c;
    } else if (h >= 240 && h < 300) {
        r1 = x, g1 = 0, b1 = c;
    } else {
        r1 = c, g1 = 0, b1 = x;
    }

    *r = (u8)((r1 + m) * 255);
    *g = (u8)((g1 + m) * 255);
    *b = (u8)((b1 + m) * 255);
}

void HSVtoRGB24(double h, double s, double v, u32* rgb) {
    u8 r, g, b;
    HSVtoRGB(h, s, v, &r, &g, &b);
    *rgb = ((u32)r << 16) | ((u32)g << 8) | b;
}

//初始化
void WS_init(u16 count) {
    GPIO_config();
    total_count = count;
}

//点亮第一个灯
void WS_Send_RGB(u8 R, u8 G, u8 B) {

    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);
    SEND_AND_MOVE(G);


    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);
    SEND_AND_MOVE(R);


    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
    SEND_AND_MOVE(B);
}

//点亮指定开始结束位置的灯
void WS_light_on_range_RGB(u16 start,u16 end, u8 R, u8 G, u8 B) {
    int i;
    for(i = 0; i < total_count; i++) {
        if( i >= start && i < end ) {
            WS_Send_RGB(R, G, B);
        } else {
            WS_Send_RGB(0, 0, 0);
        }
    }
    WS_Send_Reset();
}


//点亮第N个灯
void WS_light_on_RGB(u16 n, u8 R, u8 G, u8 B) {
    int i;
    for(i = 0; i < total_count; i++) {
        if( i == n ) {
            WS_Send_RGB(R, G, B);
        } else {
            WS_Send_RGB(0, 0, 0);
        }
    }
    WS_Send_Reset();
}
#define PI	3.14159265

#define EXCHANGE_FUNC(input) 	(1 - cos(input * PI * 0.5))

/**
 * 
流水灯：
指定颜色，饱和度，
指定宽度，指定模式
*/
void WS_light_on_flowing_HSV(
	double hue, double sat, 
    u16 pos, 
	FLOWING_MODE mode // 0为单向前进，1位单向回流, 2为双向前进，3为双向回流
){
    int i = 0, start_i, end_i;
    u32 color;
		int width = FLOWING_WIDTH;
	// 根据mode和width算出hsv并转成RGB存到数组
    u32 arr[FLOWING_WIDTH] = {0}; // 动态分配数组内存空间

    if (mode == HALF_FORWARD) {
        // -->>O--------------------
        for (i = 0; i < width; i++) { // 0.1, 0.2, 0.3, 0.4...1.0
            HSVtoRGB24(hue, sat, EXCHANGE_FUNC(i * 1.0 / width), &arr[i]);
        }
        start_i = pos - width + 1;
        end_i = pos;
    }else if ( mode == HALF_BACKWARD) {
      
        // -------------------O<<--
        for (i = 0; i < width; i++) { // 1.0, 0.9, 0.8, 0.7...0.1
            HSVtoRGB24(hue, sat, EXCHANGE_FUNC(i * 1.0 / width), &arr[width - i - 1]);
        }
        start_i = pos;
        end_i = pos + width - 1;

    }else if ( mode == FULL) { // 9 -> 4, 8 -> 4, 7 -> 3
        // -->>O<<--------------
        int half_index = width / 2;
        for (i = 0; i < half_index; i++) {
            HSVtoRGB24(hue, sat, EXCHANGE_FUNC(i * 1.0 / width), &arr[i]);
        }
        for (i = half_index; i < width; i++) {
            HSVtoRGB24(hue, sat, EXCHANGE_FUNC(i * 1.0 / width), &arr[width - i - 1]);
        }
        start_i = pos - half_index;
        end_i = pos + half_index;
    }

	// 循环发送
    for(i = 0; i < total_count; i++) {
        if( i >= start_i && i <= end_i ) {
            color = arr[i - start_i];
//						color = 0xFF00FF;
            WS_Send_RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
            // WS_Send_RGB(R, G, B);
        } else {
            WS_Send_RGB(0, 0, 0);
        }
    }

    WS_Send_Reset();
	// Reset
//    free(arr); // 释放动态分配的内存
}

//结束 >= 50ms 的拉低
void WS_Send_Reset() {
    RGB_L;
    Delay100us();
}


void WS_light_off_all() {
    u16 cnt = total_count;
    while(cnt--) {
        WS_Send_RGB(0, 0, 0);
    }
    WS_Send_Reset();
}



