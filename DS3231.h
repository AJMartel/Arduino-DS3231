/*
DS3231.h - Header file for the DS3231 Real-Time Clock

Version: 1.0.0
(c) 2014 Korneliusz Jarzebski
www.jarzebski.pl

(c) 2017 Rinaldi Segecin

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

#pragma once

#ifndef _DS3231_h
#define _DS3231_h

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#include <Wire.h>

#define DS3231_ADDRESS              (0x68)

#define DS3231_REG_TIME             (0x00)
#define DS3231_REG_ALARM_1          (0x07)
#define DS3231_REG_ALARM_2          (0x0B)
#define DS3231_REG_CONTROL          (0x0E)
#define DS3231_REG_STATUS           (0x0F)
#define DS3231_REG_TEMPERATURE      (0x11)

#define BASE_YEAR				2000UL
#define SECS_PER_MIN			60UL
#define SECS_PER_HOUR			3600UL
#define SECS_PER_DAY			SECS_PER_HOUR * 24UL
#define SECS_PER_YEAR			SECS_PER_DAY * 365UL
#define SECS_FEB				SECS_PER_DAY * 28UL
#define SECS_FEB_LEAP			SECS_PER_DAY * 29UL
#define SECS_PER_MONTH_EVEN		SECS_PER_DAY * 30UL
#define SECS_PER_MONTH_ODD		SECS_PER_DAY * 31UL

#define LEAP_YEAR(Y)	(((BASE_YEAR + Y) > 0) && !((BASE_YEAR + Y) % 4) && (((BASE_YEAR + Y) % 100) || !((BASE_YEAR + Y) % 400)))

const PROGMEM uint8_t MONTH_DAYS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
const PROGMEM uint8_t SCHWERDTFEGER[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };

struct sDateTime
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
};

struct sAlarmTime
{
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
};

typedef enum
{
    DS3231_1HZ      = 0x00,
    DS3231_4096HZ   = 0x01,
    DS3231_8192HZ   = 0x02,
    DS3231_32768HZ  = 0x03
} eDS3231_sqw_t;

typedef enum
{
    DS3231_EVERY_SECOND     = 0b00001111,
    DS3231_MATCH_S          = 0b00001110,
    DS3231_MATCH_M_S        = 0b00001100,
    DS3231_MATCH_H_M_S      = 0b00001000,
    DS3231_MATCH_DT_H_M_S   = 0b00000000,
    DS3231_MATCH_DY_H_M_S   = 0b00010000
} eDS3231_alarm1_t;

typedef enum
{
    DS3231_EVERY_MINUTE = 0b00001110,
    DS3231_MATCH_M      = 0b00001100,
    DS3231_MATCH_H_M    = 0b00001000,
    DS3231_MATCH_DT_H_M = 0b00000000,
    DS3231_MATCH_DY_H_M = 0b00010000
} eDS3231_alarm2_t;

class DS3231Class
{
public:
    bool Begin(void);

    void SetDateTime(sDateTime & pDateTime); // Set the RTC's module to the given pDateTime
    void GetDateTime(sDateTime & pDateTime); // Get the RTC's module to the given pDateTime

    static void ParseStrDateTime(sDateTime & pDateTime, char pStrDateTime[]); // Parse from ISO 8601 string format to sDateTime
    static void ConvertToSeconds(uint32_t & pSeconds, sDateTime & pDateTime); // Convert from sDateTime to number of seconds since Jan 1st of 2000
    static void ConvertToDateTime(sDateTime & pDateTime, uint32_t pSeconds); // Convert from number of seconds since beginning of 2000 to sDateTime
    static uint8_t GetDayOfWeek(uint16_t pYear, uint8_t pMonth, uint8_t pDay);

    void SetAlarm1(uint8_t dydw, uint8_t hour, uint8_t minute, uint8_t second, eDS3231_alarm1_t mode, bool armed = true);
    void GetAlarm1(sAlarmTime & pAlarmTime);
    void GetAlarmType1(eDS3231_alarm1_t & pDS3231_alarm1_t);
    bool IsAlarm1(bool clear = true);
    void ArmAlarm1(bool armed);
    bool IsArmed1(void);
    void ClearAlarm1(void);

    void SetAlarm2(uint8_t dydw, uint8_t hour, uint8_t minute, eDS3231_alarm2_t mode, bool armed = true);
    void GetAlarm2(sAlarmTime & pAlarmTime);
    void GetAlarmType2(eDS3231_alarm2_t & pDS3231_alarm2_t);
    bool IsAlarm2(bool clear = true);
    void ArmAlarm2(bool armed);
    bool IsArmed2(void);
    void ClearAlarm2(void);

    void GetOutput(eDS3231_sqw_t &pMode);
    void SetOutput(eDS3231_sqw_t pMode);
    void EnableOutput(bool enabled);
    bool IsOutput(void);
    void Enable32kHz(bool enabled);
    bool Is32kHz(void);

    void ForceConversion(void);
    float GetTemperature(void);

    void SetBattery(bool timeBattery, bool squareBattery);

private:
    inline uint8_t WireRead() {
#if ARDUINO >= 100
        return Wire.read();
#else
        return Wire.receive();
#endif
    };

    inline void WireWrite(uint8_t data) {
#if ARDUINO >= 100
        Wire.write(data);
#else
        Wire.send(data);
#endif
    };

    static uint8_t bcd2dec(uint8_t bcd);
    static uint8_t dec2bcd(uint8_t dec);

    void writeRegister8(uint8_t reg, uint8_t value);
    uint8_t readRegister8(uint8_t reg);
};

extern DS3231Class DS3231;

#endif