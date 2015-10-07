

#include <SPI.h>
#include <RH_RF69.h>

/* Singleton instance of the radio driver */
RH_RF69 rf69(4, 2); 

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
	uint8_t key[] = { 0x37, 0x4d, 0x32, 0x47, 0x5a, 0x66, 0x6a, 0x56, 0x54, 0x55, 0x37, 0x58, 0x45, 0x68, 0x61, 0x77 };
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

			Serial.print("got request: ");
			Serial.println((char*)buf);

			// // Send a reply
			// uint8_t data[] = "And hello back to you";
			// rf69.send(data, sizeof(data));
			// rf69.waitPacketSent();
			// Serial.println("Sent a reply");
		}
		else
		{
			Serial.println("recv failed");
		}
    }
}

