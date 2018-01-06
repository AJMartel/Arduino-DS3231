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
        BeginningOfSummerTime(beginningOfSummerTime, year + i);

        sDateTime endingOfSummerTime;
        EndingOfSummerTime(endingOfSummerTime, year + 1 + i);

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

void CarnavalSunday(sDateTime & pDateTime, uint16_t pYear) // Return the Carnaval day from the given year
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
    uint8_t Month = (h + L - 7 * m + 114) / 31;
    uint8_t Day = ((h + L - 7 * m + 114) % 31) + 1;

    pDateTime.Year = pYear;
    pDateTime.Month = Month;
    pDateTime.Day = Day;

    uint32_t time;
    CalendarHelperClass::ConvertToSeconds(time, pDateTime);
    time -= 49 * SECS_PER_DAY;
    CalendarHelperClass::ConvertToDateTime(pDateTime, time);
}

void BeginningOfSummerTime(sDateTime & pDateTime, uint16_t pYear) // Return the date when the summer time begins in Brazil
{
    // Beginning of the summer time is on the third sunday in October
    pDateTime.Year = pYear;
    pDateTime.Month = 10;
    pDateTime.Day = 15 + ((7 - CalendarHelperClass::GetDayOfWeek(pYear, 10, 1)) % 7);
}

void EndingOfSummerTime(sDateTime & pDateTime, uint16_t pYear)
{
    sDateTime carnavalSunday;
    CarnavalSunday(carnavalSunday, pYear);

    // Sets the third february sunday
    pDateTime.Year = pYear;
    pDateTime.Month = 2;
    pDateTime.Day = 14 + ((7 - CalendarHelperClass::GetDayOfWeek(pYear, 2, 15)) % 7);
    
    if (carnavalSunday.Day == pDateTime.Day)
        pDateTime.Day += 7;
}