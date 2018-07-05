


#define RF_NETWORK_ID		"1234"
#define RF_NETWORK_AESKEY	"01234567890abcdef0123456789abcdef"

#define THINGSPEAK_API_KEY1 	"XXX"
#define THINGSPEAK_API_KEY2		"YYY"
#define THINGSPEAK_CHANNELID1	"AA"
#define THINGSPEAK_CHANNELID2	"BB"
#define THINGSPEAK_URL			"https://api.thingspeak.com/update.json"



#include "M2M_Mira.h"

M2M_Mira mira(Serial1);

void setup()
{
	SerialUSB.begin(115200);
	Serial1.begin(115200);
	while (!SerialUSB);

	Serial.println("Mira IoT gateway example");
	mira.begin(RF_NETWORK_ID, RF_NETWORK_AESKEY, true);

}

void loop()
{
	if (mira.available)
	{
		M2M_MiraMessage message = mira.receive();
		switch (message.messageType)
		{
			case MiraMessageType.DataMessage:
				sendDataToThingspeak(message);
				break;
		}
	}
}