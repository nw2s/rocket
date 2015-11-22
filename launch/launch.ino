/*

	nw2s::r - an open source model rocket launcher and telemetry system

	Copyright (C) 2015 Scott Wilson (thomas.scott.wilson@gmail.com)

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <SPI.h>
#include <RH_RF69.h>
#include "rocket.h"

#define DEBUG false
#define FLASH_SS 23

#define SWITCH_SOURCE 21
#define SWITCH_SAFETY 22
#define SWITCH_SPARE 23
#define RELAY1 18

/* Singleton instances */
RH_RF69 rf69(4, 2); 

uint32_t timer = 0;

/* 

	States:

	0 = Safety
	1 = Armed
	2 = Launching

*/

uint32_t ignitionStartTime = 0;
int ignitionState = 0;
int launchState = 0;
int lastArmState = 0;

void setup() 
{
	Serial.begin(9600);

	pinMode(SWITCH_SOURCE, OUTPUT);
	digitalWrite(SWITCH_SOURCE, LOW);

	pinMode(SWITCH_SAFETY, INPUT_PULLUP);
	pinMode(SWITCH_SPARE, INPUT_PULLUP);
	
	pinMode(RELAY1, OUTPUT);
	digitalWrite(RELAY1, LOW);

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

	/* Set up the LED to be blinky when we send data */
    pinMode(15, OUTPUT);
}


void loop()
{
	delay(1);
	
	/* Get the current millis() */
	uint32_t ms = millis();

	/* See if the safety switch flipped */
	int newArmState = digitalRead(SWITCH_SAFETY);
	
	/* If it's more than 5s after ignition start, then stop ignition */
	if ((ms > (ignitionStartTime + 5000)) && (ignitionState == 1))
	{
		if (newArmState == LOW)
		{
			launchState = 1;
		}
		else
		{
			launchState = 0;
		}	
			
		ignitionState = 0;
		digitalWrite(RELAY1, LOW);
	}
		
    /* if millis() or timer wraps around, we'll just reset it */
    if (timer > ms) timer = millis();
	
	/* Switches are active low */
	if (newArmState != lastArmState)
	{
		if (newArmState == LOW)
		{
			launchState = 1;
		}
		else
		{
			launchState = 0;
		}
		
		lastArmState = newArmState;
	}
	
	/* TESTING ONLY!!! */
	// if (digitalRead(SWITCH_SPARE) == LOW)
	// {
	// 	if (ms > (ignitionStartTime + 5000))
	// 	{
	// 		Serial.println("igniting!");
	//
	// 		/* Go for it! */
	// 		ignitionStartTime = ms;
	// 		ignitionState = 1;
	// 		digitalWrite(RELAY1, HIGH);
	// 	}
	// }
	
    if (rf69.available())
    {
		uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);
		
		DataIgnition* ignition;

		/* Seems there's some data available. */
		if (rf69.recv(buf, &len))
		{
			/* See if it's an ignition commad */
			if (buf[0] == DATA_TYPE_IGNITION)
			{
				ignition = (DataIgnition *)buf;

				Serial.println(ignition->ignitionState);
				
				/* See if it was a request to start ignition */
				if (ignition->ignitionState == 1)
				{
					/* See if we are armed */
					if ((launchState == 1) && (ignitionState == 0))
					{
						Serial.println("igniting!");
						
						/* Go for it! */
						ignitionStartTime = ms;
						ignitionState = 1;
						launchState = 2;
						digitalWrite(RELAY1, HIGH);
					}
				}
				else
				{
					/* Assume we should cancel ignition */
					ignitionState = 0;
					digitalWrite(RELAY1, LOW);
				}
			}
		}
	}
	
    /* approximately every 400ms or so, transmit */
    if (ms - timer > 407) 
	{ 
		timer = ms;

		DataLauncher data;
		
		data.dataType = DATA_TYPE_LAUNCHER;
		data.armState = launchState;
		
		/* Cast the data to an array of bytes and send it */
		rf69.send((uint8_t *)&data, sizeof(data));
    	
	    digitalWrite(15,HIGH);
	    delay(10);
	    digitalWrite(15,LOW);	
	}
	
	//TODO: See if anyone sent a character and dump the flash if so
}

