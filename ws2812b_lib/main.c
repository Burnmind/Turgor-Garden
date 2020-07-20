/*
 * ws2812b_lib.c
 *
 * Created: 15.12.2019 17:12:53
 * Author : Burnmind
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <math.h>

#define LED_QUANTITY 150
#define LED_IN_SEED 40

#define MOOVE_SPEED 40

#define START_R 0xff
#define START_G 0x00
#define START_B 0x00
#define END_R 0x00
#define END_G 0x00
#define END_B 0xff

#define COMPRESS_VALUE 35.0

#define ClearOutBit PORTC &= ~(1<<1)
#define SetOutBit   PORTC |= (1<<1)

void Set0(void)
{
	SetOutBit;
	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	ClearOutBit;
}

void Set1(void)
{
	SetOutBit;
	asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
	ClearOutBit;
}

unsigned long int ledValues[LED_QUANTITY];

void setLedValues(void)
{
	unsigned long int a;
	unsigned int j, i;

	for (j = 0; j < LED_QUANTITY; j++)
	{
		a = 0x1000000;

		for (i = 0; i < 24; i++)
		{
			a = a >> 1;
			if ((ledValues[j]&a) == 0x00000000) {
				Set0();
			} else {
				Set1();
			}
		}
	}
}

float map(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ¬озвращает рандомное число от -1.0 до 1.0
float randomVector()
{
	int randomIntValue = random()%100;
	float randomValue  = (float)randomIntValue;

	return map(randomValue, 0.0, 100.0, -1.0, 1.0);
}

float prevVector, nextVector;

//ѕосле каждой обработки сегмента смещаемс€ к следующему
void moveVectors(void)
{
	prevVector = nextVector;
	nextVector =  randomVector();
}

float abcisePosition = 0.0;
float interpolatedPoint = 0.0;

void countNextColorPosition(void)
{				   
	float firstPoint  = prevVector*abcisePosition;
	float secondPoint = nextVector*(abcisePosition - 1.0);
	interpolatedPoint = (2.0*(firstPoint - secondPoint)*powf(abcisePosition, 4.0) - (3.0*firstPoint - 5.0*secondPoint)*powf(abcisePosition, 3.0) - 3.0*secondPoint*powf(abcisePosition, 2.0) + firstPoint*abcisePosition);
	interpolatedPoint = (expf(interpolatedPoint*COMPRESS_VALUE)-expf(-1*interpolatedPoint*COMPRESS_VALUE))/(expf(interpolatedPoint*COMPRESS_VALUE)+expf(-1*interpolatedPoint*COMPRESS_VALUE));

	abcisePosition += 1.0/LED_IN_SEED;

	if (abcisePosition > 1.0)
	{
		abcisePosition = 0.0;
		moveVectors();
	}
}

unsigned long int getRed()
{
	long int intervalColorR  = (long int)(END_R - START_R);
	return (unsigned long int)((float)(intervalColorR*map(interpolatedPoint, -1.0, 1.0, 0.0, 1.0)) + START_R)*0x100;
}

unsigned long int getGreen()
{
	long int intervalColorG  = (long int)(END_G - START_G);
	return (unsigned long int)((float)(intervalColorG*map(interpolatedPoint, -1.0, 1.0, 0.0, 1.0)) + START_G)*0x10000;
}

unsigned long int getBlue() {
	long int intervalColorB  = (long int)(END_B - START_B);
	return (unsigned long int)((float)(intervalColorB*map(interpolatedPoint, -1.0, 1.0, 0.0, 1.0)) + START_B);
}

// сдвиг вправо с рассчетом нового элемента
void moveArray(void) {
	unsigned long int newElement;
	newElement = getGreen() + getRed() + getBlue();
	for (int i = LED_QUANTITY - 1; i > 0; i--)
	{
		ledValues[i] = ledValues[i-1];
	}

	ledValues[0] = newElement;
}

int main(void)
{
	DDRC |= (1<<1);
	prevVector = randomVector();
	nextVector = randomVector();

	for (int k = 0; k < 300; k++)
	{
		for (int i = 0; i < 24; i++)
		{
			Set0();
		}
	}

	//инициализируем ленту черным цветом
	for (int j = 0; j < LED_QUANTITY; j++)
	{
		ledValues[j] = 0x000000;
	}
	
    while (1)
    {
		setLedValues();
		countNextColorPosition();
		moveArray();

		_delay_ms(MOOVE_SPEED);
    }
}

