


#define ENCRYPTION_KEY { 0x37, 0x4d, 0x32, 0x47, 0x5a, 0x66, 0x6a, 0x56, 0x54, 0x55, 0x37, 0x58, 0x45, 0x68, 0x61, 0x77 }

#define DATA_TYPE_TELEMETRY 0xA1

typedef struct dataTelemetry {
	
	int dataType;
	int accelX;
	int accelY;
	int accelZ;
	float altitude;
	float temperature;
	float latitudeDegrees;
	float longitudeDegrees;
	float altitudeGPS;
	bool fix;
	uint8_t fixQuality;
	uint8_t satellites;
	uint8_t hour;
	uint8_t minute; 
	uint8_t seconds;
	uint16_t milliseconds;
	
} DataTelemetry;

