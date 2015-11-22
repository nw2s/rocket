/*
This sketch is demonstrating loading images from an array.
Use BMP24toILI565Array to generate .h files from your .bmp images.
Then include those .h files in your sketch and use the array name
(which is same as filename) in drawImage call.
*/

#include "SPI.h"
#include "ILI9341_due_config.h"
#include "ILI9341_due.h"
#include "launchcontrol2.h"
#include "numbers.h"
//#include "SD.h"
#include "SdFat.h"
#include "SdFatUtil.h"
#include "rocket.h"


/* PIN DEFINITIONS */
#define SD_CS 4		// Chip Select for LCD
#define LCD_CS 5	// Chip Select for LCD
#define LCD_DC 6	// Command/Data for LCD
#define LCD_RST 13	// Reset for LCD

#define SWITCH_SOURCE 26
#define SWITCH_LAUNCH 24
#define SWITCH_RECORD 22

/* DISPLAY CONSTANTS */
#define NUMBER_WIDTH 13
#define NUMBER_HEIGHT 17

#define MAX_ALTITUDE_Y 4
#define ACCELERATION_Y 28
#define LATITUDE_Y 52
#define LONGITUDE_Y 76
#define ALTITUDE_Y 100
#define DIGIT0_X 283

ILI9341_due tft(LCD_CS, LCD_DC, LCD_RST);

/* Represents possible values for launch control arming */
enum armState {
	
	ARM_SAFE,
	ARM_ARMED,
	ARM_FIRING
};

/* Represents possible values for generic ready state displays */
enum readyState {
	
	READY_READY,
	READY_WAITING,
	READY_ERROR
};


/* Static state management */
float maxaltitude;
float altitude;
float acceleration;
float latitude;
float longitude;

bool recordSwitchLastState;
bool launchSwitchLastState;
bool recording;
bool launching;

SdFat sd;
SdFile dataFile;

uint32_t lastTime;
uint32_t lastLaunchTime;
uint32_t lastVehicleTime;
uint32_t launchStartTime;
uint8_t launchSafetyState;


/* Opens a file for recording data with a consecutive file name */
void startLogFile()
{
	char filename[7] = "00.txt";
	
    while (sd.exists(filename)) 
	{
		if (filename[1] != '9') 
		{
        	filename[1]++;
      	} 
		else if (filename[0] != '9') 
		{
        	filename[1] = '0';        
			filename[0]++;
      	} 
		else 
		{
			recording = false;
			setVehicleRecord(READY_ERROR);
			filename[0] = '\0';
      	}
    }

	if (filename[0] != '\0')
	{
		if (dataFile.open(filename, O_CREAT | O_WRITE | O_EXCL))
		{
			recording = true;
			setVehicleRecord(READY_READY);
		}
		else
		{
			recording = false;
			setVehicleRecord(READY_ERROR);
		}
	}
}



/* Finds a digit value at a position in a floating point number */
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

/* Renders a digit on screen */
void displayDigit(uint8_t row, uint8_t column, uint8_t value)
{
	int y = MAX_ALTITUDE_Y + (row * 24);
	int x = DIGIT0_X - (column * 13);
	
	tft.drawImage(NUMBERS[value], x, y, 13, 17);
}

/* Updates the max altitude display */
void setMaxAltitude(float alt)
{
	for (int i = 0; i < 6; i++)
	{
		displayDigit(0, i, findDigit(i, alt));
	}
}

/* Updates the max acceleration value */
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

/* Updates the latitude display */
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

/* Updates the longitude display */
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

/* Updates the current altitude display */
void setAltitude(float alt)
{
	for (int i = 0; i < 6; i++)
	{
		displayDigit(4, i, findDigit(i, alt));
	}
}

/* Updates the arm state display */
void setArmState(int state)
{
	switch(state)
	{
		case ARM_SAFE:			
		
			tft.fillRect(2, 143, 119, 237, ILI9341_GREEN);
			break;

		case ARM_ARMED:			
	
			tft.fillRect(2, 143, 119, 237, ILI9341_YELLOW);
			break;

		case ARM_FIRING:			
	
			tft.fillRect(2, 143, 119, 237, ILI9341_RED);
			break;
	}
}

/* Updates the display state of the launch connector */
void setLaunchConnect(int state)
{
	//TODO: Timeout after 1 second
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
	/* Initialize Serial ports */
	/* Serial0 = console output */
	/* Serial1 = radio intput/output */
	
	Serial.begin(9600);
	Serial1.begin(9600);	
	
	altitude = 0.0f;
	acceleration = 1.0;
	latitude = 0.0f;
	longitude = 0.0f;

	/* 26 = 0V, 24 = PULLUP, 22 = PULLUP */
	pinMode(SWITCH_SOURCE, OUTPUT);
	digitalWrite(SWITCH_SOURCE, LOW);
	
	pinMode(SWITCH_RECORD, INPUT_PULLUP);
	pinMode(SWITCH_LAUNCH, INPUT_PULLUP);

	/* switch states are active low */
	recordSwitchLastState = HIGH;
	launchSwitchLastState = HIGH;
	
	recording = false;
	launching = false;

	lastTime = millis();
	lastLaunchTime = 0;
	lastVehicleTime = 0;

	/* TFT  has to be set up after SD */
	tft.begin();

	/* Start up the SD connection before TFT */
	bool sdBegin = sd.begin(SD_CS, SPI_HALF_SPEED);
	
	/* Set to landscape */
	tft.setRotation(iliRotation270);

	tft.fillScreen(ILI9341_BLACK);
	tft.drawImage(LAUNCHCONTROL2_IMAGE, 0, 0, 320, 240);

	setMaxAltitude(0.0f);

	setArmState(ARM_SAFE);
	setLaunchConnect(READY_WAITING);
	setVehicleConnect(READY_WAITING);

	if (!sdBegin)
	{
		setVehicleRecord(READY_ERROR);
	}
	else
	{
		setVehicleRecord(READY_WAITING);
	}
}

void loop()
{	
	lastTime = millis();
	
	/* Update switch states and do whatever we need to */
	int newRecordState = digitalRead(SWITCH_RECORD);
	int newLaunchState = digitalRead(SWITCH_LAUNCH);
	
	/* Start/Stop recording */
	if (recordSwitchLastState != newRecordState)
	{
		if (newRecordState == LOW)
		{
			/* If low, start recording */
			startLogFile();
		}
		else
		{
			/* If high, stop recording */
			dataFile.close();
			recording = false;
			setVehicleRecord(READY_WAITING);
		}
		
		recordSwitchLastState = newRecordState;
	}
	
	/* If we are armed and the launch switch goes from high to low, then send fire signal */
	if ((launchSafetyState == ARM_ARMED) && (launchSwitchLastState == HIGH) && (newLaunchState == LOW))
	{
		Serial1.println(INDICATOR_IGNITE);
		
		launchSwitchLastState = newLaunchState;
	}
	
	/* If the launchState ever turns off, make sure we recognize that */
	if (newLaunchState == HIGH)
	{
		launchSwitchLastState = newLaunchState;
	}


	/* Pull data from radio module */
	while (Serial1.available() > 0)
	{
		char cmd;
		char* data;
		char buffer[64];
		uint8_t i = 0;
		
		do
		{
			buffer[i] = (char)Serial1.read();
			i++;
		} 
		while ((buffer[i - 1] != '\n') && (Serial1.available()));  

		buffer[i - 1] = '\0';
		cmd = buffer[0];
		data = &buffer[1];

		Serial.print((int)cmd);
		Serial.print(' ');
		Serial.println((int)data[0]);

		switch(cmd)
		{
			case INDICATOR_ALTITUDE:

				setVehicleConnect(READY_READY);
				lastVehicleTime = lastTime;
				altitude = atof(data);
				
				if (altitude < 0.0f)
				{
					altitude = 1.0f;
				}
				
				if (altitude > maxaltitude) 
				{
					maxaltitude = altitude;
				}
				
				break;

			case INDICATOR_LATITUDE:

				latitude = atof(data);
				break;

			case INDICATOR_LONGITUDE:

				longitude = -1.0f * atof(data);
				break;
				
			case INDICATOR_LAUNCHARM:
			
				setLaunchConnect(READY_READY);
				
				lastLaunchTime = lastTime;
				
				/* Note: We HAVE to receive the state when the launch switch is off (active low!) */
				/* before we'll say that we are armed! This should help prevent accidents */
				if ((data[0] == '1') && (digitalRead(SWITCH_LAUNCH) == HIGH))
				{
					setArmState(ARM_ARMED);
					launchSafetyState = ARM_ARMED;
				}
				
				/* Ignition attempting */
				if (data[0] == '2')
				{
					setArmState(ARM_FIRING);
					launchSafetyState = ARM_FIRING;
				}
				
				/* Starting sequence over */
				if (data[0] == '0')
				{
					setArmState(ARM_SAFE);
					launchSafetyState = ARM_SAFE;
				}
				
				break;
		}
		
		if (recording)
		{
			dataFile.print(lastTime);
			dataFile.print('\t');
			dataFile.print(cmd);
			dataFile.print('\t');
			dataFile.println(data);
		}
	}

	/* Update the display every 100ms */
	if (lastTime % 100 == 0)
	{		
		setMaxAltitude(maxaltitude);
		setAltitude(altitude);
		setMaxAcceleration(acceleration);
		setLatitude(latitude);
		setLongitude(longitude);
	}
	
	/* If either of the devices haven't pinged in 1s, flag as disconnected */
	if (lastTime > (lastVehicleTime + 5000))
	{
		setVehicleConnect(READY_ERROR);
	}
	else if (lastTime > (lastVehicleTime + 1000))
	{
		setVehicleConnect(READY_WAITING);
	}
	
	if (lastTime > (lastLaunchTime + 5000))
	{
		setLaunchConnect(READY_ERROR);
	}
	else if (lastTime > (lastLaunchTime + 1000))
	{
		setLaunchConnect(READY_WAITING);
	}	
}
