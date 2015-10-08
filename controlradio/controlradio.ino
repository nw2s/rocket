

#include <SPI.h>
#include <RH_RF69.h>
#include "rocket.h"

#define DEBUG true

/* Singleton instance of the radio driver */
RH_RF69 rf69(4, 2); 

DataTelemetry* telemetry;

void debug()
{
	Serial.print("Temperature:   ");
	Serial.println(telemetry->temperature);
	Serial.print("Altitude:      ");
	Serial.println(telemetry->altitude);

	Serial.print("Latitude:      ");
	Serial.println(telemetry->latitudeDegrees, 11);
	Serial.print("Longitude:     ");
	Serial.println(telemetry->longitudeDegrees, 11);
	Serial.print("Altitude GPS:  ");
	Serial.println(telemetry->altitudeGPS);
	Serial.print("Fix:           ");
	Serial.println(telemetry->fix);
	Serial.print("Fix Quality:   ");
	Serial.println(telemetry->fixQuality);
	Serial.print("Satellites:   ");
	Serial.println(telemetry->satellites);
	Serial.print("Time:         ");
	Serial.print(telemetry->hour);
	Serial.print(":");
	Serial.print(telemetry->minute);
	Serial.print(":");
	Serial.print(telemetry->seconds);
	Serial.print(".");
	Serial.println(telemetry->milliseconds);


    Serial.print("X Value:       ");
    Serial.print(telemetry->accelX);
    Serial.println(" mG");
    Serial.print("Y Value:       ");
    Serial.print(telemetry->accelY);
    Serial.println(" mG");
    Serial.print("Z Value:       ");
    Serial.print(telemetry->accelZ);
    Serial.println(" mG");
    Serial.println("\n\n\n--------------------------------");
}


void setup() 
{
	Serial.begin(9600);

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
	rf69.setTxPower(14);

	/* The encryption key has to be the same on all nodes */
	uint8_t key[] = ENCRYPTION_KEY;
	rf69.setEncryptionKey(key);
	
    pinMode(15, OUTPUT);
    digitalWrite(15,HIGH);
    delay(10);
    digitalWrite(15,LOW);	
}


void loop()
{
    if (rf69.available())
    {
		/* Should be a message for us now */
		uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);

		if (rf69.recv(buf, &len))
		{
			/* Signal that we got a packet */
		    digitalWrite(15,HIGH);
		    delay(10);
		    digitalWrite(15,LOW);	

			Serial.print("got request: 0x");
			Serial.println(buf[0], HEX);

			if (buf[0] == DATA_TYPE_TELEMETRY)
			{
				/* It's telemetry, so cast it to that and ship to the display/logger */
				telemetry = (DataTelemetry *)buf;
				
				if (DEBUG) debug();				
			}
		}
		else
		{
			Serial.println("recv failed");
		}
    }
}

