
#ifndef __DATE_TIME__
#define __DATE_TIME__

#include <time.h>

#include "misc.h"
#include "strsup.h"
#include "DataList.h"

#define SECONDS_PER_DAY (24 * 3600)
#define MS_PER_DAY (SECONDS_PER_DAY * 1000)

class ADateTime {
public:
	enum {
		Time_Absolute = 0,
		Time_Existing,
		Time_Relative_Today,
		Time_Relative_Today_UTC,
		Time_Relative_UTC,
		Time_Relative_Local,
	};

	ADateTime();
	ADateTime(uint32_t days, uint32_t ms);
	ADateTime(uint64_t ms);
	ADateTime(const ADateTime& object);
	ADateTime(const AString&   str, uint_t relative = Time_Relative_Today, uint_t *specified = NULL);

	static const uint32_t DaysSince1970;
#ifdef _WIN32
	static const uint32_t DaysSince1601;
#endif

	ADateTime& Set(uint_t day, uint_t month, uint_t year, uint_t hr = 0, uint_t minute = 0, uint_t second = 0);
	ADateTime& Set(uint32_t days, uint32_t ms) {DateTime.MilliSeconds = ms; DateTime.Days = days; return *this;}
	ADateTime& Set(uint64_t ms);
	ADateTime& SetDays(uint32_t days) {DateTime.Days         = days; return *this;}
	ADateTime& SetMS(uint32_t ms)     {DateTime.MilliSeconds = ms;   return *this;}
	ADateTime& TimeStamp(bool utc = false);

	ADateTime& NextDay()  {return Set(GetDays() + 1, GetMS());}
	ADateTime& PrevDay()  {return Set(GetDays() - 1, GetMS());}
	ADateTime& NextWeek() {return Set(GetDays() + 7, GetMS());}
	ADateTime& PrevWeek() {return Set(GetDays() - 7, GetMS());}

	ADateTime& PrevWeekDay(uint_t wd) {assert(wd < 7); uint_t d = (GetWeekDay() + 7 - wd) % 7; return Set(GetDays() - d, GetMS());}
	ADateTime& NextWeekDay(uint_t wd) {assert(wd < 7); uint_t d = (wd + 7 - GetWeekDay()) % 7; return Set(GetDays() + d, GetMS());}

	uint32_t GetDays()    const {return DateTime.Days;}
	uint32_t GetMS()      const {return DateTime.MilliSeconds;}
	
	uint_t   GetWeekDay() const;
	uint_t   GetDay()     const;
	uint_t   GetMonth()   const;
	uint_t   GetYear()    const;

	uint_t   GetHours()   const;
	uint_t   GetMinutes() const;
	uint_t   GetSeconds() const;
	uint_t   GetMilliSeconds() const;

	uint64_t GetAbsoluteSecond() const {return (uint64_t)DateTime.Days * SECONDS_PER_DAY + DateTime.MilliSeconds / 1000;}
	uint32_t GetAbsoluteHour() 	 const {return DateTime.Days * 24 + GetHours();}
	uint32_t GetAbsoluteDay()  	 const {return DateTime.Days;}
	uint32_t GetAbsoluteWeek() 	 const {return DateTime.Days / 7;}
	uint32_t GetAbsoluteMonth()  const {return (uint32_t)GetYear() * 12 + GetMonth();}

	static int FindDay(const char *str, bool utc = false);
	static int FindMonth(const char *str, bool utc = false);

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

	static const AString ISO8601Format;
	static const AString ISO8601FormatWithMS;
	
	AString DateFormat(const char *format) const;

	static void EnableDebugStrToDate(bool enable = true) {bDebugStrToDate = enable;}
	
	enum {
		Specified_Time = 1,
		Specified_Date = 2,
		Specified_Day  = 4,
	};
	ADateTime& StrToDate(const AString& String, uint_t relative = Time_Relative_Today, uint_t *specified = NULL, AString *errors = NULL);

	operator uint64_t() const;
	operator sint64_t() const;

	const char *GetDayName(uint_t d) const;
	const char *GetShortDayName(uint_t d) const;
	const char *GetMonthName(uint_t m) const;
	const char *GetShortMonthName(uint_t m) const;
	uint_t GetMonthDays(uint_t m, uint_t y) const;
	uint_t GetMonthStartDay(uint_t m, uint_t y) const;

	time_t     totime() const;
	ADateTime& fromtime(time_t t);
	ADateTime& fromtime(const struct tm *tm);
	ADateTime& fromtime_local(time_t t);

	sint64_t   GetUTCOffset() const;

	ADateTime  UTCToLocal() const;
	ADateTime  LocalToUTC() const;

	enum {
		TIMESTAMP_FORMAT_TERSE = 0,
		TIMESTAMP_FORMAT_FULL,

		TIMESTAMP_FORMAT_TIMEZONE = 0x100,
	};

	AString ToTimeStamp(uint_t format = TIMESTAMP_FORMAT_TERSE) const;
	bool    FromTimeStamp(const AString& str, bool utc = false);

	static const ADateTime MinDateTime;
	static const ADateTime MaxDateTime;

protected:
	typedef struct {
		uint32_t MilliSeconds;
		uint32_t Days;
	} DATETIME;
	typedef struct {
		uint_t WeekDay;
		uint_t Day;
		uint_t Month;
		uint_t Year;
	} DAYMONTHYEAR;

	typedef struct {
		AValue  val;
		AString valstr;
		AString str;
	} TERM;
	static void __DeleteTerm(uptr_t item, void *context) {
		(void)context;
		delete (TERM *)item;
	}
	uint_t ParseTerms(const AString& str, ADataList& list) const;

	void InitData();

	uint_t GetMonthLength(const DAYMONTHYEAR& dmy) const {return GetMonthLength(dmy.Month, dmy.Year);}
	uint_t GetMonthLength(uint_t m, uint_t y) const;
	uint_t GetMonthStart(const DAYMONTHYEAR& dmy) const {return GetMonthStart(dmy.Month, dmy.Year);}
	uint_t GetMonthStart(uint_t m, uint_t y) const;
	uint_t GetMonthEnd(const DAYMONTHYEAR& dmy) const {return GetMonthEnd(dmy.Month, dmy.Year);}
	uint_t GetMonthEnd(uint_t m, uint_t y) const;

	void CurrentTime(DATETIME& dt, bool utc = false) const;

	void AddDateTime(DATETIME& dt, uint32_t days, uint32_t ms) const;
	void SubDateTime(DATETIME& dt, uint32_t days, uint32_t ms) const;

	void AddDMY(DAYMONTHYEAR& dmy, uint_t years, uint_t months, uint_t days) const;
	void SubDMY(DAYMONTHYEAR& dmy, uint_t years, uint_t months, uint_t days) const;

	void ModifyYear(DATETIME& dt, double n, bool pos, bool neg, bool rel = false) const;
	void ModifyMonth(DATETIME& dt, double n, bool pos, bool neg, bool rel = false) const;
	void ModifyWeeks(DATETIME& dt, double n, bool pos, bool neg, bool rel = false) const;
	void ModifyDay(DATETIME& dt, double n, bool pos, bool neg, bool rel = false) const;
	void ModifyDays(DATETIME& dt, double n, bool pos, bool neg) const;
	void ModifyMS(DATETIME& dt, sint64_t n, bool pos, bool neg) const;

	void DateTimeToDMY(const DATETIME& dt, DAYMONTHYEAR& dmy) const;
	void DMYToDateTime(const DAYMONTHYEAR& dmy, DATETIME& dt) const;

protected:
	DATETIME DateTime;

	static bool bDataInit;
	static bool bDebugStrToDate;
	static const char *DayNames[];
	static const char *ShortDayNames[];
	static const char *MonthNames[];
	static const char *ShortMonthNames[];
	static const uint_t  MonthLengths[];
	static uint_t MonthStartDay[];
};

#endif


