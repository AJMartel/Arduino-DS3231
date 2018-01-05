/*
DS3231.cpp - Class file for the DS3231 Real-Time Clock

Version: 1.0.1
(c) 2014 Korneliusz Jarzebski
www.jarzebski.pl

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "DS3231.h"

bool DS3231Class::Begin(void)
{
    Wire.begin();

    SetBattery(true, false);

    return true;
}

void DS3231Class::SetDateTime(sDateTime & pDateTime)
{
    Wire.beginTransmission(DS3231_ADDRESS);

    WireWrite(DS3231_REG_TIME);

    WireWrite(dec2bcd(pDateTime.second));
    WireWrite(dec2bcd(pDateTime.minute));
    WireWrite(dec2bcd(pDateTime.hour));
    WireWrite(dec2bcd(GetDayOfWeek(pDateTime.year, pDateTime.month, pDateTime.day))); // 0 is sunday
    WireWrite(dec2bcd(pDateTime.day));
    WireWrite(dec2bcd(pDateTime.month));
    WireWrite(dec2bcd(pDateTime.year - 2000));

    WireWrite(DS3231_REG_TIME);

    Wire.endTransmission();
}

void DS3231Class::GetDateTime(sDateTime & pDateTime)
{
    int values[7];

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_TIME);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 7);

    while (!Wire.available()) {};

    for (uint8_t i = 0; i < 7; i++)
    {
        values[i] = bcd2dec(WireRead());
    }

    Wire.endTransmission();

    pDateTime.second = values[0];
    pDateTime.minute = values[1];
    pDateTime.hour = values[2];
    pDateTime.dayOfWeek = values[3];
    pDateTime.day = values[4];
    pDateTime.month = values[5];
    pDateTime.year = values[6] + 2000;
}

void DS3231Class::ParseStrDateTime(sDateTime & pDateTime, char pStrDateTime[])
{
    char aux[5];

    if ((pStrDateTime[10] == 'T') && ((pStrDateTime[19] == 'Z') || (pStrDateTime[23] == 'Z')))
    {
        memcpy(aux, pStrDateTime, 4);
        pDateTime.year = atoi(aux);
        memset(aux, 0x00, 5);
        memcpy(aux, &pStrDateTime[5], 2);
        pDateTime.month = atoi(aux);
        memcpy(aux, &pStrDateTime[8], 2);
        pDateTime.day = atoi(aux);
        memcpy(aux, &pStrDateTime[11], 2);
        pDateTime.hour = atoi(aux);
        memcpy(aux, &pStrDateTime[14], 2);
        pDateTime.minute = atoi(aux);
        memcpy(aux, &pStrDateTime[17], 2);
        pDateTime.second = atoi(aux);
        pDateTime.dayOfWeek = GetDayOfWeek(pDateTime.year, pDateTime.month, pDateTime.day);
    }
}

void DS3231Class::ConvertToSeconds(uint32_t & pSeconds, sDateTime & pDateTime)
{
    int i;
    int16_t year = pDateTime.year - 2000;

    // seconds from 2000 till 1st jan 00:00:00 of the given year
    pSeconds = year * SECS_PER_YEAR;
    for (i = 0; i < year; i++)
    {
        if (LEAP_YEAR(i))
        {
            pSeconds += SECS_PER_DAY;
        }
    }

    // add days for this actual year passed on datetime, months start from 1
    for (i = 1; i < pDateTime.month; i++)
    {
        switch (pgm_read_byte(MONTH_DAYS + i - 1))
        {
        case 28:
            if (LEAP_YEAR(year))
                pSeconds += SECS_FEB_LEAP;
            else
                pSeconds += SECS_FEB;
            break;
        case 30:
            pSeconds += SECS_PER_MONTH_EVEN;
            break;
        case 31:
            pSeconds += SECS_PER_MONTH_ODD;
            break;
        default:
            break;
        }
    }

    pSeconds += (pDateTime.day - 1) * SECS_PER_DAY;
    pSeconds += pDateTime.hour * SECS_PER_HOUR;
    pSeconds += pDateTime.minute * SECS_PER_MIN;
    pSeconds += pDateTime.second;
}

void DS3231Class::ConvertToDateTime(sDateTime & pDateTime, uint32_t pSeconds)
{
    uint8_t year;
    uint8_t month, monthLength;
    uint32_t time;
    uint16_t days;

    /*sprintf(lBuffer, "RTC has been set %lu.", (uint32_t)RTCTimer.Time);
    SerialInterpreter.Send(lBuffer);*/

    time = (uint32_t)pSeconds;
    pDateTime.second = time % 60;
    time /= 60; // now it is minutes
    pDateTime.minute = time % 60;
    time /= 60; // now it is hours
    pDateTime.hour = time % 24;
    time /= 24; // now it is days
    pDateTime.dayOfWeek = ((time + 4) % 7) + 1;  // Sunday is day 1 

    year = 0;
    days = 0;
    for (;;)
    {
        days += (LEAP_YEAR(year) ? 366 : 365);
        if (days < time)
            year++;
        else
            break;
    }
    pDateTime.year = year + 2000; // year is offset from 2000 

    days -= (LEAP_YEAR(year) ? 366 : 365);
    time -= days; // now it is days in this year, starting at 0

    days = 0;
    month = 0;
    monthLength = 0;

    for (month = 0; month < 12; month++)
    {
        if ((month == 1) && (LEAP_YEAR(year)))
        {
            monthLength = 29;
        }
        else
        {
            monthLength = pgm_read_byte(MONTH_DAYS + month);
        }

        if (time >= monthLength)
        {
            time -= monthLength;
        }
        else
        {
            break;
        }
    }

    pDateTime.month = month + 1;  // jan is month 1  
    pDateTime.day = time + 1;     // day of month
}

void DS3231Class::GetAlarm1(sAlarmTime & pAlarmTime)
{
    uint8_t values[4];

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_ALARM_1);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 4);

    while (!Wire.available()) {};

    for (int i = 3; i >= 0; i--)
    {
        values[i] = bcd2dec(WireRead() & 0b01111111);
    }

    Wire.endTransmission();

    pAlarmTime.day = values[0];
    pAlarmTime.hour = values[1];
    pAlarmTime.minute = values[2];
    pAlarmTime.second = values[3];
}

void DS3231Class::GetAlarmType1(eDS3231_alarm1_t & pDS3231_alarm1_t)
{
    uint8_t values[4];
    uint8_t mode = 0;

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_ALARM_1);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 4);

    while (!Wire.available()) {};

    for (int i = 3; i >= 0; i--)
    {
        values[i] = bcd2dec(WireRead());
    }

    Wire.endTransmission();

    mode |= ((values[3] & 0b01000000) >> 6);
    mode |= ((values[2] & 0b01000000) >> 5);
    mode |= ((values[1] & 0b01000000) >> 4);
    mode |= ((values[0] & 0b01000000) >> 3);
    mode |= ((values[0] & 0b00100000) >> 1);

    pDS3231_alarm1_t = (eDS3231_alarm1_t)mode;
}

void DS3231Class::SetAlarm1(uint8_t dydw, uint8_t hour, uint8_t minute, uint8_t second, eDS3231_alarm1_t mode, bool armed)
{
    second = dec2bcd(second);
    minute = dec2bcd(minute);
    hour = dec2bcd(hour);
    dydw = dec2bcd(dydw);

    switch (mode)
    {
    case DS3231_EVERY_SECOND:
        second |= 0b10000000;
        minute |= 0b10000000;
        hour |= 0b10000000;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_S:
        second &= 0b01111111;
        minute |= 0b10000000;
        hour |= 0b10000000;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_M_S:
        second &= 0b01111111;
        minute &= 0b01111111;
        hour |= 0b10000000;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_H_M_S:
        second &= 0b01111111;
        minute &= 0b01111111;
        hour &= 0b01111111;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_DT_H_M_S:
        second &= 0b01111111;
        minute &= 0b01111111;
        hour &= 0b01111111;
        dydw &= 0b01111111;
        break;

    case DS3231_MATCH_DY_H_M_S:
        second &= 0b01111111;
        minute &= 0b01111111;
        hour &= 0b01111111;
        dydw &= 0b01111111;
        dydw |= 0b01000000;
        break;
    }

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_ALARM_1);
    WireWrite(second);
    WireWrite(minute);
    WireWrite(hour);
    WireWrite(dydw);
    Wire.endTransmission();

    ArmAlarm1(armed);

    ClearAlarm1();
}

bool DS3231Class::IsAlarm1(bool clear)
{
    uint8_t alarm;

    alarm = readRegister8(DS3231_REG_STATUS);
    alarm &= 0b00000001;

    if (alarm && clear)
    {
        ClearAlarm1();
    }

    return alarm;
}

void DS3231Class::ArmAlarm1(bool armed)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);

    if (armed)
    {
        value |= 0b00000001;
    }
    else
    {
        value &= 0b11111110;
    }

    writeRegister8(DS3231_REG_CONTROL, value);
}

bool DS3231Class::IsArmed1(void)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);
    value &= 0b00000001;
    return value;
}

void DS3231Class::ClearAlarm1(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);
    value &= 0b11111110;

    writeRegister8(DS3231_REG_STATUS, value);
}

void DS3231Class::GetAlarm2(sAlarmTime & pAlarmTime)
{
    uint8_t values[3];

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_ALARM_2);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 3);

    while (!Wire.available()) {};

    for (int i = 2; i >= 0; i--)
    {
        values[i] = bcd2dec(WireRead() & 0b01111111);
    }

    Wire.endTransmission();

    pAlarmTime.day = values[0];
    pAlarmTime.hour = values[1];
    pAlarmTime.minute = values[2];
    pAlarmTime.second = 0;
}

void DS3231Class::GetAlarmType2(eDS3231_alarm2_t & pDS3231_alarm2_t)
{
    uint8_t values[3];
    uint8_t mode = 0;

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_ALARM_2);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 3);

    while (!Wire.available()) {};

    for (int i = 2; i >= 0; i--)
    {
        values[i] = bcd2dec(WireRead());
    }

    Wire.endTransmission();

    mode |= ((values[2] & 0b01000000) >> 5);
    mode |= ((values[1] & 0b01000000) >> 4);
    mode |= ((values[0] & 0b01000000) >> 3);
    mode |= ((values[0] & 0b00100000) >> 1);

    pDS3231_alarm2_t = (eDS3231_alarm2_t)mode;
}

void DS3231Class::SetAlarm2(uint8_t dydw, uint8_t hour, uint8_t minute, eDS3231_alarm2_t mode, bool armed)
{
    minute = dec2bcd(minute);
    hour = dec2bcd(hour);
    dydw = dec2bcd(dydw);

    switch (mode)
    {
    case DS3231_EVERY_MINUTE:
        minute |= 0b10000000;
        hour |= 0b10000000;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_M:
        minute &= 0b01111111;
        hour |= 0b10000000;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_H_M:
        minute &= 0b01111111;
        hour &= 0b01111111;
        dydw |= 0b10000000;
        break;

    case DS3231_MATCH_DT_H_M:
        minute &= 0b01111111;
        hour &= 0b01111111;
        dydw &= 0b01111111;
        break;

    case DS3231_MATCH_DY_H_M:
        minute &= 0b01111111;
        hour &= 0b01111111;
        dydw &= 0b01111111;
        dydw |= 0b01000000;
        break;
    }

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_ALARM_2);
    WireWrite(minute);
    WireWrite(hour);
    WireWrite(dydw);
    Wire.endTransmission();

    ArmAlarm2(armed);

    ClearAlarm2();
}

void DS3231Class::ArmAlarm2(bool armed)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);

    if (armed)
    {
        value |= 0b00000010;
    }
    else
    {
        value &= 0b11111101;
    }

    writeRegister8(DS3231_REG_CONTROL, value);
}

bool DS3231Class::IsArmed2(void)
{
    uint8_t value;
    value = readRegister8(DS3231_REG_CONTROL);
    value &= 0b00000010;
    value >>= 1;
    return value;
}

void DS3231Class::ClearAlarm2(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);
    value &= 0b11111101;

    writeRegister8(DS3231_REG_STATUS, value);
}

bool DS3231Class::IsAlarm2(bool clear)
{
    uint8_t alarm;

    alarm = readRegister8(DS3231_REG_STATUS);
    alarm &= 0b00000010;

    if (alarm && clear)
    {
        ClearAlarm2();
    }

    return alarm;
}

void DS3231Class::GetOutput(eDS3231_sqw_t &pMode)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b00011000;
    value >>= 3;

    pMode = (eDS3231_sqw_t)value;
}

void DS3231Class::SetOutput(eDS3231_sqw_t mode)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b11100111;
    value |= (mode << 3);

    writeRegister8(DS3231_REG_CONTROL, value);
}

void DS3231Class::EnableOutput(bool enabled)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b11111011;
    value |= (!enabled << 2);

    writeRegister8(DS3231_REG_CONTROL, value);
}

bool DS3231Class::IsOutput(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    value &= 0b00000100;
    value >>= 2;

    return !value;
}

void DS3231Class::Enable32kHz(bool enabled)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);

    value &= 0b11110111;
    value |= (enabled << 3);

    writeRegister8(DS3231_REG_STATUS, value);
}

bool DS3231Class::Is32kHz(void)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_STATUS);

    value &= 0b00001000;
    value >>= 3;

    return value;
}

void DS3231Class::ForceConversion(void)
{
    uint8_t busy;
    uint8_t value;

    busy = readRegister8(DS3231_REG_STATUS);

    if (busy & 0b00000100)
    {
        value = readRegister8(DS3231_REG_CONTROL);
        value |= 0b00100000;
        writeRegister8(DS3231_REG_CONTROL, value);

        do {} while ((readRegister8(DS3231_REG_CONTROL) & 0b00100000) != 0);
    }
}

float DS3231Class::GetTemperature(void)
{
    uint8_t msb, lsb;

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(DS3231_REG_TEMPERATURE);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 2);

    while (!Wire.available()) {};

    msb = WireRead();
    lsb = WireRead();

    return ((((short)msb << 8) | (short)lsb) >> 6) / 4.0f;
}

void DS3231Class::SetBattery(bool timeBattery, bool squareBattery)
{
    uint8_t value;

    value = readRegister8(DS3231_REG_CONTROL);

    if (squareBattery)
    {
        value |= 0b01000000;
    }
    else
    {
        value &= 0b10111111;
    }

    if (timeBattery)
    {
        value &= 0b01111011;
    }
    else
    {
        value |= 0b10000000;
    }

    writeRegister8(DS3231_REG_CONTROL, value);
}

/* Calculate day of week in proleptic Gregorian calendar. Sunday == 0. */
uint8_t DS3231Class::GetDayOfWeek(uint16_t pYear, uint8_t pMonth, uint8_t pDay)
{
    int adjustment, mm, yy;

    adjustment = (14 - pMonth) / 12;
    mm = pMonth + 12 * adjustment - 2;
    yy = pYear - adjustment;
    return (pDay + (13 * mm - 1) / 5 + yy + yy / 4 - yy / 100 + yy / 400) % 7;
}

uint8_t DS3231Class::bcd2dec(uint8_t bcd)
{
    return ((bcd / 16) * 10) + (bcd % 16);
}

uint8_t DS3231Class::dec2bcd(uint8_t dec)
{
    return ((dec / 10) * 16) + (dec % 10);
}

void DS3231Class::writeRegister8(uint8_t reg, uint8_t value)
{
    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(reg);
    WireWrite(value);
    Wire.endTransmission();
}

uint8_t DS3231Class::readRegister8(uint8_t reg)
{
    uint8_t value;

    Wire.beginTransmission(DS3231_ADDRESS);
    WireWrite(reg);
    Wire.endTransmission();

    Wire.requestFrom(DS3231_ADDRESS, 1);
    while (!Wire.available()) {};
    value = WireRead();
    Wire.endTransmission();

    return value;
}

DS3231Class DS3231;