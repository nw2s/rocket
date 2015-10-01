/*
This sketch is demonstrating loading images from an array.
Use BMP24toILI565Array to generate .h files from your .bmp images.
Then include those .h files in your sketch and use the array name
(which is same as filename) in drawImage call.
*/

#include "SPI.h"
#include "ILI9341_due_config.h"
#include "ILI9341_due.h"
#include "alert.h"
#include "info.h"
#include "close.h"
#include "launchcontrol2.h"
#include "numbers.h"

// #define LCD_CS 10	// Chip Select for LCD
// #define LCD_DC 9	// Command/Data for LCD
// #define LCD_RST 8	// Reset for LCD

#define LCD_CS 5	// Chip Select for LCD
#define LCD_DC 6	// Command/Data for LCD
#define LCD_RST 13	// Reset for LCD

#define NUMBER_WIDTH 13
#define NUMBER_HEIGHT 17

#define MAX_ALTITUDE_Y 4
#define ACCELERATION_Y 28
#define LATITUDE_Y 52
#define LONGITUDE_Y 76
#define ALTITUDE_Y 100
#define DIGIT0_X 283

// armed button
// 2x143 - 119x237
//
//
// launch 298x148 - 311 x 161
//
// vehicle 180
//
// record 212


ILI9341_due tft(LCD_CS, LCD_DC, LCD_RST);

float maxaltitude;
float altitude;
float acceleration;
float latitude;
float longitude;

uint8_t findDigit(int8_t digit, float value)
{
	/* Shift the number to the point where 1's digit is what we want */
	if (digit > 0)
	{
		for (int i = 0; i < digit; i++)
		{
			value /= 10;
		}
	}
	else
	{
		for (int i = 0; i > digit; i--)
		{
			value *= 10;
		}
	}
	
	/* Mod by 10 to get the single digit */
	return (int)value % 10;
}

void displayDigit(uint8_t row, uint8_t column, uint8_t value)
{
	int y = MAX_ALTITUDE_Y + (row * 24);
	int x = DIGIT0_X - (column * 13);
	
	tft.drawImage(NUMBERS[value], x, y, 13, 17);
}

void setMaxAltitude(float alt)
{
	for (int i = 0; i < 6; i++)
	{
		displayDigit(0, i, findDigit(i, alt));
	}
}

void setMaxAcceleration(float g)
{
	for (int i = -2; i < 0; i++)
	{
		displayDigit(1, i + 2, findDigit(i, g));
	}

	for (int i = 0; i < 2; i++)
	{
		displayDigit(1, i + 3, findDigit(i, g));
	}
}

void setLatitude(float lat)
{
	for (int i = -6; i < 0; i++)
	{
		displayDigit(2, i + 6, findDigit(i, lat));
	}

	for (int i = 0; i < 2; i++)
	{
		displayDigit(2, i + 7, findDigit(i, lat));
	}	
}

void setLongitude(float lon)
{
	for (int i = -6; i < 0; i++)
	{
		displayDigit(3, i + 6, findDigit(i, lon));
	}

	for (int i = 0; i < 2; i++)
	{
		displayDigit(3, i + 7, findDigit(i, lon));
	}
}

void setAltitude(float alt)
{
	for (int i = 0; i < 6; i++)
	{
		displayDigit(4, i, findDigit(i, alt));
	}
}

enum armState {
	
	ARM_SAFE,
	ARM_READY,
	ARM_ARMED
};

void setArmState(int state)
{
	switch(state)
	{
		case ARM_SAFE:			
		
			tft.fillRect(2, 143, 119, 237, ILI9341_GREEN);
			break;

		case ARM_READY:			
	
			tft.fillRect(2, 143, 119, 237, ILI9341_YELLOW);
			break;

		case ARM_ARMED:			
	
			tft.fillRect(2, 143, 119, 237, ILI9341_RED);
			break;
	}
}

enum readyState {
	
	READY_READY,
	READY_WAITING,
	READY_ERROR
};

void setLaunchConnect(int state)
{
	switch(state)
	{
		case READY_READY:			
		
			tft.fillRect(298, 148, 14, 14, ILI9341_GREEN);
			break;

		case READY_WAITING:			
	
			tft.fillRect(298, 148, 14, 14, ILI9341_YELLOW);
			break;

		case READY_ERROR:			
	
			tft.fillRect(298, 148, 14, 14, ILI9341_RED);
			break;
	}
}

void setVehicleConnect(int state)
{
	switch(state)
	{
		case READY_READY:			
		
			tft.fillRect(298, 180, 14, 14, ILI9341_GREEN);
			break;

		case READY_WAITING:			
	
			tft.fillRect(298, 180, 14, 14, ILI9341_YELLOW);
			break;

		case READY_ERROR:			
	
			tft.fillRect(298, 180, 14, 14, ILI9341_RED);
			break;
	}
}

void setVehicleRecord(int state)
{
	switch(state)
	{
		case READY_READY:			
		
			tft.fillRect(298, 212, 14, 14, ILI9341_GREEN);
			break;

		case READY_WAITING:			
	
			tft.fillRect(298, 212, 14, 14, ILI9341_YELLOW);
			break;

		case READY_ERROR:			
	
			tft.fillRect(298, 212, 14, 14, ILI9341_RED);
			break;
	}
}

void setup()
{
	altitude = 123.456f;
	acceleration = 1.0;
	latitude = 40.7127;
	longitude = 74.0059;
	
	Serial.begin(9600);
	tft.begin();
	
	/* Set to landscape */
	tft.setRotation(iliRotation270);
		
	tft.fillScreen(ILI9341_BLACK);
	tft.drawImage(LAUNCHCONTROL2_IMAGE, 0, 0, 320, 240);

	setMaxAltitude(0.0f);
	
	setArmState(ARM_SAFE);
	
	setLaunchConnect(READY_WAITING);
	setVehicleConnect(READY_ERROR);
	setVehicleRecord(READY_READY);
}

void loop()
{

	if (millis() % 100 == 0)
	{
		setMaxAltitude(altitude);	
		setAltitude(altitude);	
		setMaxAcceleration(acceleration);
		setLatitude(latitude);
		setLongitude(longitude);


		altitude += 12.4;
		if (altitude > 10000) altitude = 0.0f;
		
		acceleration += 0.01;
		if (acceleration > 29) acceleration = 0.0f;
		
		longitude += 0.0000115;
		if (longitude > 90) longitude = 0.0f;
		
		latitude += 0.000011;
		if (latitude > 90) latitude = 0.0f;
		
	}
}
