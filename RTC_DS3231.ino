#include "CalendarHelper.h"
#include "DS3231.h"

char serialBuffer[120];
sDateTime datetime;

bool summerTimeSet = false;

void setup()
{
    Serial.begin(115200);
    Serial.println("Hello");

    DS3231.Begin();

    CalendarHelperClass::ParseStrDateTime(datetime, "2018-01-05T04:25:10.050Z");
    DS3231.SetDateTime(datetime);

    uint16_t year = 2017;

    for (uint8_t i = 0; i < 10; i++)
    {
        sDateTime beginningOfSummerTime;
        CalendarHelperClass::BeginningOfSummerTime(beginningOfSummerTime, year + i);

        sDateTime endingOfSummerTime;
        CalendarHelperClass::EndingOfSummerTime(endingOfSummerTime, year + 1 + i);

        sprintf(serialBuffer, "Summer time in %u will start in Brazil on %i/%i/%i and end on %i/%i/%i", year + i,
            beginningOfSummerTime.Day, beginningOfSummerTime.Month, beginningOfSummerTime.Year,
            endingOfSummerTime.Day, endingOfSummerTime.Month, endingOfSummerTime.Year);
        Serial.println(serialBuffer);
    }
}

void loop()
{
    DS3231.GetDateTime(datetime);
    
    sprintf(serialBuffer, "now: %i/%i/%i %i:%i:%i",
        datetime.Day, datetime.Month, datetime.Year, datetime.Hour, datetime.Minute, datetime.Second);
    Serial.println(serialBuffer);

    if (datetime.Second % 5 == 0)
    {
        sprintf(serialBuffer, "The temperature is %.2f", DS3231.GetTemperature());
        Serial.println(serialBuffer);
    }

    delay(1000);
}