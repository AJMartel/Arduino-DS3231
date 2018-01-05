#include "DS3231.h"

char serialBuffer[120];
sDateTime datetime;

void setup()
{
    Serial.begin(115200);
    Serial.println("Hello");

    DS3231.Begin();

    DS3231Class::ParseStrDateTime(datetime, "2018-01-05T04:25:10.050Z");
    DS3231.SetDateTime(datetime);
}

void loop()
{
    DS3231.GetDateTime(datetime);
    
    sprintf(serialBuffer, "now: %i/%i/%i %i:%i:%i",
        datetime.day, datetime.month, datetime.year, datetime.hour, datetime.minute, datetime.second);
    Serial.println(serialBuffer);

    if (datetime.second % 5 == 0)
    {
        sprintf(serialBuffer, "The temperature is %.2f", DS3231.GetTemperature());
        Serial.println(serialBuffer);
    }

    delay(1000);
}
