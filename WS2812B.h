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

//������N����, ����Ϩ��
void WS_light_on_RGB(u16 n, u8 R, u8 G, u8 B);

//����ָ����ʼ����λ�õĵƣ�����Ϩ��, ��ΧΪ[start, end) ����������λ��
void WS_light_on_range_RGB(u16 start, u16 end, u8 R, u8 G, u8 B);

// �ر����е�
void WS_light_off_all();

typedef enum {
	HALF_FORWARD,	// ����ǰ��
	HALF_BACKWARD,	// �������
	FULL,	// ˫��
} FLOWING_MODE;

// ��ˮ�ƣ�
// ָ����ɫ�����Ͷȣ�
// ָ����ȣ�ָ��ģʽ
void WS_light_on_flowing_HSV(
	double hue, double sat,
	u16 pos,
	FLOWING_MODE mode 
);
		

//����
void WS_Send_Reset();




#endif