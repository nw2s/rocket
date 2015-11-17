


#define ENCRYPTION_KEY { 0x37, 0x4d, 0x32, 0x47, 0x5a, 0x66, 0x6a, 0x56, 0x54, 0x55, 0x37, 0x58, 0x45, 0x68, 0x61, 0x77 }

#define DATA_TYPE_TELEMETRY 0xA1
#define DATA_TYPE_LAUNCHER 0xA2

/* 38 bytes? */
typedef struct dataTelemetry {
	
	uint8_t dataType;
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
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute; 
	uint8_t seconds;
	uint16_t milliseconds;
	
} DataTelemetry;

typedef struct dataLauncher {
	
	uint8_t dataType;
	uint8_t armState;

} DataLauncher;

#define INDICATOR_TEMPERATURE 	't'
#define INDICATOR_ALTITUDE 		'a'
#define INDICATOR_LATITUDE		'l'
#define INDICATOR_LONGITUDE		'L'
#define INDICATOR_ALTITUDEGPS	'A'
#define INDICATOR_FIX			'f'
#define INDICATOR_QUALITY		'q'
#define INDICATOR_SATELLITES	's'
#define INDICATOR_TIME			't'
#define INDICATOR_ACCELX		'x'
#define INDICATOR_ACCELY		'y'
#define INDICATOR_ACCELZ		'z'
#define INDICATOR_LAUNCHARM		'i'
#define INDICATOR_IGNITE		'Z'
					
