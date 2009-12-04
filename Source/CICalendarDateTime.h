/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/*
	CICalendarDateTime.h

	Author:
	Description:	<describe the CICalendarDateTime class here>
*/

#ifndef CICalendarDateTime_H
#define CICalendarDateTime_H

#include "CICalendarDefinitions.h"
#include "CICalendarTimezone.h"

#include <stdint.h>
#include <ostream>
#include <vector>

#include "cdstring.h"


namespace iCal {

typedef uint32_t	CICalendarRef;	// Unique reference to object
class CICalendarDuration;

class CICalendarDateTime
{
public:
	enum EDayOfWeek
	{
		eSunday,
		eMonday,
		eTuesday,
		eWednesday,
		eThursday,
		eFriday,
		eSaturday
	};

	enum ELocaleDate
	{
		eFullDate,
		eAbbrevDate,
		eNumericDate,
		
		eFullDateNoYear,
		eAbbrevDateNoYear,
		eNumericDateNoYear
	};

	CICalendarDateTime()
		{ _init_CICalendarDateTime(); }
	explicit CICalendarDateTime(int32_t packed, const CICalendarTimezone* tzid = NULL);
	CICalendarDateTime(int32_t year, int32_t month, int32_t day, const CICalendarTimezone* tzid = NULL)
		{ _init_CICalendarDateTime(); mYear = year; mMonth = month; mDay = day; mDateOnly = true; if (tzid != NULL) mTimezone = *tzid; }
	CICalendarDateTime(int32_t year, int32_t month, int32_t day, int32_t hours, int32_t minutes, int32_t seconds, const CICalendarTimezone* tzid = NULL)
		{ _init_CICalendarDateTime(); mYear = year; mMonth = month; mDay = day; mHours = hours; mMinutes = minutes; mSeconds = seconds; if (tzid != NULL) mTimezone = *tzid; }
	CICalendarDateTime(const CICalendarDateTime& copy)
		{ _copy_CICalendarDateTime(copy); }
	virtual ~CICalendarDateTime() {}

	CICalendarDateTime& operator=(const CICalendarDateTime& copy)
		{ if (this != &copy) _copy_CICalendarDateTime(copy); return *this; }

	CICalendarDateTime operator+(const CICalendarDuration& duration) const;
	CICalendarDuration operator-(const CICalendarDateTime& date) const;

	int operator==(const CICalendarDateTime& comp) const
		{ return CompareDateTime(comp) == 0 ? 1 : 0; }
	int operator!=(const CICalendarDateTime& comp) const
		{ return CompareDateTime(comp) != 0 ? 1 : 0; }
	int operator>=(const CICalendarDateTime& comp) const
		{ return CompareDateTime(comp) >= 0 ? 1 : 0; }
	int operator<=(const CICalendarDateTime& comp) const
		{ return CompareDateTime(comp) <= 0 ? 1 : 0; }
	int operator>(const CICalendarDateTime& comp) const
		{ return CompareDateTime(comp) > 0 ? 1 : 0; }
	int operator<(const CICalendarDateTime& comp) const
		{ return CompareDateTime(comp) < 0 ? 1 : 0; }

	int CompareDateTime(const CICalendarDateTime& comp) const;
	bool CompareDate(const CICalendarDateTime& comp) const
		{ return (mYear == comp.mYear) && (mMonth == comp.mMonth) && (mDay == comp.mDay); }
	int64_t GetPosixTime() const;

	bool IsDateOnly() const
		{ return mDateOnly; }
	void SetDateOnly(bool date_only)
		{ mDateOnly = date_only; Changed(); }

	int32_t GetYear() const
		{ return mYear; }
	void SetYear(int32_t year)
		{ if (mYear != year) { mYear = year; Changed(); } }
	void OffsetYear(int32_t diff_year)
		{ mYear += diff_year; Normalise(); }

	int32_t GetMonth() const
		{ return mMonth; }
	const cdstring& GetMonthText(bool short_txt = true) const;
	void SetMonth(int32_t month)
		{ if (mMonth != month) { mMonth = month; Changed(); } }
	void OffsetMonth(int32_t diff_month)
		{ mMonth += diff_month; Normalise(); }

	int32_t GetDay() const
		{ return mDay; }
	void SetDay(int32_t day)
		{ if (mDay != day) { mDay = day; Changed(); } }
	void OffsetDay(int32_t diff_day)
		{ mDay += diff_day; Normalise(); }

	void SetYearDay(int32_t day);
	int32_t GetYearDay() const;

	void SetMonthDay(int32_t day);
	bool IsMonthDay(int32_t day) const;

	void SetWeekNo(int32_t weekno);
	int32_t GetWeekNo() const;
	bool IsWeekNo(int32_t weekno) const;

	void SetDayOfWeekInYear(int32_t offset, EDayOfWeek day);
	void SetDayOfWeekInMonth(int32_t offset, EDayOfWeek day);
	bool IsDayOfWeekInMonth(int32_t offset, EDayOfWeek day) const;

	EDayOfWeek GetDayOfWeek() const;
	const cdstring& GetDayOfWeekText(EDayOfWeek day) const;

	void SetHHMMSS(int32_t hours, int32_t minutes, int32_t seconds)
		{ if ((mHours != hours) || (mMinutes != minutes) || (mSeconds != seconds)) { mHours = hours; mMinutes = minutes; mSeconds = seconds; Changed(); } }

	int32_t GetHours() const
		{ return mHours; }
	void SetHours(int32_t hours)
		{ if (mHours != hours) { mHours = hours; Changed(); } }
	void OffsetHours(int32_t diff_hour)
		{ mHours += diff_hour; Normalise(); }

	int32_t GetMinutes() const
		{ return mMinutes; }
	void SetMinutes(int32_t minutes)
		{ if (mMinutes != minutes) { mMinutes = minutes; Changed(); } }
	void OffsetMinutes(int32_t diff_minutes)
		{ mMinutes += diff_minutes; Normalise(); }

	int32_t GetSeconds() const
		{ return mSeconds; }
	void SetSeconds(int32_t seconds)
		{ if (mSeconds != seconds) { mSeconds = seconds; Changed(); } }
	void OffsetSeconds(int32_t diff_seconds)
		{ mSeconds += diff_seconds; Normalise(); }

	const CICalendarTimezone& GetTimezone() const
		{ return mTimezone; }
	CICalendarTimezone& GetTimezone()
		{ return mTimezone; }
	void SetTimezone(const CICalendarTimezone& tzid)
		{ mTimezone = tzid; Changed(); }
	void AdjustTimezone(const CICalendarTimezone& tzid);
	void AdjustToUTC();

	CICalendarDateTime GetAdjustedTime() const;
	CICalendarDateTime GetAdjustedTime(const CICalendarTimezone& tzid) const;

	void SetToday()
		{ *this = GetToday(&mTimezone); }
	static CICalendarDateTime GetToday(const CICalendarTimezone* tzid = NULL);
	void SetNow()
		{ *this = GetNow(&mTimezone); }
	static CICalendarDateTime GetNow(const CICalendarTimezone* tzid = NULL);
	void SetNowUTC()
		{ *this = GetNowUTC(); }
	static CICalendarDateTime GetNowUTC();

	void Recur(ERecurrence_FREQ freq, int32_t interval);

	cdstring GetLocaleDate(ELocaleDate locale) const;
	cdstring GetTime(bool with_seconds, bool am_pm, bool tzid = false) const;
	cdstring GetLocaleDateTime(ELocaleDate locale, bool with_seconds, bool am_pm, bool tzid = false) const;
	cdstring GetText() const;

	void Parse(const cdstring& data);
	void Generate(std::ostream& os) const;
	void GenerateRFC2822(std::ostream& os) const;

protected:
	int32_t			mYear;		// full 4-digit year
	int32_t			mMonth;		// 1...12
	int32_t			mDay;		// 1...31

	int32_t			mHours;		// 0...23
	int32_t			mMinutes;	// 0...59
	int32_t			mSeconds;	// 0...60

	bool			mDateOnly;

	CICalendarTimezone	mTimezone;

	mutable std::pair<bool, int64_t>	mPosixTime;

	void		Normalise();

	int32_t		TimeZoneSecondsOffset() const
		{ return mTimezone.TimeZoneSecondsOffset(*this); }
	cdstring	TimeZoneDescriptor() const
		{ return mTimezone.TimeZoneDescriptor(*this); }

private:
	void _init_CICalendarDateTime();
	void _copy_CICalendarDateTime(const CICalendarDateTime& copy);

	void Changed() const
		{ mPosixTime.first = false; }
	
	int32_t	DaysSince1970() const;
};

typedef std::vector<CICalendarDateTime> CICalendarDateTimeList;

}	// namespace iCal

#endif	// CICalendarDateTime_H
