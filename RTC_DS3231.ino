#include "DS3231.h"

char serialBuffer[120];
sDateTime datetime;

bool summerTimeSet = false;

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

    //if ((datetime.month == 10) && (datetime.day >= 15) && (!summerTimeSet))
    //{
    //    sDateTime beginningOfSummerTime;
    //    BeginningOfSummerTime(beginningOfSummerTime, datetime.year);
    //    if (beginningOfSummerTime.day == datetime.day)
    //    {
    //        summerTimeSet = true;
    //        datetime.hour += 1;
    //    }
    //}
}

void CarnavalSunday(sDateTime & pDateTime, uint16_t pYear) // Return the Easter day from the given year
{
    uint8_t a = pYear % 19;
    uint8_t b = pYear / 100;
    uint8_t c = pYear % 100;
    uint8_t d = b / 4;
    uint8_t e = b % 4;
    uint8_t f = (b + 8) / 25;
    uint8_t g = (b - f + 1) / 3;
    uint8_t h = (19 * a + b - d - g + 15) % 30;
    uint8_t i = c / 4;
    uint8_t k = c % 4;
    uint8_t L = (32 + 2 * e + 2 * i - h - k) % 7;
    uint8_t m = (a + 11 * h + 22 * L) / 451;
    uint8_t month = (h + L - 7 * m + 114) / 31;
    uint8_t day = ((h + L - 7 * m + 114) % 31) + 1;

    pDateTime.year = pYear;
    pDateTime.month = month;
    pDateTime.day = day;

    uint32_t time;
    DS3231Class::ConvertToSeconds(time, pDateTime);
    time -= 49 * SECS_PER_DAY;
    DS3231Class::ConvertToDateTime(pDateTime, time);
}

void BeginningOfSummerTime(sDateTime & pDateTime, uint16_t pYear) // Return the date when the summer time begins in Brazil
{
    // It's the third sunday in October
    pDateTime.year = pYear;
    pDateTime.month = 10;
    pDateTime.day = 15 + ((7 - DS3231Class::GetDayOfWeek(pYear, 10, 1)) % 7);
}

void EndingOfSummerTime(sDateTime & pDateTime, uint16_t pYear)
{
    sDateTime carnavalSunday;
    CarnavalSunday(carnavalSunday, pYear);

    // sets the third february sunday
    pDateTime.year = pYear;
    pDateTime.month = 2;
    pDateTime.day = 15 + ((7 - DS3231Class::GetDayOfWeek(pYear, 2, 15)) % 7);
    
    if (carnavalSunday.day == pDateTime.day)
        pDateTime.day += 7;
}