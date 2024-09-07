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
    GPIO_InitTypeDef	GPIO_InitStructure;		//�ṹ����
    GPIO_InitStructure.Pin  = WS_GPIO_PIN;		//ָ��Ҫ��ʼ����IO,
    GPIO_InitStructure.Mode = GPIO_PullUp;	//ָ��IO������������ʽ,GPIO_PullUp,GPIO_HighZ,GPIO_OUT_OD,GPIO_OUT_PP
    GPIO_Inilize(WS_GPIO, &GPIO_InitStructure);//��ʼ��
}


#define DALY_300NS()		 NOP6()
#define DALY_1080NS()		 NOP25()

//����0/1���ź�
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

HSV��ɫģʽ�� https://robot.czxy.com/docs/opencv/02_opencv_advance1/02_hsv/
H(hue) 				ɫ��	 [0, 360)  ɫ�ʵ���ò����ͼ�����ɫ������ ��Ȼ���������
S(saturation) ���Ͷȣ�0%-100%����ɫ����ǳ��Ũ�ȡ����޳̶ȡ����ɫ���Է�Ϊ��졢��졢ǳ��ȡ�
V(Value��			����  ��0%-100%����ʾ��ɫ��ǿ��,�����ȡ�
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

//��ʼ��
void WS_init(u16 count) {
    GPIO_config();
    total_count = count;
}

//������һ����
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

//����ָ����ʼ����λ�õĵ�
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


//������N����
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
��ˮ�ƣ�
ָ����ɫ�����Ͷȣ�
ָ����ȣ�ָ��ģʽ
*/
void WS_light_on_flowing_HSV(
	double hue, double sat, 
    u16 pos, 
	FLOWING_MODE mode // 0Ϊ����ǰ����1λ�������, 2Ϊ˫��ǰ����3Ϊ˫�����
){
    int i = 0, start_i, end_i;
    u32 color;
		int width = FLOWING_WIDTH;
	// ����mode��width���hsv��ת��RGB�浽����
    u32 arr[FLOWING_WIDTH] = {0}; // ��̬���������ڴ�ռ�

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

	// ѭ������
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
//    free(arr); // �ͷŶ�̬������ڴ�
}

//���� >= 50ms ������
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



