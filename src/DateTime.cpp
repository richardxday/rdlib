
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
#define DAYSCOUNTSINCEDATUM(y) (DAYSCOUNT(y) - DAYSCOUNT(DATUM_YEAR))
#define DAYSINYEAR(y) (ISLEAP(y) ? 366 : 365)

const uint32_t ADateTime::DaysSince1970 = DAYSCOUNT(DATUM_YEAR) - DAYSCOUNT(1970);
#ifdef _WIN32
const uint32_t ADateTime::DaysSince1601 = DAYSCOUNT(DATUM_YEAR) - DAYSCOUNT(1601);
#endif

const ADateTime ADateTime::MinDateTime(0ull);
const ADateTime ADateTime::MaxDateTime("31-dec-2037 23:59:59");

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

uint_t ADateTime::MonthStartDay[13] = {0};

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

ADateTime::ADateTime(const AString& str, uint_t relative, uint_t *specified)
{
	InitData();

	StrToDate(str, relative, specified);
}

void ADateTime::InitData()
{
	if (!bDataInit) {
		bDataInit = true;

		uint i, n = 0;
		for (i = 0; i < NUMBEROF(MonthLengths); i++) {
			MonthStartDay[i] = n;
			n += MonthLengths[i];
		}
		MonthStartDay[i] = n;

		assert(n == 365);
	}
}

uint_t ADateTime::GetMonthLength(uint_t m, uint_t y) const
{
	return MonthLengths[m] + (ISLEAP(y) && (m == 1));
}

uint_t ADateTime::GetMonthStart(uint_t m, uint_t y) const
{
	return MonthStartDay[m] + (ISLEAP(y) && (m >= 2));
}

uint_t ADateTime::GetMonthEnd(uint_t m, uint_t y) const
{
	return MonthStartDay[m + 1] + (ISLEAP(y) && (m >= 1)) - 1;
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
	dmy.Day   = SUBZ((uint_t)st.wDay, 1);
	dmy.Month = SUBZ((uint_t)st.wMonth, 1);
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
	operator = (operator uint64_t() + (uint64_t)object);

	return *this;
}

ADateTime& ADateTime::operator -= (const ADateTime& object)
{
	uint64_t val1 = operator uint64_t();
	uint64_t val2 = (uint64_t)object;

	if (val1 >= val2) val1 -= val2;
	else			  val1  = 0;

	operator = (val1);

	return *this;
}

ADateTime& ADateTime::Set(uint_t day, uint_t month, uint_t year, uint_t hour, uint_t minute, uint_t second)
{
	DAYMONTHYEAR dmy;

	memset(&dmy, 0, sizeof(dmy));

	dmy.Day   = SUBZ(day, 1);
	dmy.Month = SUBZ(month, 1);
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

uint_t ADateTime::GetWeekDay() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.WeekDay;
}

uint_t ADateTime::GetDay() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.Day + 1;
}

uint_t ADateTime::GetMonth() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.Month + 1;
}

uint_t ADateTime::GetYear() const
{
	DAYMONTHYEAR dmy;
	DateTimeToDMY(DateTime, dmy);
	return dmy.Year;
}

uint_t ADateTime::GetHours() const
{
	return (uint_t)(DateTime.MilliSeconds / (1000 * 60 * 60));
}

uint_t ADateTime::GetMinutes() const
{
	return (uint_t)((DateTime.MilliSeconds / (1000 * 60)) % 60);
}

uint_t ADateTime::GetSeconds() const
{
	return (uint_t)((DateTime.MilliSeconds / 1000) % 60);
}

uint_t ADateTime::GetMilliSeconds() const
{
	return (uint_t)(DateTime.MilliSeconds % 1000);
}

void ADateTime::DateTimeToDMY(const DATETIME& dt, DAYMONTHYEAR& dmy) const
{
	uint32_t days = dt.Days + (dt.MilliSeconds / MS_PER_DAY);
	uint_t inc;

	memset(&dmy, 0, sizeof(dmy));

	dmy.WeekDay = (days + DATUM_DAY) % 7;

	dmy.Year = DATUM_YEAR;
	inc = 128;
	while (inc && (days > DAYSCOUNTSINCEDATUM(dmy.Year))) {
		if (days >= DAYSCOUNTSINCEDATUM(dmy.Year + inc)) dmy.Year += inc;
		else inc >>= 1;
	}
	
	days -= DAYSCOUNTSINCEDATUM(dmy.Year);

	assert(days <= 366);

	dmy.Month = NUMBEROF(MonthLengths) >> 1;
	inc = dmy.Month >> 1;
	while ((days < GetMonthStart(dmy)) || (days > GetMonthEnd(dmy))) {
		if (days < GetMonthStart(dmy)) dmy.Month = SUBZ(dmy.Month, inc);
		else						   dmy.Month = MIN(dmy.Month + inc, NUMBEROF(MonthLengths) - 1);
		inc = (inc + 1) >> 1;
	}
	days -= GetMonthStart(dmy);

	assert(dmy.Month < NUMBEROF(MonthLengths));
	assert(days < 31);

	dmy.Day = days;
}

void ADateTime::DMYToDateTime(const DAYMONTHYEAR& dmy, DATETIME& dt) const
{
	DAYMONTHYEAR dmy1 = dmy;

	memset(&dt, 0, sizeof(dt));

	dmy1.Year  += dmy1.Month / NUMBEROF(MonthLengths);
	dmy1.Month %= NUMBEROF(MonthLengths);

	dmy1.Year = MAX(dmy1.Year, DATUM_YEAR);
	dt.Days   = DAYSCOUNTSINCEDATUM(dmy1.Year);

	dt.Days  += GetMonthStart(dmy1);
	dt.Days  += dmy1.Day;
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
				  ShortDayNames[dmy.WeekDay], dmy.Day + 1, ShortMonthNames[dmy.Month], dmy.Year,
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

void ADateTime::AddDateTime(DATETIME& dt, uint32_t days, uint32_t ms) const
{
	days += ms / MS_PER_DAY;
	ms   %= MS_PER_DAY;

	if (days) dt.Days   += days;
	if (ms) {
		dt.MilliSeconds += ms;
		dt.Days         += dt.MilliSeconds / MS_PER_DAY;
		dt.MilliSeconds %= MS_PER_DAY;
	}
}

void ADateTime::SubDateTime(DATETIME& dt, uint32_t days, uint32_t ms) const
{
	days += ms / MS_PER_DAY;
	ms   %= MS_PER_DAY;

	if (days) dt.Days = SUBZ(dt.Days, days);
	if (ms) {
		if (dt.MilliSeconds < ms) {
			dt.MilliSeconds += MS_PER_DAY;
			dt.Days          = SUBZ(dt.Days, 1);
		}
		dt.MilliSeconds = SUBZ(dt.MilliSeconds, ms);
	}
}

void ADateTime::AddDMY(DAYMONTHYEAR& dmy, uint_t years, uint_t months, uint_t days) const
{
	if (days) {
		DATETIME dt;

		DMYToDateTime(dmy, dt);
		AddDateTime(dt, days, 0);
		DateTimeToDMY(dt, dmy);
	}

	dmy.Month += months % NUMBEROF(MonthLengths);
	dmy.Year  += (months / NUMBEROF(MonthLengths)) + (dmy.Month / NUMBEROF(MonthLengths)) + years;
	dmy.Month %= NUMBEROF(MonthLengths);
}

void ADateTime::SubDMY(DAYMONTHYEAR& dmy, uint_t years, uint_t months, uint_t days) const
{
	if (days) {
		DATETIME dt;

		DMYToDateTime(dmy, dt);
		SubDateTime(dt, days, 0);
		DateTimeToDMY(dt, dmy);
	}

	if (months) {
		dmy.Year  = SUBZ(dmy.Year, months / NUMBEROF(MonthLengths));
		months   %= NUMBEROF(MonthLengths);
		if (dmy.Month < months) {
			dmy.Month += NUMBEROF(MonthLengths);
			dmy.Year   = SUBZ(dmy.Year, 1);
		}
		dmy.Month -= months;
	}
	dmy.Year = SUBZ(dmy.Year, years);
	dmy.Year = MAX(dmy.Year, DATUM_YEAR);
}

void ADateTime::ModifyYear(DATETIME& dt, double n, bool pos, bool neg) const
{
	DAYMONTHYEAR dmy;
	uint32_t ms = dt.MilliSeconds;

	if ((n < 0.0) && (pos || neg)) {
		n   = -n;
		pos = !pos;
		neg = !neg;
	}
	else n = MAX(n, 0.0);

	uint_t _n = (uint_t)n;
	
	DateTimeToDMY(dt, dmy);

	if		(pos) AddDMY(dmy, _n, 0, 0);
	else if (neg) SubDMY(dmy, _n, 0, 0);
	else          dmy.Year = MAX(_n, DATUM_YEAR);

	DMYToDateTime(dmy, dt);
	dt.MilliSeconds = ms;

	double _f = n - floor(n);
	if (_f != 0.0) ModifyDays(dt, _f * (double)DAYSINYEAR(dmy.Year), pos || !neg, neg);
}

void ADateTime::ModifyMonth(DATETIME& dt, double n, bool pos, bool neg) const
{
	DAYMONTHYEAR dmy;
	uint32_t ms = dt.MilliSeconds;

	if ((n < 0.0) && (pos || neg)) {
		n   = -n;
		pos = !pos;
		neg = !neg;
	}
	else n = MAX(n, 0.0);

	uint_t _n = (uint_t)n;
	
	DateTimeToDMY(dt, dmy);
	if		(pos) AddDMY(dmy, 0, _n, 0);
	else if (neg) SubDMY(dmy, 0, _n, 0);
	else {
		_n = SUBZ(_n, 1);
		dmy.Month  = _n % NUMBEROF(MonthLengths);
		dmy.Year  += _n / NUMBEROF(MonthLengths);
	}

	DMYToDateTime(dmy, dt);
	dt.MilliSeconds = ms;

	double _f = n - floor(n);
	if (_f != 0.0) ModifyDays(dt, _f * (double)GetMonthLength(dmy), pos || !neg, neg);
}

void ADateTime::ModifyDay(DATETIME& dt, double n, bool pos, bool neg) const
{
	if (!pos && !neg) {
		DAYMONTHYEAR dmy;
	
		DateTimeToDMY(dt, dmy);

		n  -= (double)(dmy.Day + 1);
		pos = true;
	}
	
	ModifyDays(dt, n, pos, neg);
}

void ADateTime::ModifyDays(DATETIME& dt, double n, bool pos, bool neg) const
{
	if ((n < 0.0) && (pos || neg)) {
		n   = -n;
		pos = !pos;
		neg = !neg;
	}
	else n = MAX(n, 0.0);

	uint32_t days = (uint32_t)n;
	uint32_t ms   = (uint32_t)((n - (double)days) * 24.0 * 3600.0 * 1000.0);

	if		(pos) AddDateTime(dt, days, ms);
	else if (neg) SubDateTime(dt, days, ms);
	else {
		dt.Days          = days;
		dt.MilliSeconds  = ms;
	}
}

void ADateTime::ModifyMS(DATETIME& dt, sint64_t n, bool pos, bool neg) const
{
	pos |= !neg;

	if (n < 0) {
		n   = -n;
		pos = !pos;
		neg = !neg;
	}

	uint32_t days = n / MS_PER_DAY;
	uint32_t ms   = n % MS_PER_DAY;
	
	if (pos) AddDateTime(dt, days, ms);
	else     SubDateTime(dt, days, ms);
}

uint_t ADateTime::ParseTerms(const AString& str, ADataList& list) const
{
	uint_t p = 0;

	list.SetDestructor(&__DeleteTerm);

	while (IsWhiteSpace(str[p]) || (str[p] == ',')) p++;

	while (str[p]) {
		TERM *term;
		uint_t p0 = p;

		if ((term = new TERM) != NULL) {
			uint_t p0 = term->val.EvalNumber(str, p, false);

			term->numstr = str.Mid(p, p0 - p);
			
			p = p0;
			if ((str[p] == '/') || (str[p] == '-') || (str[p] == '+') || (str[p] == ':')) p++;
			else if (IsWhiteSpace(str[p]) || (str[p] == ',')) p++;
			else while (IsAlphaChar(str[p])) p++;
			
			term->str.Create(str.str() + p0, p - p0);

			list.Add((uptr_t)term);
		}
		
		if (p == p0) break;
	}

	return list.Count();
}

ADateTime& ADateTime::StrToDate(const AString& String, uint_t relative, uint_t *specified)
{
	ADataList terms;
	uint_t    _specified = 0;

	ParseTerms(String, terms);

	switch (relative) {
		default:
		case Time_Absolute:
			memset(&DateTime, 0, sizeof(DateTime));
			break;

		case Time_Existing:
			break;

		case Time_Relative_Today:
		case Time_Relative_Today_UTC:
			CurrentTime(DateTime, (relative == Time_Relative_Today_UTC));
			DateTime.MilliSeconds = 0;
			break;

		case Time_Relative_UTC:
		case Time_Relative_Local:
			CurrentTime(DateTime, (relative == Time_Relative_UTC));
			break;
	}

	uint_t i, n = terms.Count();
	bool   pos = false, neg = false;
	const TERM **termlist = (const TERM **)terms.List();
	for (i = 0; i < terms.Count(); i++) {
		const AValue&  val 	  = termlist[i]->val;
		const AString& str 	  = termlist[i]->str;
		const AString& numstr = termlist[i]->numstr;
		int ind;

#if 0
		debug("%u/%u: '%s' / '%s' ('%s', %s, pos=%u, neg=%u)\n",
			  i, n,
			  val.IsValid() ? val.GenerateStringNice().str() : "<invalid>",
			  str.str(),
			  termlist[i]->numstr.str(),
			  DateToStr().str(),
			  (uint_t)pos, (uint_t)neg);
#endif
		
		if (!val.IsValid()) {
			if		(str == "+") {pos = true;  neg = false;}
			else if	(str == "-") {pos = false; neg = true;}
			else if	((str == "now") || (str == "utc")) {
				DATETIME dt;

				CurrentTime(dt, (str == "utc"));

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else if	(str == "min") {
				const DATETIME& dt = ADateTime::MinDateTime.DateTime;

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else if	(str == "max") {
				const DATETIME& dt = ADateTime::MaxDateTime.DateTime;

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else if	((str == "today") || (str == "todayutc")) {
				DATETIME dt;

				CurrentTime(dt, (str == "todayutc"));

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	((str == "tomorrow") || (str == "tomorrowutc")) {
				DATETIME dt;

				CurrentTime(dt, (str == "tomorrowutc"));

				dt.Days++;

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	((str == "yesterday") || (str == "yesterdayutc")) {
				DATETIME dt;

				CurrentTime(dt, (str == "yesterdayutc"));

				dt.Days = SUBZ(dt.Days, 1);

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	((str == "noon") || (str == "midday")) {
				if		(pos) AddDateTime(DateTime, 0, MS_PER_DAY / 2);
				else if (neg) SubDateTime(DateTime, 0, MS_PER_DAY / 2);
				else		  DateTime.MilliSeconds = MS_PER_DAY / 2;

				_specified |= Specified_Time;
			}
			else if	(str == "midnight") {
				if (!pos && !neg) DateTime.MilliSeconds = 0;

				_specified |= Specified_Time;
			}
			else if	(str == "utctolocal") {
				*this = UTCToLocal();
				_specified |= Specified_Time;
			}
			else if	(str == "localtoutc") {
				*this = LocalToUTC();
				_specified |= Specified_Time;
			}
			else if ((ind = FindDay(str)) >= 0) {
				DAYMONTHYEAR dmy;

				DateTimeToDMY(DateTime, dmy);

				DateTime.Days += (ind + 7 - dmy.WeekDay) % 7;

				_specified |= Specified_Day;
			}
			else if ((ind = FindMonth(str)) >= 0) {
				DAYMONTHYEAR dmy;

				uint32_t ms = DateTime.MilliSeconds;
				DateTimeToDMY(DateTime, dmy);

				dmy.Month = ind;

				DMYToDateTime(dmy, DateTime);
				DateTime.MilliSeconds = ms;

				_specified |= Specified_Date;
			}
			else if (((str == "next") || (str == "last")) &&
					 ((i + 2) < n) &&
					 !termlist[i + 1]->val.IsValid() &&
					 (termlist[i + 1]->str.Words(0).Empty() || (termlist[i + 1]->str == ",")) &&
					 !termlist[i + 2]->val.IsValid()) {
				const AString& str2 = termlist[i + 2]->str;
				
				i += 2;

				if (str2 == "week") {
					if (str == "next") DateTime.Days += 7;
					else			   DateTime.Days  = SUBZ(DateTime.Days, 7);

					_specified |= Specified_Date;
				}
				else if (str2 == "month") {
					ModifyMonth(DateTime, 1.0, (str == "next"), (str == "last"));

					_specified |= Specified_Date;
				}
				else if (str2 == "year") {
					ModifyYear(DateTime, 1.0, (str == "next"), (str == "last"));

					_specified |= Specified_Date;
				}
				else if ((ind = FindDay(str2)) >= 0) {
					DAYMONTHYEAR dmy;

					if (str == "next") DateTime.Days += 7;
					else			   DateTime.Days  = SUBZ(DateTime.Days, 7);

					DateTimeToDMY(DateTime, dmy);

					DateTime.Days += (ind + 7 - dmy.WeekDay) % 7;

					_specified |= Specified_Day;
				}
				else if ((ind = FindMonth(str2)) >= 0) {
					DAYMONTHYEAR dmy;

					uint32_t ms = DateTime.MilliSeconds;
					DateTimeToDMY(DateTime, dmy);

					if (str == "next") dmy.Year++;
					else			   dmy.Year = SUBZ(dmy.Year, 1);
					dmy.Year   = MAX(dmy.Year, DATUM_YEAR);

					dmy.Month += (ind + NUMBEROF(MonthLengths) - dmy.Month) % NUMBEROF(MonthLengths);

					DMYToDateTime(dmy, DateTime);
					DateTime.MilliSeconds = ms;

					_specified |= Specified_Date;
				}
				else debug("Unrecognized next/last string '%s' in date\n", str2.str());
			}
			else if (str.Words(0).Empty() || (str == ",")) pos = neg = false;
			else debug("Unrecognized string '%s' in date\n", str.str());
		}
		else if ((str == "y") || (str == "Y")) {
			ModifyYear(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		else if (str == "M") {
			ModifyMonth(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		else if ((str == "d") || (str == "D")) {
			ModifyDay(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		else if (str == "w") {
			ModifyDays(DateTime, (double)val * 7.0, pos, neg);

			_specified |= Specified_Date;
		}
		else if (str == "h") {
			if (!pos && !neg) DateTime.MilliSeconds = 0;
			ModifyMS(DateTime, (sint64_t)((double)val * 3600.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		else if (str == "m") {
			if (!pos && !neg) DateTime.MilliSeconds -= DateTime.MilliSeconds % (3600ul * 1000ul);
			ModifyMS(DateTime, (sint64_t)((double)val * 60.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		else if (str == "s") {
			if (!pos && !neg) DateTime.MilliSeconds -= DateTime.MilliSeconds % (60ul * 1000ul);
			ModifyMS(DateTime, (sint64_t)((double)val * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		else if (str == "S") {
			if (!pos && !neg) DateTime.MilliSeconds -= DateTime.MilliSeconds % 1000ul;
			ModifyMS(DateTime, (sint64_t)val, pos, neg);

			_specified |= Specified_Time;
		}
		else if (str == "am") {
			if (!pos && !neg) DateTime.MilliSeconds = 0;
			ModifyMS(DateTime, (sint64_t)(fmod((double)val, 12.0) * 3600.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		else if (str == "pm") {
			if (!pos && !neg) DateTime.MilliSeconds = 0;
			ModifyMS(DateTime, (sint64_t)((12.0 + fmod((double)val, 12.0)) * 3600.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		else if (str.Words(0).Empty() || (str == ",")) {
			DAYMONTHYEAR dmy;
			uint_t h, m;
			double s;
			
			if ((numstr.len() >= 14) &&
				(sscanf(numstr.str(), "%4u%2u%2u%2u%2u%lf",
						&dmy.Year, &dmy.Month, &dmy.Day,
						&h, &m, &s) == 6)) {
				DATETIME dt;

				dmy.Month = SUBZ(dmy.Month, 1);
				dmy.Day   = SUBZ(dmy.Day, 1);
				
				DMYToDateTime(dmy, dt);

				dt.MilliSeconds = ((uint32_t)h * 60 + (uint32_t)m) * 60000 + (uint32_t)(s * 1000.0);

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else {
				if (!pos && !neg) DateTime.MilliSeconds = 0;
				ModifyMS(DateTime, (sint64_t)((double)val * 3600.0 * 1000.0), pos, neg);

				_specified |= Specified_Time;
			}
			
			pos = neg = false;
		}
		else if ((str == ":") && ((i + 1) < n) && termlist[i + 1]->val.IsValid()) {
			double vals[6];
			uint_t j;

			memset(vals, 0, sizeof(vals));

			for (j = 1; (((i + j + 1) < n) &&
						 (j < (NUMBEROF(vals) - 1)) &&
						 termlist[i + j]->val.IsValid() &&
						 (termlist[i + j]->str == ":") &&
						 termlist[i + j + 1]->val.IsValid()); j++) ;

			uint_t nterms = j + 1;
			for (j = 0; j < nterms; j++) {
				vals[NUMBEROF(vals) - nterms - (nterms <= 2) + j] = (double)termlist[i + j]->val;
			}

			i += nterms - 1;

			if (nterms >= 6) ModifyYear(DateTime,  vals[0], pos, neg);
			if (nterms >= 5) ModifyMonth(DateTime, vals[1], pos, neg);
			if (nterms >= 4) ModifyDay(DateTime,   vals[2], pos, neg);

			if (nterms >= 4) {
				_specified |= Specified_Date;
			}

			if		(termlist[i]->str == "am") vals[3] = fmod(vals[3], 12.0);
			else if (termlist[i]->str == "pm") vals[3] = 12.0 + fmod(vals[3], 12.0);

			uint32_t ms = (uint32_t)(vals[3] * 3600.0 * 1000.0 + vals[4] * 60.0 * 1000.0 + vals[5] * 1000.0);
			if (ms) {
				if		(pos) AddDateTime(DateTime, 0, ms);
				else if (neg) SubDateTime(DateTime, 0, ms);
				else {
					DateTime.MilliSeconds  = ms;
					DateTime.Days         += DateTime.MilliSeconds / MS_PER_DAY;
					DateTime.MilliSeconds %= MS_PER_DAY;
				}
				
				_specified |= Specified_Time;
			}
		}
		else if (((str == "-") || (str == "/")) && ((i + 1) < n)) {
			const AValue&  val2 = termlist[++i]->val;
			const AString& str2 = termlist[i]->str;
			uint_t inc = 0;

			if ((uint_t)val >= DATUM_YEAR) {
				ModifyYear(DateTime, (double)val, false, false);

				_specified |= Specified_Date;
			}
			else if (val2.IsValid() && (str2 == str) && ((i + 1) < n) && termlist[i + 1]->val.IsValid()) {
				ModifyYear(DateTime, (double)termlist[i + 1]->val, false, false);
				inc = 1;

				_specified |= Specified_Date;
			}
			else if (!val2.IsValid() && ((i + 2) < n) &&
					 !termlist[i + 1]->val.IsValid() && (termlist[i + 1]->str == str) &&
					 termlist[i + 2]->val.IsValid()) {
				ModifyYear(DateTime, (double)termlist[i + 2]->val, false, false);
				inc = 2;

				_specified |= Specified_Date;
			}

			if (val2.IsValid()) {
				ModifyMonth(DateTime, (double)val2, false, false);

				_specified |= Specified_Date;
			}
			else if ((ind = FindMonth(str2)) >= 0) {
				DAYMONTHYEAR dmy;

				uint32_t ms = DateTime.MilliSeconds;
				DateTimeToDMY(DateTime, dmy);

				dmy.Month = ind;

				DMYToDateTime(dmy, DateTime);
				DateTime.MilliSeconds = ms;

				_specified |= Specified_Date;
			}

			if ((uint_t)val < DATUM_YEAR) {
				ModifyDay(DateTime, (double)val, false, false);

				_specified |= Specified_Date;
			}
			else if (val2.IsValid() && (str2 == str) && ((i + 1) < n) && termlist[i + 1]->val.IsValid()) {
				ModifyDay(DateTime, (double)termlist[i + 1]->val, false, false);
				inc = 1;

				_specified |= Specified_Date;
			}
			else if (!val2.IsValid() && ((i + 2) < n) &&
					 !termlist[i + 1]->val.IsValid() && (termlist[i + 1]->str == str) &&
					 termlist[i + 2]->val.IsValid()) {
				ModifyDay(DateTime, (double)termlist[i + 2]->val, false, false);
				inc = 2;

				_specified |= Specified_Date;
			}

			i += inc;
		}
		else debug("Unrecognized string '%s' in date\n", str.str());

		if		(termlist[i]->str == "+") {pos = true;  neg = false;}
		else if (termlist[i]->str == "-") {pos = false; neg = true;}
	}

	if (specified) *specified = _specified;

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

	dmy.Day   = SUBZ(tm->tm_mday, 1);
	dmy.Month = tm->tm_mon;
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

sint64_t ADateTime::GetUTCOffset() const
{
	time_t t = totime();
	return (uint64_t)ADateTime().fromtime_local(t) - (uint64_t)ADateTime().fromtime(t);
}

ADateTime ADateTime::UTCToLocal() const
{
	return operator uint64_t() + GetUTCOffset();
}

ADateTime ADateTime::LocalToUTC() const
{
	return operator uint64_t() - GetUTCOffset();
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
	bool   success = false;

	if ((sscanf(str.Word(0).str(), 	   "%4u-%2u-%2uT%2u:%2u:%2u.%3u", &year, &month, &day, &hour, &minutes, &seconds, &ms) >= 6) ||
		(sscanf(str.Words(0, 2).str(), "%4u-%2u-%2u %2u:%2u:%2u.%3u", &year, &month, &day, &hour, &minutes, &seconds, &ms) >= 6) ||
		(sscanf(str.Word(0).str(), 	   "%4u%2u%2u%2u%2u%2u%3u",       &year, &month, &day, &hour, &minutes, &seconds, &ms) >= 6)) {
		int p;

		Set(day, month, year, hour, minutes, seconds);

		DateTime.MilliSeconds += ms;

		if (utc && (((p = str.Pos(" +")) >= 0) || ((p = str.Pos(" -")) >= 0))) {
			AString _offset = str.Mid(p + 1).SearchAndReplace(":", "");
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
