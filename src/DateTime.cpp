
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __LINUX__
#include <sys/time.h>
#endif

#include "DateTime.h"

/* end of includes */

#define DATUM_DAY 2
#define DATUM_YEAR 1980

#define ISLEAP(y) ((((y) % 4) == 0) && ((((y) % 100) != 0) || (((y) % 400) == 0)))
#define DAYSCOUNT(y) (365 * (y) + (((y) - 1) / 4) - (((y) - 1) / 100) + (((y) - 1) / 400))
#define DAYSINYEAR(y) (ISLEAP(y) ? 366 : 365)

const uint32_t ADateTime::DaysSince1970 = DAYSCOUNT(DATUM_YEAR) - DAYSCOUNT(1970);
#ifdef _WIN32
const uint32_t ADateTime::DaysSince1601 = DAYSCOUNT(DATUM_YEAR) - DAYSCOUNT(1601);
#endif

const ADateTime ADateTime::MinDateTime(0, 0);
const ADateTime ADateTime::MaxDateTime(~0, ~0);

bool ADateTime::bDataInit = false;

const char *ADateTime::DayNames[7] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
};

const char *ADateTime::ShortDayNames[7] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
};

const char *ADateTime::MonthNames[12] = {
	"January",
	"February",
	"March",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
};

const char *ADateTime::ShortMonthNames[12] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
};

const uint_t ADateTime::MonthLengths[12] = {
	31, 28, 31, 30, 31, 30,
	31, 31, 30, 31, 30, 31,
};

uint_t ADateTime::MonthStartDay[12] = {0};

ADateTime::ADateTime()
{
	InitData();

	TimeStamp(false);
}

ADateTime::ADateTime(uint32_t days, uint32_t ms)
{
	InitData();

	DateTime.MilliSeconds = ms;
	DateTime.Days         = days;
}

ADateTime::ADateTime(uint64_t ms)
{
	InitData();

	DateTime.Days         = (uint32_t)(ms / MS_PER_DAY);
	DateTime.MilliSeconds = (uint32_t)(ms - (uint64_t)MS_PER_DAY * (uint64_t)DateTime.Days);
}

ADateTime::ADateTime(const ADateTime& object)
{
	InitData();

	DateTime = object.DateTime;
}

ADateTime::ADateTime(const AString& str)
{
	InitData();

	StrToDate(str);
}

void ADateTime::InitData()
{
	if (!bDataInit) {
		bDataInit = true;

		int i, n = 0;
		for (i = 0; i < 12; i++) {
			MonthStartDay[i] = n;
			n += MonthLengths[i];
		}

		assert(n == 365);
	}
}

ADateTime& ADateTime::TimeStamp(bool utc)
{
	CurrentTime(DateTime, utc);

	return *this;
}

void ADateTime::CurrentTime(DATETIME& dt, bool utc) const
{
#ifdef _WIN32
	DAYMONTHYEAR dmy;
	SYSTEMTIME   st;

	if (utc) ::GetSystemTime(&st);
	else	 ::GetLocalTime(&st);

	memset(&dmy, 0, sizeof(dmy));
	dmy.Day   = (uint8_t)st.wDay;
	dmy.Month = (uint8_t)st.wMonth;
	dmy.Year  = st.wYear;

	DMYToDateTime(dmy, dt);
	dt.MilliSeconds = (uint32_t)(((uint32_t)st.wHour * 60 + (uint32_t)st.wMinute) * 60 + (uint32_t)st.wSecond) * 1000 + st.wMilliseconds;
#endif

#ifdef __LINUX__
	struct timeval timespec;

	gettimeofday(&timespec, NULL);

	if (!utc) {
		struct tm *tm = localtime(&timespec.tv_sec);

		tm->tm_isdst = 0;

		timespec.tv_sec = mktime(tm);
	}

	dt.Days          = timespec.tv_sec / SECONDS_PER_DAY;
	dt.MilliSeconds  = (timespec.tv_sec % SECONDS_PER_DAY) * 1000 + (timespec.tv_usec / 1000);
	dt.Days		    -= DaysSince1970;
#endif
}

ADateTime& ADateTime::operator = (uint64_t ms)
{
	DateTime.Days         = (uint32_t)(ms / MS_PER_DAY);
	DateTime.MilliSeconds = (uint32_t)(ms - MS_PER_DAY * (uint64_t)DateTime.Days);

	return *this;
}

ADateTime& ADateTime::operator += (const ADateTime& object)
{
	operator = ((uint64_t)*this + (uint64_t)object);

	return *this;
}

ADateTime& ADateTime::operator -= (const ADateTime& object)
{
	uint64_t val1 = (uint64_t)*this;
	uint64_t val2 = (uint64_t)object;

	if (val1 >= val2) val1 -= val2;
	else			  val1  = 0;

	operator = (val1);

	return *this;
}

ADateTime& ADateTime::Set(uint8_t day, uint8_t month, uint16_t year, uint8_t hour, uint8_t minute, uint8_t second)
{
	DAYMONTHYEAR dmy;

	memset(&dmy, 0, sizeof(dmy));

	dmy.Day   = day;
	dmy.Month = month;
	dmy.Year  = year;

	DMYToDateTime(dmy, DateTime);

	DateTime.MilliSeconds += ((uint32_t)hour * 3600 + (uint32_t)minute * 60 + (uint32_t)second) * 1000;

	return *this;
}

ADateTime& ADateTime::Set(uint64_t ms)
{
	DateTime.Days         = (uint32_t)(ms / MS_PER_DAY);
	DateTime.MilliSeconds = (uint32_t)(ms % MS_PER_DAY);

	return *this;
}

uint8_t ADateTime::GetWeekDay() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.WeekDay;
}

uint8_t ADateTime::GetDay() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.Day;
}

uint8_t ADateTime::GetMonth() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.Month;
}

uint16_t ADateTime::GetYear() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.Year;
}

uint8_t ADateTime::GetHours() const
{
	return (uint8_t)(DateTime.MilliSeconds / (1000 * 60 * 60));
}

uint8_t ADateTime::GetMinutes() const
{
	return (uint8_t)((DateTime.MilliSeconds / (1000 * 60)) % 60);
}

uint8_t ADateTime::GetSeconds() const
{
	return (uint8_t)((DateTime.MilliSeconds / 1000) % 60);
}

uint16_t ADateTime::GetMilliSeconds() const
{
	return (uint16_t)(DateTime.MilliSeconds % 1000);
}

void ADateTime::DateTimeToDMY(const DATETIME& dt, DAYMONTHYEAR& dmy) const
{
	uint32_t days = dt.Days + (dt.MilliSeconds / MS_PER_DAY);
	uint32_t n, m;

	memset(&dmy, 0, sizeof(dmy));

	dmy.WeekDay = (uint8_t)((days + DATUM_DAY) % 7);

	n = DATUM_YEAR;
	while (true) {
		m = ISLEAP(n) ? 366 : 365;

		if (days >= m) {
			days -= m;
			n++;
		}
		else break;
	}
	
	assert(days <= 366);

	dmy.Year = (uint16_t)n;
	n = 0;
	while (true) {
		assert(n < 12);

		m = ((n == 1) && ISLEAP(dmy.Year)) ? 29 : MonthLengths[n];

		if (days >= m) {
			days -= m;
			n++;
		}
		else break;
	}

	assert(n < 12);
	assert(days < 31);

	dmy.Month = (uint8_t)(1 + n);
	dmy.Day   = (uint8_t)(1 + days);
}

void ADateTime::DMYToDateTime(const DAYMONTHYEAR& dmy, DATETIME& dt) const
{
	DAYMONTHYEAR dmy1 = dmy;

	dmy1.Year  = MAX(dmy1.Year, DATUM_YEAR);
	dmy1.Month = LIMIT(dmy1.Month, 1, 12);
	dmy1.Day   = LIMIT(dmy1.Day, 1, 31);

	memset(&dt, 0, sizeof(dt));

	dt.Days  = DAYSCOUNT(dmy1.Year) - DAYSCOUNT(DATUM_YEAR);
	dt.Days += MonthStartDay[dmy1.Month - 1];
	dt.Days += ((dmy1.Month > 2) && ISLEAP(dmy1.Year));
	dt.Days += dmy1.Day - 1;
	
	dt.MilliSeconds = 0;
}

AString ADateTime::DateToStr() const
{
	AString String;
	DAYMONTHYEAR dmy;
	uint32_t t;
	uint_t   hrs, mins, secs;

	DateTimeToDMY(DateTime, dmy);

	t    = (DateTime.MilliSeconds + 500) / 1000;
	hrs  = t / 3600;
	t   -= hrs * 3600;
	mins = t / 60;
	secs = t - mins * 60;

	String.Format("%s %02u-%s-%4u %02u:%02u:%02u",
				  ShortDayNames[dmy.WeekDay], dmy.Day, ShortMonthNames[dmy.Month - 1], dmy.Year,
				  hrs, mins, secs);

	return String;
}

AString ADateTime::DateFormat(const char *format) const
{
	AString str;
	const char *p = format;
	
	while (p[0]) {
		if (p[0] == '%') {
			p++;

			switch (p[0]) {
				case 'D':
					str.printf("%02u", GetDay());
					break;

				case 'M':
					str.printf("%02u", GetMonth());
					break;

				case 'Y':
					str.printf("%04u", GetYear());
					break;

				case 'h':
					str.printf("%02u", GetHours());
					break;

				case 'm':
					str.printf("%02u", GetMinutes());
					break;

				case 's':
					str.printf("%02u", GetSeconds());
					break;

				case 'S':
					str.printf("%03u", GetMilliSeconds());
					break;

				case 'd':
					str += ShortDayNames[GetWeekDay()];
					break;

				case 'l':
					str += DayNames[GetWeekDay()];
					break;

				case 'N':
					str += ShortMonthNames[GetMonth() - 1];
					break;

				case 't':
					str.printf("%03u", GetMilliSeconds());
					break;

				default:
					break;
			}
		}
		else str += p[0];

		p++;
	}

	return str;
}

int ADateTime::FindDay(const char *str)
{
	int res;

	for (res = 0; res < (int)NUMBEROF(ShortDayNames); res++) {
		if (stricmp(str, ShortDayNames[res]) == 0) return res;
	}
	for (res = 0; res < (int)NUMBEROF(DayNames); res++) {
		if (stricmp(str, DayNames[res]) == 0) return res;
	}

	return -1;
}

int ADateTime::FindMonth(const char *str)
{
	int res;

	for (res = 0; res < (int)NUMBEROF(ShortMonthNames); res++) {
		if (stricmp(str, ShortMonthNames[res]) == 0) return res;
	}
	for (res = 0; res < (int)NUMBEROF(MonthNames); res++) {
		if (stricmp(str, MonthNames[res]) == 0) return res;
	}

	return -1;
}

ADateTime& ADateTime::StrToDate(const AString& String, bool current, uint_t *specified)
{
	DAYMONTHYEAR dmy;
	DATETIME     dt;
	AString String1 = String.SearchAndReplace(",", " ");
	uint_t i, j, n = String1.CountWords();

	if (current) {
		CurrentTime(DateTime);
		DateTime.MilliSeconds = 0;
	}

	if (specified) *specified = 0;

	for (i = 0; i < n; i++) {
		AString word = String1.Word(i);

		if (IsAlphaChar(word[0])) {
			uint_t j;

			for (j = 0; IsAlphaChar(word[j]); j++) ;

			if (j < (uint_t)word.len()) {
				String1 = String1.Words(0, i) + " " + word.Left(j) + " " + word.Mid(j) + " " + String1.Words(i + 1);
				word = word.Left(j);
				n = String1.CountWords();
			}
		}

		//debug("word %u='%s'\n", i, word.str());

		for (j = 0; j < NUMBEROF(DayNames); j++) {
			if ((CompareNoCase(word, DayNames[j]) == 0) || (CompareNoCase(word, ShortDayNames[j]) == 0)) {
				break;
			}
		}
		
		if (j < 7) {
			//CurrentTime(dt);
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days += (j + 7 - dmy.WeekDay) % 7;
			if (specified) *specified |= Specified_Day;
		}
		else if (CompareNoCase(word, "now") == 0) {
			CurrentTime(DateTime);
			if (specified) *specified |= Specified_Time | Specified_Date;
		}
		else if (CompareNoCase(word, "today") == 0) {
			CurrentTime(dt);
			DateTime.Days = dt.Days;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "yesterday") == 0) {
			CurrentTime(dt);
			DateTime.Days = dt.Days - 1;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "tomorrow") == 0) {
			CurrentTime(dt);
			DateTime.Days = dt.Days + 1;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "lastmonth") == 0) {
			DateTimeToDMY(DateTime, dmy);

			if ((--dmy.Month) < 1) {
				dmy.Month += NUMBEROF(MonthNames);
				dmy.Year--;
			}

			DateTime.Days -= MonthLengths[dmy.Month - 1] + (((dmy.Month == 2) && ISLEAP(dmy.Year)) ? 1 : 0);
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "nextmonth") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days += MonthLengths[dmy.Month - 1] + (((dmy.Month == 2) && ISLEAP(dmy.Year)) ? 1 : 0);
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "firstdayofmonth") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days -= dmy.Day - 1;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "lastdayofmonth") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days += MonthLengths[dmy.Month] - dmy.Day + (((dmy.Month == 2) && ISLEAP(dmy.Year)) ? 1 : 0);
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "lastweek") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days -= 7;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "nextweek") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days += 7;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "firstdayofweek") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days -= dmy.WeekDay;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "lastdayofweek") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days += 6 - dmy.WeekDay;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "lastday") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days -= 1;
			if (specified) *specified |= Specified_Date;
		}
		else if (CompareNoCase(word, "nextday") == 0) {
			DateTimeToDMY(DateTime, dmy);

			DateTime.Days += 1;
			if (specified) *specified |= Specified_Date;
		}
		else if ((word.Pos("-") > 0) || (word.Pos("/") > 0)) {
			AString word1;
			AString str;
			uint_t day, month, year;

			word.Replace("-", " ");
			word.Replace("/", " ");

			DateTimeToDMY(DateTime, dmy);

			day   = (uint_t)word.Word(0);
			month = dmy.Month;
			year  = dmy.Year;

			uint_t nwords = word.CountWords();
			if (nwords > 2) {
				year = (uint_t)word.Word(2);
				if      (RANGE(year, 80, 99)) year += 1900;
				else if (year <= 79)		  year += 2000;
			}

			if (nwords > 1) {
				word1 = word.Word(1);
				
				for (j = 0; j < NUMBEROF(MonthNames); j++) {
					if ((CompareNoCase(word1, MonthNames[j]) == 0) || (CompareNoCase(word1, ShortMonthNames[j]) == 0)) {
						month = j + 1;
						break;
					}
				}
				
				if (j == NUMBEROF(ShortMonthNames)) month = (uint_t)word1;
			}

			if (RANGE(day, 1, 31) && RANGE(month, 1, 12) && (year >= 2000)) {
				dmy.Day   = day;
				dmy.Month = month;
				dmy.Year  = year;
				DMYToDateTime(dmy, dt);
				DateTime.Days = dt.Days;
				if (specified) *specified |= Specified_Date;
			}

			word.Delete();
		}
		else {
			uint32_t hour = 0, minute = 0, second = 0;
			uint32_t days = 0, ms = 0;
			bool   	 neg = (word.FirstChar() == '-');
			bool   	 pos = (word.FirstChar() == '+');
			sint_t   p;

			word.Replace(":", " ");

			if ((p = word.Pos("d")) >= 0) {
				word = word.Left(p + 1) + " " + word.Mid(p + 1);
			}
			if ((p = word.Pos("h")) >= 0) {
				word = word.Left(p + 1) + " " + word.Mid(p + 1);
			}
			if ((p = word.Pos("m")) >= 0) {
				word = word.Left(p + 1) + " " + word.Mid(p + 1);
			}
			if ((p = word.ToLower().Pos("s")) >= 0) {
				word = word.Left(p + 1) + " " + word.Mid(p + 1);
			}

			bool   valid = false;
			uint_t j, k, nsubwords = word.CountWords();
			for (j = k = 0; j < nsubwords; j++) {
				AString subword = word.Word(j);

				valid |= IsNumeralChar(subword.FirstChar());
				switch (subword.LastChar()) {
					case 'd':
						days   = (uint32_t)fabs((double)subword);
						break;

					case 'h':
						hour   = (uint32_t)(fabs((double)subword) * 3600.0 * 1000.0);
						break;

					case 'm':
						minute = (uint32_t)(fabs((double)subword) *   60.0 * 1000.0);
						break;

					case 's':
						second = (uint32_t)(fabs((double)subword)          * 1000.0);
						break;

					case 'S':
						ms     = (uint32_t)fabs((double)subword);
						break;

					default:
						switch (k++) {
							case 0:
								hour   = (uint32_t)(fabs((double)subword) * 3600.0 * 1000.0);
								break;

							case 1:
								minute = (uint32_t)(fabs((double)subword) *   60.0 * 1000.0);
								break;

							case 2:
								second = (uint32_t)(fabs((double)subword)          * 1000.0);
								break;

							case 3:
								ms     = (uint32_t)fabs((double)subword);
								break;
						}
						break;
				}
			}

			AString suffix;
			if ((suffix = word.Right(2).ToLower()) == "am") {
				hour  = (hour % (12ul * 3600ul * 1000ul));
			}
			else if (suffix == "pm") {
				hour  = (hour % (12ul * 3600ul * 1000ul));
				hour += 12ul * 3600ul * 1000ul;
			}

			ms += hour + minute + second;
			if (ms >= MS_PER_DAY) {
				days += ms / MS_PER_DAY;
				ms   %= MS_PER_DAY;
			}
 
			if (neg) {
				DateTime.Days -= days;
				if (ms > DateTime.MilliSeconds) {
					DateTime.Days--;
					DateTime.MilliSeconds += MS_PER_DAY;
				}
				DateTime.MilliSeconds -= ms;
			}
			else {
				if (!pos) DateTime.MilliSeconds = 0;

				DateTime.Days         += days;
				DateTime.MilliSeconds += ms;

				if (DateTime.MilliSeconds >= MS_PER_DAY) {
					DateTime.Days         += DateTime.MilliSeconds / MS_PER_DAY;
					DateTime.MilliSeconds %= MS_PER_DAY;
				}
			}

			if (specified && valid) *specified |= Specified_Time;
		}
	}

	return *this;
}

AString ADateTime::TimeStr() const
{
	return DateFormat("%h:%m:%s");
}

time_t ADateTime::totime() const
{
	uint32_t days = DateTime.Days + DaysSince1970;
	uint32_t sec  = DateTime.MilliSeconds / 1000;
	return (time_t)(days * SECONDS_PER_DAY + sec);
}

ADateTime& ADateTime::fromtime(const struct tm *tm)
{
	DAYMONTHYEAR dmy;

	dmy.Day   = tm->tm_mday;
	dmy.Month = tm->tm_mon  + 1;
	dmy.Year  = tm->tm_year + 1900;

	DMYToDateTime(dmy, DateTime);
	DateTime.MilliSeconds = ((uint32_t)tm->tm_hour * 3600l + (uint32_t)tm->tm_min * 60l + (uint32_t)tm->tm_sec) * 1000l;

	return *this;
}

ADateTime& ADateTime::fromtime(time_t t)
{
	fromtime(gmtime(&t));

	return *this;
}

ADateTime& ADateTime::fromtime_local(time_t t)
{
	fromtime(localtime(&t));

	return *this;
}

ADateTime ADateTime::UTCToLocal() const
{
	ADateTime res;
	uint32_t ms = DateTime.MilliSeconds % 1000;
	time_t t = totime();

	res.fromtime_local(t);
	res.DateTime.MilliSeconds += ms;
	
	return res;
}

AString ADateTime::ToTimeStamp(uint_t format) const
{
	AString res;

	switch (format) {
		default:
		case TIMESTAMP_FORMAT_TERSE:
			res = DateFormat("%Y%M%D%h%m%s");
			break;

		case TIMESTAMP_FORMAT_FULL:
			res = DateFormat("%Y-%M-%DT%h:%m:%s");
			break;
	}

	if (format & TIMESTAMP_FORMAT_TIMEZONE) {
		// TODO!
	}

	return res;
}

bool ADateTime::FromTimeStamp(const AString& str, bool utc)
{
	uint_t year, month, day, hour, minutes, seconds, ms = 0;
	bool success = false;

	if ((sscanf(str.Word(0).str(), 	   "%4u-%2u-%2uT%2u:%2u:%2u.%3u", &year, &month, &day, &hour, &minutes, &seconds, &ms) >= 6) ||
		(sscanf(str.Words(0, 2).str(), "%4u-%2u-%2u %2u:%2u:%2u.%3u", &year, &month, &day, &hour, &minutes, &seconds, &ms) >= 6) ||
		(sscanf(str.Word(0).str(), 	   "%4u%2u%2u%2u%2u%2u%3u",       &year, &month, &day, &hour, &minutes, &seconds, &ms) >= 6)) {
		int p;

		Set(day, month, year, hour, minutes, seconds);

		DateTime.MilliSeconds += ms;

		if (utc && (((p = str.Pos("+")) >= 0) || ((p = str.Pos("-")) >= 0))) {
			AString _offset = str.Mid(p).SearchAndReplace(":", "");
			int     offset;

			if (sscanf(_offset.str(), "%d", &offset) == 1) {
				bool neg = (offset < 0);

				offset = abs(offset);
				offset = (offset % 100) + ((offset / 100) * 60);
				if (neg) offset = -offset;

				DateTime.MilliSeconds += MS_PER_DAY - offset * 60 * 1000;
				DateTime.Days         += DateTime.MilliSeconds / MS_PER_DAY - 1;
				DateTime.MilliSeconds %= MS_PER_DAY;
			}
		}

		success = true;
	}

	return success;
}

bool operator == (const ADateTime& object1, const ADateTime& object2)
{
	uint64_t a = (uint64_t)object1;
	uint64_t b = (uint64_t)object2;
	return (a == b);
}

bool operator != (const ADateTime& object1, const ADateTime& object2)
{
	uint64_t a = (uint64_t)object1;
	uint64_t b = (uint64_t)object2;
	return (a != b);
}

bool operator <= (const ADateTime& object1, const ADateTime& object2)
{
	uint64_t a = (uint64_t)object1;
	uint64_t b = (uint64_t)object2;
	return (a <= b);
}

bool operator < (const ADateTime& object1, const ADateTime& object2)
{
	uint64_t a = (uint64_t)object1;
	uint64_t b = (uint64_t)object2;
	return (a < b);
}

bool operator >= (const ADateTime& object1, const ADateTime& object2)
{
	uint64_t a = (uint64_t)object1;
	uint64_t b = (uint64_t)object2;
	return (a >= b);
}

bool operator > (const ADateTime& object1, const ADateTime& object2)
{
	uint64_t a = (uint64_t)object1;
	uint64_t b = (uint64_t)object2;
	return (a > b);
}

ADateTime::operator uint64_t() const
{
	return (uint64_t)DateTime.MilliSeconds + (uint64_t)DateTime.Days * MS_PER_DAY;
}

ADateTime::operator sint64_t() const
{
	return (sint64_t)DateTime.MilliSeconds + (sint64_t)DateTime.Days * MS_PER_DAY;
}

const char *ADateTime::GetDayName(uint_t d) const
{
	return (d < NUMBEROF(DayNames)) ? DayNames[d] : NULL;
}

const char *ADateTime::GetShortDayName(uint_t d) const
{
	return (d < NUMBEROF(ShortDayNames)) ? ShortDayNames[d] : NULL;
}

const char *ADateTime::GetMonthName(uint_t m) const
{
	return RANGE(m, 1, NUMBEROF(MonthNames)) ? MonthNames[m - 1] : NULL;
}

const char *ADateTime::GetShortMonthName(uint_t m) const
{
	return RANGE(m, 1, NUMBEROF(ShortMonthNames)) ? ShortMonthNames[m - 1] : NULL;
}

uint_t ADateTime::GetMonthDays(uint_t m) const
{
	return RANGE(m, 1, NUMBEROF(MonthLengths)) ? MonthLengths[m - 1] : 0;
}

uint_t ADateTime::GetMonthStartDay(uint_t m) const
{
	return RANGE(m, 1, NUMBEROF(MonthStartDay)) ? MonthStartDay[m - 1] : 0;
}

int CompareDates(const ADateTime& object1, const ADateTime& object2)
{
	uint64_t d1 = (uint64_t)object1, d2 = (uint64_t)object2;

	if		(d1 < d2) return -1;
	else if (d1 > d2) return 1;
	else              return 0;
}
