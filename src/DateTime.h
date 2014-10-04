
#ifndef __DATE_TIME__
#define __DATE_TIME__

#include <time.h>

#include "misc.h"
#include "strsup.h"

#define SECONDS_PER_DAY (24 * 3600)
#define MS_PER_DAY (SECONDS_PER_DAY * 1000)

class ADateTime {
public:
	ADateTime();
	ADateTime(uint32_t days, uint32_t ms);
	ADateTime(uint64_t ms);
	ADateTime(const ADateTime& object);
	ADateTime(const AString&   str);

	static const uint32_t DaysSince1970;
#ifdef _WIN32
	static const uint32_t DaysSince1601;
#endif

	ADateTime& Set(uint8_t day, uint8_t month, uint16_t year, uint8_t hr = 0, uint8_t minute = 0, uint8_t second = 0);
	ADateTime& Set(uint32_t days, uint32_t ms) {DateTime.MilliSeconds = ms; DateTime.Days = days; return *this;}
	ADateTime& Set(uint64_t ms);
	ADateTime& SetDays(uint32_t days) {DateTime.Days         = days; return *this;}
	ADateTime& SetMS(uint32_t ms)     {DateTime.MilliSeconds = ms;   return *this;}
	ADateTime& TimeStamp(bool utc = false);

	ADateTime& NextDay()  {return Set(GetDays() + 1, GetMS());}
	ADateTime& PrevDay()  {return Set(GetDays() - 1, GetMS());}
	ADateTime& NextWeek() {return Set(GetDays() + 7, GetMS());}
	ADateTime& PrevWeek() {return Set(GetDays() - 7, GetMS());}

	ADateTime& PrevWeekDay(uint8_t wd) {assert(wd < 7); uint_t d = (GetWeekDay() + 7 - wd) % 7; return Set(GetDays() - d, GetMS());}
	ADateTime& NextWeekDay(uint8_t wd) {assert(wd < 7); uint_t d = (wd + 7 - GetWeekDay()) % 7; return Set(GetDays() + d, GetMS());}

	uint32_t GetDays()  const {return DateTime.Days;}
	uint32_t GetMS()    const {return DateTime.MilliSeconds;}
	
	uint8_t  GetWeekDay() const;
	uint8_t  GetDay()     const;
	uint8_t  GetMonth()   const;
	uint16_t GetYear()    const;

	uint8_t  GetHours()   const;
	uint8_t  GetMinutes() const;
	uint8_t  GetSeconds() const;
	uint16_t GetMilliSeconds() const;

	uint64_t GetAbsoluteSecond() const {return (uint64_t)DateTime.Days * SECONDS_PER_DAY + DateTime.MilliSeconds / 1000;}
	uint32_t GetAbsoluteHour() 	 const {return DateTime.Days * 24 + GetHours();}
	uint32_t GetAbsoluteDay()  	 const {return DateTime.Days;}
	uint32_t GetAbsoluteWeek() 	 const {return DateTime.Days / 7;}
	uint32_t GetAbsoluteMonth()  const {return (uint32_t)GetYear() * 12 + GetMonth();}

	static int FindDay(const char *str);
	static int FindMonth(const char *str);

	ADateTime& operator  = (uint64_t ms);
	ADateTime& operator  = (const ADateTime& object) {DateTime = object.DateTime; return *this;}
	ADateTime& operator  = (const AString&   str)    {StrToDate(str); return *this;}
	
	ADateTime& operator += (const ADateTime& object);
	ADateTime& operator -= (const ADateTime& object);

	friend ADateTime operator + (const ADateTime& object1, const ADateTime& object2) {ADateTime res = object1; res += object2; return res;}
	friend ADateTime operator - (const ADateTime& object1, const ADateTime& object2) {ADateTime res = object1; res -= object2; return res;}

	friend bool operator == (const ADateTime& object1, const ADateTime& object2);
	friend bool operator != (const ADateTime& object1, const ADateTime& object2);
	friend bool operator <= (const ADateTime& object1, const ADateTime& object2);
	friend bool operator <  (const ADateTime& object1, const ADateTime& object2);
	friend bool operator >= (const ADateTime& object1, const ADateTime& object2);
	friend bool operator >  (const ADateTime& object1, const ADateTime& object2);

	friend int CompareDates(const ADateTime& object1, const ADateTime& object2);

	AString DateToStr() const;
	AString TimeStr() const;

	AString DateFormat(const char *format) const;

	enum {
		Specified_Time = 1,
		Specified_Date = 2,
		Specified_Day  = 4,
	};
	ADateTime& StrToDate(const AString& String, bool current = true, uint_t *specified = NULL);

	operator uint64_t() const;
	operator sint64_t() const;

	const char *GetDayName(uint_t d) const;
	const char *GetShortDayName(uint_t d) const;
	const char *GetMonthName(uint_t m) const;
	const char *GetShortMonthName(uint_t m) const;
	uint_t GetMonthDays(uint_t m) const;
	uint_t GetMonthStartDay(uint_t m) const;

	time_t     totime() const;
	ADateTime& fromtime(time_t t);
	ADateTime& fromtime(const struct tm *tm);
	ADateTime& fromtime_local(time_t t);

	ADateTime  UTCToLocal() const;

	enum {
		TIMESTAMP_FORMAT_TERSE = 0,
		TIMESTAMP_FORMAT_FULL,

		TIMESTAMP_FORMAT_TIMEZONE = 0x100,
	};

	AString ToTimeStamp(uint_t format = TIMESTAMP_FORMAT_TERSE) const;
	bool    FromTimeStamp(const AString& str, bool utc = false);

	static sint32_t  GetLocalOffsetMinutes();
	static ADateTime GetLocalOffset(bool& backwards);

	static const ADateTime MinDateTime;
	static const ADateTime MaxDateTime;

protected:
	typedef struct {
		uint32_t MilliSeconds;
		uint32_t Days;
	} DATETIME;
	typedef struct {
		uint8_t WeekDay;
		uint8_t Day;
		uint8_t Month;
		uint16_t Year;
	} DAYMONTHYEAR;

	void InitData();

	void CurrentTime(DATETIME& dt, bool utc = false) const;

	void DateTimeToDMY(const DATETIME& dt, DAYMONTHYEAR& dmy) const;
	void DMYToDateTime(const DAYMONTHYEAR& dmy, DATETIME& dt) const;

protected:
	DATETIME DateTime;

	static bool bDataInit;
	static const char *DayNames[];
	static const char *ShortDayNames[];
	static const char *MonthNames[];
	static const char *ShortMonthNames[];
	static const uint_t  MonthLengths[];
	static uint_t MonthStartDay[];
};

#endif


