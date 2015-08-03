
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

#define DATUM_DAY 1
#define DATUM_YEAR 1980

#define ISLEAP(y) ((((y) % 4) == 0) && ((((y) % 100) != 0) || (((y) % 400) == 0)))
#define DAYSCOUNT(y) (365 * (y) + (SUBZ(y, 1) / 4) - (SUBZ(y, 1) / 100) + (SUBZ(y, 1) / 400))
#define DAYSCOUNTSINCEDATUM(y) SUBZ(DAYSCOUNT(y), DAYSCOUNT(DATUM_YEAR))
#define DAYSINYEAR(y) (ISLEAP(y) ? 366 : 365)

const uint32_t ADateTime::DaysSince1970 = DAYSCOUNT(DATUM_YEAR) - DAYSCOUNT(1970);
#ifdef _WIN32
const uint32_t ADateTime::DaysSince1601 = DAYSCOUNT(DATUM_YEAR) - DAYSCOUNT(1601);
#endif

const ADateTime ADateTime::MinDateTime((uint64_t)0);
const ADateTime ADateTime::MaxDateTime((sizeof(time_t) == sizeof(uint32_t)) ? "20371231235959.999" : "99991231235959.999");

const AString ADateTime::ISO8601Format       = "%Y-%M-%D %h:%m:%s";
const AString ADateTime::ISO8601FormatWithMS = "%Y-%M-%D %h:%m:%s.%S";

bool ADateTime::bDataInit       = false;
bool ADateTime::bDebugStrToDate = false;

const char *ADateTime::DayNames[7] = {
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"Sunday",
};

const char *ADateTime::ShortDayNames[7] = {
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"Sun",
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

	operator = (SUBZ(val1, val2));

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

	t    = DateTime.MilliSeconds / 1000;
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

int ADateTime::FindDay(const char *str, bool utc)
{
	int res;

	if (stricmp(str, "thisday") == 0) return ADateTime().TimeStamp(utc).GetWeekDay();

	for (res = 0; res < (int)NUMBEROF(ShortDayNames); res++) {
		if (stricmp(str, ShortDayNames[res]) == 0) return res;
	}
	for (res = 0; res < (int)NUMBEROF(DayNames); res++) {
		if (stricmp(str, DayNames[res]) == 0) return res;
	}

	return -1;
}

int ADateTime::FindMonth(const char *str, bool utc)
{
	int res;

	if (stricmp(str, "thismonth") == 0) return ADateTime().TimeStamp(utc).GetMonth() - 1;

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

	if (bDebugStrToDate) debug("AddDateTime(%u, %u) result (%u, %u: %s)\n", (uint_t)days, (uint_t)ms, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
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

	if (bDebugStrToDate) debug("SubDateTime(%u, %u) result (%u, %u: %s)\n", (uint_t)days, (uint_t)ms, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
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

	if (bDebugStrToDate) debug("AddDMY(%u, %u, %u) result (%u, %u, %u)\n", years, months, days, dmy.Year, dmy.Month, dmy.Day);
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

	if (bDebugStrToDate) debug("SubDMY(%u, %u, %u) result (%u, %u, %u)\n", years, months, days, dmy.Year, dmy.Month, dmy.Day);
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

	if		(pos) dmy.Year += _n;
	else if (neg) dmy.Year 	= SUBZ(dmy.Year, _n);
	else          dmy.Year 	= MAX(_n, DATUM_YEAR);

	DMYToDateTime(dmy, dt);
	dt.MilliSeconds = ms;

	double _f = n - floor(n);
	if (_f != 0.0) ModifyDays(dt, _f * (double)DAYSINYEAR(dmy.Year), pos || !neg, neg);

	if (bDebugStrToDate) debug("ModifyYear(%0.3lf, %u, %u) result (%u, %u: %s)\n", n, (uint_t)pos, (uint_t)neg, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
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

	if (bDebugStrToDate) debug("ModifyMonth(%0.3lf, %u, %u) result (%u, %u: %s)\n", n, (uint_t)pos, (uint_t)neg, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
}

void ADateTime::ModifyWeeks(DATETIME& dt, double n, bool pos, bool neg) const
{
	if (!pos && !neg) {
		DAYMONTHYEAR dmy;

		DateTimeToDMY(dt, dmy);

		dt.Days         = DAYSCOUNTSINCEDATUM(dmy.Year);
		dt.Days        -= (DATUM_DAY + DAYSCOUNTSINCEDATUM(dmy.Year)) % 7;
		dt.MilliSeconds = 0;

		pos = true;
		if (n >= 1.0) n--;
	}

	ModifyDays(dt, 7.0 * n, pos, neg);

	if (bDebugStrToDate) debug("ModifyWeeks(%0.3lf, %u, %u) result (%u, %u: %s)\n", n, (uint_t)pos, (uint_t)neg, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
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

	if (bDebugStrToDate) debug("ModifyDay(%0.3lf, %u, %u) result (%u, %u: %s)\n", n, (uint_t)pos, (uint_t)neg, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
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

	if (bDebugStrToDate) debug("ModifyDays(%0.3lf, %u, %u) result (%u, %u: %s)\n", n, (uint_t)pos, (uint_t)neg, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
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

	if (bDebugStrToDate) debug("ModifyMS(%ld, %u, %u) result (%u, %u: %s)\n", (long)n, (uint_t)pos, (uint_t)neg, (uint_t)dt.Days, (uint_t)dt.MilliSeconds, ADateTime(dt.Days, dt.MilliSeconds).DateToStr().str());
}

uint_t ADateTime::ParseTerms(const AString& str, ADataList& list) const
{
	uint_t p = 0;

	// set destructor for supplied ADataList
	list.SetDestructor(&__DeleteTerm);

	// ignore whitespace and commas
	while (IsWhiteSpace(str[p]) || (str[p] == ',')) p++;

	// split string into a series of terms made up from:
	// 1. val: a value (AValue)
	// 2. valstr: the string representation of the value (AString)
	// 3. str: a non-value string *after* the number
	//
	// val can be invalid meaning no value was found
	while (str[p]) {
		TERM *term;
		uint_t p0 = p;

		if ((term = new TERM) != NULL) {
			// attempt to decode value at p
			p = term->val.EvalNumber(str, p, false);

			// copy string representation of value
			term->valstr.Create(str.str() + p0, p - p0);

			// reset starting point
			p0 = p;
			
			// extract string terminator

			// allow single character terminator of those types below
			if ((str[p] == '/') || (str[p] == '-') || (str[p] == '+') || (str[p] == ':')) p++;

			// collect all whitespace and/or commas together
			else if (IsWhiteSpace(str[p]) || (str[p] == ',')) p++;

			// collect all alpha characters together
			else while (IsAlphaChar(str[p])) p++;

			// create string from terminator
			term->str.Create(str.str() + p0, p - p0);

			list.Add((uptr_t)term);
		}

		// if nothing decoded or extracted, abort
		if (p == p0) break;
	}

	return list.Count();
}

ADateTime& ADateTime::StrToDate(const AString& String, uint_t relative, uint_t *specified, AString *errors)
{
	ADataList terms;
	uint_t    _specified = 0;
	bool      utc = false;		// assume local times initially

	// delete any errors
	if (errors) errors->Delete();

	// split string into terms
	ParseTerms(String, terms);

	// Set date/time starting point
	switch (relative) {
		default:
		case Time_Absolute:
			// time supplied is assumed to be absolute
			memset(&DateTime, 0, sizeof(DateTime));
			break;

		case Time_Existing:
			// leave current value as is
			break;

		case Time_Relative_Today_UTC:
			// time is relative to midnight (UTC), today
			utc = true;
		case Time_Relative_Today:
			// time is relative to midnight, today
			CurrentTime(DateTime, utc);
			// reset to midnight
			DateTime.MilliSeconds = 0;
			break;

		case Time_Relative_UTC:
			// time is relative to now (UTC)
			utc = true;
		case Time_Relative_Local:
			// time is relative to now (local)
			CurrentTime(DateTime, utc);
			break;
	}

	bool   pos = false, neg = false;
	uint_t i, n = terms.Count();
	const TERM **termlist = (const TERM **)terms.List();

	if (bDebugStrToDate) {
		// debug: output all terms
		debug("All terms from '%s':\n", String.str());
		for (i = 0; i < terms.Count(); i++) {
			const AValue&  val 	  = termlist[i]->val;
			const AString& str 	  = termlist[i]->str;
			const AString& valstr = termlist[i]->valstr;

			debug("%u/%u: '%s' ('%s') / '%s'\n",
				  i, n,
				  val.IsValid() ? val.GenerateStringNice().str() : "<invalid>", valstr.str(),
				  str.str());
		}
		debug("Terms as they are decoded:\n");
	}
	
	for (i = 0; i < terms.Count(); i++) {
		const AValue&  val 	  = termlist[i]->val;
		const AString& str 	  = termlist[i]->str;
		const AString& valstr = termlist[i]->valstr;
		int ind;

		if (bDebugStrToDate) {
			// debug: output terms as they are decoded
			debug("%u/%u: '%s' ('%s') / '%s' (%s, pos=%u, neg=%u)\n",
				  i, n,
				  val.IsValid() ? val.GenerateStringNice().str() : "<invalid>", valstr.str(),
				  str.str(),
				  DateToStr().str(),
				  (uint_t)pos, (uint_t)neg);
		}
		
		if (!val.IsValid()) {
			// no value supplied -> look at string
			if		(str == "+") {pos = true;  neg = false;}	// additive mode
			else if	(str == "-") {pos = false; neg = true;}		// subtractive mode
			else if (str == "utc")   utc = true;				// utc mode
			else if (str == "local") utc = false;				// local mode
			else if	(str == "now") {
				// now in local or UTC
				DATETIME dt;

				CurrentTime(dt, utc);

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else if	(str == "min") {
				// minimal value
				const DATETIME& dt = ADateTime::MinDateTime.DateTime;

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else if	(str == "max") {
				// maximal value
				const DATETIME& dt = ADateTime::MaxDateTime.DateTime;

				if		(pos) AddDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else if (neg) SubDateTime(DateTime, dt.Days, dt.MilliSeconds);
				else		  DateTime = dt;

				_specified |= Specified_Date | Specified_Time;
			}
			else if	(str == "today") {
				// UTC or local today (midnight)
				DATETIME dt;

				CurrentTime(dt, utc);

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	(str == "tomorrow") {
				// UTC or local tomorrow (midnight)
				DATETIME dt;

				CurrentTime(dt, utc);

				dt.Days++;

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	(str == "yesterday") {
				// UTC or local yesterday (midnight)
				DATETIME dt;

				CurrentTime(dt, utc);

				dt.Days = SUBZ(dt.Days, 1);

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	(str == "firstdayofmonth") {
				// UTC or local first day of this month
				DATETIME dt;
				DAYMONTHYEAR dmy;

				CurrentTime(dt, utc);
				DateTimeToDMY(dt, dmy);

				dt.Days -= dmy.Day;

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	(str == "lastdayofmonth") {
				// UTC or local last day of this month
				DATETIME dt;
				DAYMONTHYEAR dmy;

				CurrentTime(dt, utc);
				DateTimeToDMY(dt, dmy);

				dt.Days += GetMonthLength(dmy) - 1 - dmy.Day;

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	(str == "firstdayofweek") {
				// UTC or local first day of this week
				DATETIME dt;
				DAYMONTHYEAR dmy;

				CurrentTime(dt, utc);
				DateTimeToDMY(dt, dmy);

				dt.Days -= dmy.WeekDay;

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	(str == "lastdayofweek") {
				// UTC or local first day of this week
				DATETIME dt;
				DAYMONTHYEAR dmy;

				CurrentTime(dt, utc);
				DateTimeToDMY(dt, dmy);

				dt.Days += 6 - dmy.WeekDay;

				if		(pos) AddDateTime(DateTime, dt.Days, 0);
				else if (neg) SubDateTime(DateTime, dt.Days, 0);
				else		  DateTime.Days = dt.Days;

				_specified |= Specified_Date;
			}
			else if	((str == "noon") || (str == "midday")) {
				// noon
				if		(pos) AddDateTime(DateTime, 0, MS_PER_DAY / 2);
				else if (neg) SubDateTime(DateTime, 0, MS_PER_DAY / 2);
				else		  DateTime.MilliSeconds = MS_PER_DAY / 2;

				_specified |= Specified_Time;
			}
			else if	(str == "midnight") {
				// midnight
				// midnight is 0 so no need to handle the additive or subtractive case
				if (!pos && !neg) DateTime.MilliSeconds = 0;

				_specified |= Specified_Time;
			}
			else if	(str == "utctolocal") {
				// convert current time, taken as UTC, to local
				*this = UTCToLocal();
				utc   = false;
				_specified |= Specified_Time;
			}
			else if	(str == "localtoutc") {
				// convert current time, taken as local, to UTC
				// this is NOT 100% accurate at all times!
				*this = LocalToUTC();
				utc   = true;
				_specified |= Specified_Time;
			}
			else if ((ind = FindDay(str, utc)) >= 0) {
				// day name (long or short) used
				DAYMONTHYEAR dmy;

				DateTimeToDMY(DateTime, dmy);

				DateTime.Days += (ind + 7 - dmy.WeekDay) % 7;

				_specified |= Specified_Day;
			}
			else if ((ind = FindMonth(str, utc)) >= 0) {
				// month name (long or short) used
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
				// handle {next|last} {week|month|year|<day>|<monthname>}
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
				else if ((ind = FindDay(str2, utc)) >= 0) {
					DAYMONTHYEAR dmy;

					if (str == "next") DateTime.Days += 7;
					else			   DateTime.Days  = SUBZ(DateTime.Days, 7);

					DateTimeToDMY(DateTime, dmy);

					DateTime.Days += (ind + 7 - dmy.WeekDay) % 7;

					_specified |= Specified_Day;
				}
				else if ((ind = FindMonth(str2, utc)) >= 0) {
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
				else {
					if (errors) errors->printf("Unrecognized next/last string '%s' in date\n", str2.str());
					else		debug("Unrecognized next/last string '%s' in date\n", str2.str());
					break;
				}
			}
			// if empty string terminator, reset to absolute mode
			else if (str.Words(0).Empty() || (str == ",")) pos = neg = false;
			else {
				if (errors) errors->printf("Unrecognized string '%s' in date\n", str.str());
				else		debug("Unrecognized string '%s' in date\n", str.str());
				break;
			}
		}
		// y or Y suffix specifies year
		else if ((str == "y") || (str == "Y")) {
			ModifyYear(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		// M (only, m is used for minutes) suffix specifies month
		else if (str == "M") {
			ModifyMonth(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		// d or D suffix specifies day
		else if ((str == "d") || (str == "D")) {
			ModifyDay(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		// w or W suffix specifies week
		// w or W prefix specifies week of year (ISO8601 style)
		else if ((str == "w") || (str == "W")) {
			// look for W as prefix to two digit week or three digit week and day
			if (((i + 1) < n) &&
				termlist[i + 1]->val.IsValid() &&
				(termlist[i + 1]->str.Words(0).Empty() || (termlist[i + 1]->str.Words(0) == ","))) {
				ModifyYear(DateTime, (double)val, pos, neg);

				double w = (double)termlist[++i]->val;
				// three digit week and day (2 digit week plus 1 digit day)
				if (w >= 100.0) {
					ModifyWeeks(DateTime, floor(w * .1), pos, neg);
					// if pos=neg=false (absolute day within week), subtract 1 from value
					// and set pos to true to switch to relative mode
					ModifyDays(DateTime, fmod(w, 10.0) - (!(pos || neg) ? 1.0 : 0.0), (pos || !neg), neg);
				}
				// two digit week
				else ModifyWeeks(DateTime, w, pos, neg);
			}
			// W is suffix so just modify weeks
			else ModifyWeeks(DateTime, (double)val, pos, neg);

			_specified |= Specified_Date;
		}
		// h suffix specifies hours
		else if (str == "h") {
			if (!pos && !neg) DateTime.MilliSeconds = 0;
			ModifyMS(DateTime, (sint64_t)((double)val * 3600.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		// m suffix specifies minutes
		else if (str == "m") {
			if (!pos && !neg) DateTime.MilliSeconds -= DateTime.MilliSeconds % (3600ul * 1000ul);
			ModifyMS(DateTime, (sint64_t)((double)val * 60.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		// s suffix specifies seconds
		else if (str == "s") {
			if (!pos && !neg) DateTime.MilliSeconds -= DateTime.MilliSeconds % (60ul * 1000ul);
			ModifyMS(DateTime, (sint64_t)((double)val * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		// S suffix specifies milliseconds
		else if (str == "S") {
			if (!pos && !neg) DateTime.MilliSeconds -= DateTime.MilliSeconds % 1000ul;
			ModifyMS(DateTime, (sint64_t)val, pos, neg);

			_specified |= Specified_Time;
		}
		// am suffix
		else if (str == "am") {
			if (!pos && !neg) DateTime.MilliSeconds = 0;
			ModifyMS(DateTime, (sint64_t)(fmod((double)val, 12.0) * 3600.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		// pm suffix
		else if (str == "pm") {
			if (!pos && !neg) DateTime.MilliSeconds = 0;
			ModifyMS(DateTime, (sint64_t)((12.0 + fmod((double)val, 12.0)) * 3600.0 * 1000.0), pos, neg);

			_specified |= Specified_Time;
		}
		// no suffix means either decimal hours or 14 digit explicit date and time
		else if (str.Words(0).Empty() || (str == ",")) {
			DAYMONTHYEAR dmy;
			uint_t h, m;
			double s;

			// detect 14 digit explicit date and time
			if ((valstr.len() >= 14) &&
				(sscanf(valstr.str(), "%4u%2u%2u%2u%2u%lf",
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

				// look for +hhmm or -hhmm, evaluate then *counteract* the value to move from local to UTC
				if (((i + 2) < n) && 
					(termlist[i]->str == " ") &&
					!termlist[i + 1]->val.IsValid() &&
					((termlist[i + 1]->str == "+") || (termlist[i + 1]->str == "-")) &&
					(termlist[i + 2]->val.IsInteger() && (termlist[i + 2]->valstr.len() == 4))) {
					uint_t 	 _offset = (uint_t)termlist[i + 2]->val;
					uint_t 	 hours   = _offset / 100;
					uint_t 	 mins    = _offset % 100;
					uint32_t offset  = ((uint32_t)hours * 60 + (uint32_t)mins) * 60000;

					if (termlist[i + 1]->str == "+") SubDateTime(DateTime, 0, offset);
					else						     AddDateTime(DateTime, 0, offset);

					i += 2;
				}

				_specified |= Specified_Date | Specified_Time;
			}
			else {
				// if absolute mode, reset milliseconds to 0
				if (!pos && !neg) DateTime.MilliSeconds = 0;

				// modify milliseconds by value as hours
				ModifyMS(DateTime, (sint64_t)((double)val * 3600.0 * 1000.0), pos, neg);

				_specified |= Specified_Time;
			}
			
			pos = neg = false;
		}
		// date and time specified as [[[YYYY:]MM:]DD:]hh[:mm[:ss[.SSS]]]
		else if ((str == ":") && ((i + 1) < n) && termlist[i + 1]->val.IsValid()) {
			double vals[6];
			uint_t j;

			memset(vals, 0, sizeof(vals));

			// find number of terms with ':' as suffix
			for (j = 1; (((i + j + 1) < n) &&
						 (j < (NUMBEROF(vals) - 1)) &&
						 termlist[i + j]->val.IsValid() &&
						 (termlist[i + j]->str == ":") &&
						 termlist[i + j + 1]->val.IsValid()); j++) ;

			// evaluate terms into the correct locations:
			// terms entries
			//   6   YYYY:MM:DD:hh:mm:ss.SSS
			//   5   MM:DD:hh:mm:ss.SSS
			//   4   DD:hh:mm:ss.SSS
			//   3   hh:mm:ss.SSS
			//   2   hh:mm
			//   1   hh
			uint_t nterms = j + 1;
			for (j = 0; j < nterms; j++) {
				vals[NUMBEROF(vals) - nterms - (nterms <= 2) + j] = (double)termlist[i + j]->val;
			}

			i += nterms - 1;

			// Modify year/month/day depending on which values are non-zero
			if (vals[0] != 0.0) ModifyYear(DateTime,  vals[0], pos, neg);
			if (vals[1] != 0.0) ModifyMonth(DateTime, vals[1], pos, neg);
			if (vals[2] != 0.0) ModifyDay(DateTime,   vals[2], pos, neg);

			if (nterms >= 4) {
				// date is only specified if 4 or more terms are specified
				_specified |= Specified_Date;
			}

			// look for am or pm suffix on last term
			if		(termlist[i]->str == "am") vals[3] = fmod(vals[3], 12.0);
			else if (termlist[i]->str == "pm") vals[3] = 12.0 + fmod(vals[3], 12.0);

			// calculate ms into day
			uint32_t ms = (uint32_t)(vals[3] * 3600.0 * 1000.0 + vals[4] * 60.0 * 1000.0 + vals[5] * 1000.0);
			if (ms) {
				// update time
				if		(pos) AddDateTime(DateTime, 0, ms);
				else if (neg) SubDateTime(DateTime, 0, ms);
				else {
					DateTime.MilliSeconds  = ms;
					DateTime.Days         += DateTime.MilliSeconds / MS_PER_DAY;
					DateTime.MilliSeconds %= MS_PER_DAY;
				}
				
				_specified |= Specified_Time;
			}

			// look for +hhmm or -hhmm, evaluate then *counteract* the value to move from local to UTC
			if (((i + 2) < n) && 
				(termlist[i]->str == " ") &&
				!termlist[i + 1]->val.IsValid() &&
				((termlist[i + 1]->str == "+") || (termlist[i + 1]->str == "-")) &&
				(termlist[i + 2]->val.IsInteger() && (termlist[i + 2]->valstr.len() == 4))) {
				uint_t 	 _offset = (uint_t)termlist[i + 2]->val;
				uint_t 	 hours   = _offset / 100;
				uint_t 	 mins    = _offset % 100;
				uint32_t offset  = ((uint32_t)hours * 60 + (uint32_t)mins) * 60000;

				// +hhmm should be *subtracted*
 				if (termlist[i + 1]->str == "+") SubDateTime(DateTime, 0, offset);
				// -hhmm should be *added*
				else						     AddDateTime(DateTime, 0, offset);

				i += 2;
			}
		}
		// date specified as [[YYYY-]MM-]DD or DD[-MM[-YYYY]] or [[YYYY/]MM/]DD or DD[/MM[/YYYY]]
		else if (((str == "-") || (str == "/")) && ((i + 1) < n)) {
			AValue  val2 = termlist[++i]->val;
			AString str2 = termlist[i]->str;
			uint_t  inc  = 0;

			// detect year first
			if ((uint_t)val >= DATUM_YEAR) {
				ModifyYear(DateTime, (double)val, false, false);

				_specified |= Specified_Date;
			}
			// or year third
			else if (val2.IsValid() && (str2 == str) && ((i + 1) < n) && termlist[i + 1]->val.IsValid()) {
				ModifyYear(DateTime, (double)termlist[i + 1]->val, false, false);
				inc = 1;

				_specified |= Specified_Date;
			}
			// detect year third when month name is used which results in an extra term
			else if (!val2.IsValid() && ((i + 2) < n) &&
					 !termlist[i + 1]->val.IsValid() && (termlist[i + 1]->str == str) &&
					 termlist[i + 2]->val.IsValid()) {
				ModifyYear(DateTime, (double)termlist[i + 2]->val, false, false);
				inc = 2;

				_specified |= Specified_Date;
			}

			// second value is a number, assume it is a month
			// (either YYYY-MM or DD-MM)
			if (val2.IsValid()) {
				ModifyMonth(DateTime, (double)val2, false, false);

				_specified |= Specified_Date;
			}
			// else month name
			else if ((ind = FindMonth(str2, utc)) >= 0) {
				DAYMONTHYEAR dmy;

				uint32_t ms = DateTime.MilliSeconds;
				DateTimeToDMY(DateTime, dmy);

				dmy.Month = ind;

				DMYToDateTime(dmy, DateTime);
				DateTime.MilliSeconds = ms;

				_specified |= Specified_Date;
			}

			// detect when first value is day of month
			if ((uint_t)val < DATUM_YEAR) {
				ModifyDay(DateTime, (double)val, false, false);

				_specified |= Specified_Date;
			}
			// or month value was used and third value is present
			else if (val2.IsValid() && (str2 == str) && ((i + 1) < n) && termlist[i + 1]->val.IsValid()) {
				ModifyDay(DateTime, (double)termlist[i + 1]->val, false, false);
				inc = 1;
				
				_specified |= Specified_Date;
			}
			// or month name was used (hence extra term used) and third value is present
			else if (!val2.IsValid() && ((i + 2) < n) &&
					 !termlist[i + 1]->val.IsValid() && (termlist[i + 1]->str == str) &&
					 termlist[i + 2]->val.IsValid()) {
				ModifyDay(DateTime, (double)termlist[i + 2]->val, false, false);
				inc = 2;

				_specified |= Specified_Date;
			}
			// detect YYYY-Www or YYYY-Www-d form used (ISO8601)
			else if (((uint_t)val >= DATUM_YEAR) &&
					 !val2.IsValid() && ((i + 1) < n) &&
					 ((str2 == "W") || (str2 == "w")) &&
					 termlist[i + 1]->val.IsValid()) {
				// set week of year
				ModifyWeeks(DateTime, (double)termlist[i + 1]->val, false, false);
				inc = 1;

				// detect day on end
				if ((termlist[i + 1]->str == str) && ((i + 2) < n) &&
					termlist[i + 2]->val.IsValid()) {
					double d = (double)termlist[i + 2]->val;
					if (d >= 1.0) d -= 1.0;
					ModifyDays(DateTime, d, true, false);
					inc = 2;
				}
				
				_specified |= Specified_Date;
			}
			else {
				if (errors) errors->printf("Unrecognized string '%s' in date\n", str2.str());
				else		debug("Unrecognized string '%s' in date\n", str2.str());
				break;
			}

			i += inc;
		}
		else {
			if (errors) errors->printf("Unrecognized string '%s' in date\n", str.str());
			else		debug("Unrecognized string '%s' in date\n", str.str());
			break;
		}

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
