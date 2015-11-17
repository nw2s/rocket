

#include <SPI.h>
#include <RH_RF69.h>
#include "rocket.h"
#include <Wire.h>
#include "SparkFunMPL3115A2.h"
#include "LIS331.h"
#include "Adafruit_GPS.h"
#include <SoftwareSerial.h>
#include "SPIFlash.h"

#define DEBUG false
#define FLASH_SS 23

/* Singleton instances */
RH_RF69 rf69(4, 2); 
MPL3115A2 altimeter;
LIS331 lis;
Adafruit_GPS GPS(&Serial1);
HardwareSerial mySerial = Serial1;
SPIFlash flash(FLASH_SS, 0xEF30);

uint32_t timer = 0;

DataTelemetry telemetry;

void debug()
{
	Serial.print("Temperature:   ");
	Serial.println(telemetry.temperature);
	Serial.print("Altitude:      ");
	Serial.println(telemetry.altitude);

	Serial.print("Latitude:      ");
	Serial.println(telemetry.latitudeDegrees, 11);
	Serial.print("Longitude:     ");
	Serial.println(telemetry.longitudeDegrees, 11);
	Serial.print("Altitude GPS:  ");
	Serial.println(telemetry.altitudeGPS);
	Serial.print("Fix:           ");
	Serial.println(telemetry.fix);
	Serial.print("Fix Quality:   ");
	Serial.println(telemetry.fixQuality);
	Serial.print("Satellites:   ");
	Serial.println(telemetry.satellites);
	Serial.print("Time:         ");
	Serial.print(telemetry.hour);
	Serial.print(":");
	Serial.print(telemetry.minute);
	Serial.print(":");
	Serial.print(telemetry.seconds);
	Serial.print(".");
	Serial.println(telemetry.milliseconds);


    Serial.print("X Value:       ");
    Serial.print(telemetry.accelX);
    Serial.println(" mG");
    Serial.print("Y Value:       ");
    Serial.print(telemetry.accelY);
    Serial.println(" mG");
    Serial.print("Z Value:       ");
    Serial.print(telemetry.accelZ);
    Serial.println(" mG");
    Serial.println("\n\n\n--------------------------------");
}


void setup() 
{
	Serial.begin(9600);

	/* Setup I2C */
    Wire.begin();

	/* Initialize the radio */
	if (!rf69.init())
	{
		/* init failed */
		while(1);
	}

	/* Operating in amateur range here... get your license! */
	if (!rf69.setFrequency(431.120))
	{
		/* setFrequency failed */
		while(1);
	}

	/* Set dBm up to 20 */
	rf69.setTxPower(20);

	/* The encryption key has to be the same on all nodes */
	uint8_t key[] = ENCRYPTION_KEY;
	rf69.setEncryptionKey(key);

	/* Bring up the altimeter */
	altimeter.begin();
	altimeter.setModeAltimeter();
	altimeter.setOversampleRate(7);
	altimeter.enableEventFlags();

	/* Set up accellerometer */
    lis.setPowerStatus(LR_POWER_NORM);
    lis.setXEnable(true);
    lis.setYEnable(true);
    lis.setZEnable(true);	

	/* Set up the LED to be blinky when we send data */
    pinMode(15, OUTPUT);
	
	/* GPS Stuff */
    GPS.begin(4800);
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  
    GPS.sendCommand(PGCMD_ANTENNA);
	
	/* SPI Flsh */
    if (!flash.initialize())
	{
		while(1);
	}
	
	/* Initialize the telemetry data */
	telemetry.dataType = DATA_TYPE_TELEMETRY;
	telemetry.accelX = 0.0f;
	telemetry.accelY = 0.0f;
	telemetry.accelZ = 0.0f;
	telemetry.altitude = 0.0f;
	telemetry.temperature = 0.0f;
	telemetry.latitudeDegrees = 0.0f;
	telemetry.longitudeDegrees = 0.0f;
	telemetry.altitudeGPS = 0.0f;
	telemetry.fix = false;
	telemetry.fixQuality = 0;
	telemetry.satellites = 0;
	telemetry.year = 0;
	telemetry.month = 0;
	telemetry.day = 0;
	telemetry.hour = 0;
	telemetry.minute = 0;
	telemetry.seconds = 0;
	telemetry.milliseconds = 0;

}


void loop()
{
	/* Get the current millis() */
	uint32_t ms = millis();
	
	/* See if there is any new GPS data */
    GPS.read();
	
    if (GPS.newNMEAreceived()) 
	{  
		GPS.parse(GPS.lastNMEA());
    }
	
    /* if millis() or timer wraps around, we'll just reset it */
    if (timer > ms) timer = millis();

    /* approximately every 400ms or so, print out the current stats */
    if (ms - timer > 400) 
	{ 
		timer = ms;

		/* Get the acceleration reading */
	    lis.getXValue(&telemetry.accelX);
	    lis.getYValue(&telemetry.accelY);
	    lis.getZValue(&telemetry.accelZ);        
	
		/* Get the altitude reading */
		telemetry.altitude = altimeter.readAltitude();
		telemetry.temperature = altimeter.readTemp();
	
		/* Now get everything from GPS */
		telemetry.latitudeDegrees = GPS.latitudeDegrees;
		telemetry.longitudeDegrees = GPS.longitudeDegrees;
		telemetry.altitudeGPS = GPS.altitude;
		telemetry.fix = GPS.fix;
		telemetry.fixQuality = GPS.fixquality;
		telemetry.satellites = GPS.satellites;
		telemetry.year = GPS.year;
		telemetry.month = GPS.month;
		telemetry.day = GPS.day;
		telemetry.hour = GPS.hour;
		telemetry.minute = GPS.minute;
		telemetry.seconds = GPS.seconds;
		telemetry.milliseconds = GPS.milliseconds;

		//TODO: no milliseconds?
		//TODO: accellerometer looks off

		/* Cast the telemetry  data to an array of bytes and send it */
		rf69.send((uint8_t *)&telemetry, sizeof(telemetry));
    	
	    digitalWrite(15,HIGH);
	    delay(10);
	    digitalWrite(15,LOW);
	
		if (DEBUG) debug();
	}
	
	//TODO: See if anyone sent a character and dump the flash if so
}

