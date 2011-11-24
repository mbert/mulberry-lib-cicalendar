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
	CICalendarDateTime.cpp

	Author:
	Description:	<describe the CICalendarDateTime class here>
*/

#include "CICalendarDateTime.h"

#include "CICalendar.h"
#include "CICalendarDuration.h"
#include "CICalendarLocale.h"
#include "CICalendarManager.h"
#include "CICalendarUtils.h"

#include <ctime>
#include <iomanip>
#include <cstring>
#include <cstdio>

using namespace iCal;

CICalendarDateTime::CICalendarDateTime(int32_t packed, const CICalendarTimezone* tzid)
{
	_init_CICalendarDateTime();

	// Unpack a packed date
	mYear = CICalendarUtils::UnpackDateYear(packed);
	mMonth = CICalendarUtils::UnpackDateMonth(packed);
	mDay = CICalendarUtils::UnpackDateDay(packed);
	if (mDay < 0)
		mDay = -mDay;
	mDateOnly = true;
	
	if (tzid)
		mTimezone = *tzid;

	Normalise();
}

void CICalendarDateTime::_init_CICalendarDateTime()
{
	mYear = 1970;
	mMonth = 1;
	mDay = 1;

	mHours = 0;
	mMinutes = 0;
	mSeconds = 0;

	mDateOnly = false;

	mPosixTime.first = false;
	mPosixTime.second = 0;
}

void CICalendarDateTime::_copy_CICalendarDateTime(const CICalendarDateTime& copy)
{
	mYear = copy.mYear;
	mMonth = copy.mMonth;
	mDay = copy.mDay;

	mHours = copy.mHours;
	mMinutes = copy.mMinutes;
	mSeconds = copy.mSeconds;

	mDateOnly = copy.mDateOnly;

	mTimezone = copy.mTimezone;

	mPosixTime = copy.mPosixTime;
}

CICalendarDateTime CICalendarDateTime::operator+(const CICalendarDuration& duration) const
{
	// Add duration seconds to temp object and normalise it
	CICalendarDateTime result(*this);
	result.mSeconds += duration.GetTotalSeconds();
	result.Changed();
	result.Normalise();

	return result;
}

CICalendarDuration CICalendarDateTime::operator-(const CICalendarDateTime& date) const
{
	// Look for floating
	if (GetTimezone().Floating() || date.GetTimezone().Floating())
	{
		// Adjust the floating ones to fixed
		CICalendarDateTime copy1(*this);
		CICalendarDateTime copy2(date);
		
		if (copy1.GetTimezone().Floating() && copy2.GetTimezone().Floating())
		{
			// Set both to UTC and do comparison
			copy1.GetTimezone().SetUTC(true);
			copy2.GetTimezone().SetUTC(true);
			return copy1 - copy2;
		}
		else if (copy1.GetTimezone().Floating())
		{
			// Set to be the same
			copy1.SetTimezone(copy2.GetTimezone());
			return copy1 - copy2;
		}
		else
		{
			// Set to be the same
			copy2.SetTimezone(copy1.GetTimezone());
			return copy1 - copy2;
		}
	}
	else
	{
		// Do diff of date-time in seconds
		int32_t diff = GetPosixTime() - date.GetPosixTime();
		return CICalendarDuration(diff);
	}
}

void CICalendarDateTime::SetYearDay(int32_t day)
{
	// 1 .. 366 offset from start, or
	// -1 .. -366 offset from end
	
	if (day > 0)
	{
		// Offset current date to 1st January of current year
		mMonth = 1;
		mDay = 1;
		
		// Increment day
		mDay += day - 1;
		
		// Normalise to get proper month/day values
		Normalise();
	}
	else if (day < 0)
	{
		// Offset current date to 1st January of next year
		mYear++;
		mMonth = 1;
		mDay = 1;
		
		// Decrement day
		mDay += day;
		
		// Normalise to get proper year/month/day values
		Normalise();
	}
}

int32_t CICalendarDateTime::GetYearDay() const
{
	return mDay + CICalendarUtils::DaysUptoMonth(mMonth, mYear);
}

void CICalendarDateTime::SetMonthDay(int32_t day)
{
	// 1 .. 31 offset from start, or
	// -1 .. -31 offset from end
	
	if (day > 0)
	{
		// Offset current date to 1st of current month
		mDay = 1;
		
		// Increment day
		mDay += day - 1;
		
		// Normalise to get proper month/day values
		Normalise();
	}
	else if (day < 0)
	{
		// Offset current date to 1st of next month
		mMonth++;
		mDay = 1;
		
		// Decrement day
		mDay += day;
		
		// Normalise to get proper year/month/day values
		Normalise();
	}
}

bool CICalendarDateTime::IsMonthDay(int32_t day) const
{
	if (day > 0)
		return mDay == day;
	else if (day < 0)
		return mDay - 1 - CICalendarUtils::DaysInMonth(mMonth, mYear) == day;
	else
		return false;
}

void CICalendarDateTime::SetWeekNo(int32_t weekno)
{
	// This is the iso 8601 week number definition
	
	// What day does the current year start on
	CICalendarDateTime temp(mYear, 1, 1);
	EDayOfWeek first_day = temp.GetDayOfWeek();
	
	// Calculate and set yearday for start of week
	switch(first_day)
	{
	case eSunday:
	case eMonday:
	case eTuesday:
	case eWednesday:
	case eThursday:
		SetYearDay((weekno - 1) * 7 - first_day);
	case eFriday:
	case eSaturday:
		SetYearDay((weekno - 1) * 7 - first_day + 7);
	}
}

int32_t CICalendarDateTime::GetWeekNo() const
{
	// This is the iso 8601 week number definition
	
	// What day does the current year start on
	CICalendarDateTime temp(mYear, 1, 1);
	EDayOfWeek first_day = temp.GetDayOfWeek();
	
	// Get days upto the current one
	int32_t yearday = GetYearDay();
	
	switch(first_day)
	{
	case eSunday:
	case eMonday:
	case eTuesday:
	case eWednesday:
	case eThursday:
	default:
		return (yearday + first_day) / 7 + 1;
	case eFriday:
	case eSaturday:
		return (yearday + first_day - 7) / 7 + 1;
	}
}

bool CICalendarDateTime::IsWeekNo(int32_t weekno) const
{
	// This is the iso 8601 week number definition
	
	if (weekno > 0)
		return GetWeekNo() == weekno;
	else
		// This needs to calculate the negative offset from the last week in the current year
		return false;
}

void CICalendarDateTime::SetDayOfWeekInYear(int32_t offset, EDayOfWeek day)
{
	// Set to first day in year
	mMonth = 1;
	mDay = 1;

	// Determine first weekday in year
	int32_t first_day = GetDayOfWeek();
	
	if (offset > 0)
	{
		int32_t cycle = (offset - 1) * 7 + day;
		cycle -= first_day;
		if (first_day > day)
			cycle += 7;
		mDay = cycle + 1;
	}
	else if (offset < 0)
	{
		first_day += 365 + (CICalendarUtils::IsLeapYear(mYear) ? 1 : 0) - 1;
		first_day %= 7;
		
		int32_t cycle = (-offset - 1) * 7 - day;
		cycle += first_day;
		if (day > first_day)
			cycle += 7;
		mDay = 365 + (CICalendarUtils::IsLeapYear(mYear) ? 1 : 0) - cycle;
	}
	
	Normalise();
}

void CICalendarDateTime::SetDayOfWeekInMonth(int32_t offset, EDayOfWeek day)
{
	// Set to first day in month
	mDay = 1;

	// Determine first weekday in month
	int32_t first_day = GetDayOfWeek();
	
	if (offset > 0)
	{
		int32_t cycle = (offset - 1) * 7 + day;
		cycle -= first_day;
		if (first_day > day)
			cycle += 7;
		mDay = cycle + 1;
	}
	else if (offset < 0)
	{
		first_day += CICalendarUtils::DaysInMonth(mMonth, mYear) - 1;
		first_day %= 7;
		
		int32_t cycle = (-offset - 1) * 7 - day;
		cycle += first_day;
		if (day > first_day)
			cycle += 7;
		mDay = CICalendarUtils::DaysInMonth(mMonth, mYear) - cycle;
	}
	
	Normalise();
}

bool CICalendarDateTime::IsDayOfWeekInMonth(int32_t offset, EDayOfWeek day) const
{
	// First of the actual day must match
	if (GetDayOfWeek() != day)
		return false;
	
	// If there is no count the we match any of this day in the month
	if (offset == 0)
		return true;
	
	// Create temp date-time with the appropriate parameters and then compare
	CICalendarDateTime temp(*this);
	temp.SetDayOfWeekInMonth(offset, day);
	
	// Now compare dates
	return CompareDate(temp);
}

CICalendarDateTime::EDayOfWeek CICalendarDateTime::GetDayOfWeek() const
{
	// Count days since 01-Jan-1970 which was a Thursday
	int32_t result = eThursday + DaysSince1970();
	result %= 7;
	if (result < 0)
		result += 7;

	return static_cast<EDayOfWeek>(result);
}

const cdstring& CICalendarDateTime::GetMonthText(bool short_txt) const
{
	// Make sure range is valid
	if ((mMonth < 1) || (mMonth > 12))
		return cdstring::null_str;
	else
		return CICalendarLocale::GetMonth(mMonth, short_txt ? CICalendarLocale::eShort : CICalendarLocale::eLong);
}

const cdstring& CICalendarDateTime::GetDayOfWeekText(EDayOfWeek day) const
{
	return CICalendarLocale::GetDay(day, CICalendarLocale::eShort);
}

int CICalendarDateTime::CompareDateTime(const CICalendarDateTime& comp) const
{
	// If either are date only, then just do date compare
	if (mDateOnly || comp.mDateOnly)
	{
		if (mYear == comp.mYear)
		{
			if (mMonth == comp.mMonth)
			{
				if (mDay == comp.mDay)
					return 0;
				else
					return mDay < comp.mDay ? -1 : 1;
			}
			else
				return mMonth < comp.mMonth ? -1 : 1;
		}
		else
			return mYear < comp.mYear ? -1 : 1;
	}
	
	// If they have the same timezone do simple compare - no posix calc needed
	else if (GetTimezone() == comp.GetTimezone())
	{
		if (mYear == comp.mYear)
		{
			if (mMonth == comp.mMonth)
			{
				if (mDay == comp.mDay)
				{
					if (mHours == comp.mHours)
					{
						if (mMinutes == comp.mMinutes)
						{
							if (mSeconds == comp.mSeconds)
							{
								return 0;
							}
							else
								return mSeconds < comp.mSeconds ? -1 : 1;
						}
						else
							return mMinutes < comp.mMinutes ? -1 : 1;
					}
					else
						return mHours < comp.mHours ? -1 : 1;
				}
				else
					return mDay < comp.mDay ? -1 : 1;
			}
			else
				return mMonth < comp.mMonth ? -1 : 1;
		}
		else
			return mYear < comp.mYear ? -1 : 1;
	}
	else
	{
		int64_t posix1 = GetPosixTime();
		int64_t posix2 = comp.GetPosixTime();
		if (posix1 == posix2)
			return 0;
		else
			return posix1 < posix2 ? -1 : 1;
	}
}

int64_t CICalendarDateTime::GetPosixTime() const
{
	// Look for cached value (or floating time which has to be calculated each time)
	if (!mPosixTime.first || GetTimezone().Floating())
	{
		int64_t result = 0;

		// Add hour/mins/secs
		result = (mHours * 60LL + mMinutes) * 60LL + mSeconds;

		// Number of days since 1970
		result += DaysSince1970() * 24LL * 60LL * 60LL;

		// Adjust for timezone offset
		result -= TimeZoneSecondsOffset();

		// Now indcate cache state
		mPosixTime.first = true;
		mPosixTime.second = result;
	}

	return mPosixTime.second;
}

int32_t	CICalendarDateTime::DaysSince1970() const
{
	// Add days betweenn 1970 and current year (ignoring leap days)
	int32_t result = (mYear - 1970) * 365L;

	// Add leap days between years
	result += CICalendarUtils::LeapDaysSince1970(mYear - 1970);
	
	// Add days in current year up to current month (includes leap day for current year as needed)
	result += CICalendarUtils::DaysUptoMonth(mMonth, mYear);
	
	// Add days in month
	result += mDay - 1;
	
	return result;
}

void CICalendarDateTime::Normalise()
{
	// Normalise seconds
	int32_t normalised_secs = mSeconds % 60;
	int32_t adjustment_mins = mSeconds / 60;
	if (normalised_secs < 0)
	{
		normalised_secs += 60;
		adjustment_mins--;
	}
	mSeconds = normalised_secs;
	mMinutes += adjustment_mins;

	// Normalise minutes
	int32_t normalised_mins = mMinutes % 60;
	int32_t adjustment_hours = mMinutes / 60;
	if (normalised_mins < 0)
	{
		normalised_mins += 60;
		adjustment_hours--;
	}
	mMinutes = normalised_mins;
	mHours += adjustment_hours;

	// Normalise hours
	int32_t normalised_hours = mHours % 24;
	int32_t adjustment_days = mHours / 24;
	if (normalised_hours < 0)
	{
		normalised_hours += 24;
		adjustment_days--;
	}
	mHours = normalised_hours;
	mDay += adjustment_days;

	// Wipe the time if date only
	if (mDateOnly)
	{
		mSeconds = mMinutes = mHours = 0;
	}

	// Adjust the month first, since the day adjustment is month dependent

	// Normalise month
	int32_t normalised_month = ((mMonth - 1) % 12) + 1;
	int32_t adjustment_year = (mMonth - 1) / 12;
	if ((normalised_month - 1) < 0)
	{
		normalised_month += 12;
		adjustment_year--;
	}
	mMonth = normalised_month;
	mYear += adjustment_year;

	// Now do days
	if (mDay > 0)
	{
		while(mDay > CICalendarUtils::DaysInMonth(mMonth, mYear))
		{
			mDay -= CICalendarUtils::DaysInMonth(mMonth, mYear);
			mMonth++;
			if (mMonth > 12)
			{
				mMonth = 1;
				mYear++;
			}
		}
	}
	else
	{
		while(mDay <= 0)
		{
			mMonth--;
			if (mMonth < 1)
			{
				mMonth = 12;
				mYear--;
			}
			mDay += CICalendarUtils::DaysInMonth(mMonth, mYear);
		}
	}

	// Always invalidate posix time cache	
	Changed();
}

// Adjust from one time zone to another and do date-time offset
void CICalendarDateTime::AdjustTimezone(const CICalendarTimezone& tzid)
{
	// Only if different
	if (!(tzid == GetTimezone()))
	{
		int32_t offset_from = TimeZoneSecondsOffset();
		SetTimezone(tzid);
		int32_t offset_to = TimeZoneSecondsOffset();
		
		OffsetSeconds(offset_to - offset_from);
	}
}

// Make sure its in UTC time
void CICalendarDateTime::AdjustToUTC()
{
	if (!mTimezone.GetUTC())
	{
		CICalendarTimezone utc(true);

		int32_t offset_from = TimeZoneSecondsOffset();
		SetTimezone(utc);
		int32_t offset_to = TimeZoneSecondsOffset();
		
		OffsetSeconds(offset_to - offset_from);
	}
}

CICalendarDateTime CICalendarDateTime::GetAdjustedTime() const
{
	return GetAdjustedTime(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
}

CICalendarDateTime CICalendarDateTime::GetAdjustedTime(const CICalendarTimezone& tzid) const
{
	// Copy this and adjust to input timezone
	CICalendarDateTime adjusted(*this);
	adjusted.AdjustTimezone(tzid);
	return adjusted;
}

// Get current date (in local timezone)
CICalendarDateTime CICalendarDateTime::GetToday(const CICalendarTimezone* tzid)
{
	// Get from posix time
	time_t now = time(NULL);
	struct tm* now_tm = std::localtime(&now);
	
	return CICalendarDateTime(now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday, tzid);
}

// Get current date-time (in local timezone)
CICalendarDateTime CICalendarDateTime::GetNow(const CICalendarTimezone* tzid)
{
	CICalendarDateTime utc = GetNowUTC();
	utc.AdjustTimezone(tzid != NULL ? *tzid : CICalendarTimezone());
	
	return utc;
}

// Get current date-time (in local timezone)
CICalendarDateTime CICalendarDateTime::GetNowUTC()
{
	// Get from posix time
	time_t now = time(NULL);
	struct tm* now_tm = std::gmtime(&now);
	CICalendarTimezone tzid(true);
	
	return CICalendarDateTime(now_tm->tm_year + 1900, now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour, now_tm->tm_min, now_tm->tm_sec, &tzid);
}

void CICalendarDateTime::Recur(ERecurrence_FREQ freq, int32_t interval)
{
	// Add appropriate interval
	switch(freq)
	{
	case eRecurrence_SECONDLY:
		mSeconds += interval;
		break;
	case eRecurrence_MINUTELY:
		mMinutes += interval;
		break;
	case eRecurrence_HOURLY:
		mHours += interval;
		break;
	case eRecurrence_DAILY:
		mDay += interval;
		break;
	case eRecurrence_WEEKLY:
		mDay += 7 * interval;
		break;
	case eRecurrence_MONTHLY:
		// Iterate until a valid day in the next month is found.
		// This can happen if adding one month to e.g. 31 January.
		// That is an undefined operation - does it mean 28/29 February
		// or 1/2 May, or 31 March or what? We choose to find the next month with
		// the same day number as the current one.
		do
		{
			mMonth += interval;
			
			// Normalise month
			if ((mMonth < 1) || (mMonth > 12))
			{
				int32_t normalised_month = ((mMonth - 1) % 12) + 1;
				int32_t adjustment_year = (mMonth - 1) / 12;
				if ((normalised_month - 1) < 0)
				{
					normalised_month += 12;
					adjustment_year--;
				}
				mMonth = normalised_month;
				mYear += adjustment_year;
			}

		} while(mDay > CICalendarUtils::DaysInMonth(mMonth, mYear));
		break;
	case eRecurrence_YEARLY:
		mYear += interval;
		break;
	}
	
	// Normalise to standard date-time ranges
	Normalise();
}

cdstring CICalendarDateTime::GetLocaleDate(ELocaleDate locale) const
{
	// Determine whether to use mm/dd or dd/mm format
	bool ddmm = CICalendarLocale::UseDDMMDate();

	struct tm stm;
	std::memset(&stm, 0, sizeof(struct tm));
	stm.tm_year = GetYear() - 1900;
	stm.tm_mon = GetMonth() - 1;
	stm.tm_mday = GetDay();
	stm.tm_wday = GetDayOfWeek();

	char buf[1024];
	switch(locale)
	{
	case eFullDate:
		std::strftime(buf, 1024, "%A, %B %d, %Y", &stm);
		break;
	case eAbbrevDate:
		std::strftime(buf, 1024, "%a, %b %d, %Y", &stm);
		break;
	case eNumericDate:
		std::strftime(buf, 1024, ddmm ? "%d/%m/%Y" : "%m/%d/%Y", &stm);
		break;
	case eFullDateNoYear:
		std::strftime(buf, 1024, "%A, %B %d", &stm);
		break;
	case eAbbrevDateNoYear:
		std::strftime(buf, 1024, "%a, %b %d", &stm);
		break;
	case eNumericDateNoYear:
		std::strftime(buf, 1024, ddmm ? "%d/%m" : "%m/%d", &stm);
		break;
	}
	
	return buf;
}

cdstring CICalendarDateTime::GetTime(bool with_seconds, bool am_pm, bool tzid) const
{
	cdstring buf;
	buf.reserve(32);
	int32_t adjusted_hour = mHours;

	if (am_pm)
	{
		bool am = true;
		if (adjusted_hour >= 12)
		{
			adjusted_hour -= 12;
			am = false;
		}
		if (adjusted_hour == 0)
			adjusted_hour = 12;
		
		if (with_seconds)
			::snprintf(buf, 32, "%ld:%02ld:%02ld", adjusted_hour, mMinutes, mSeconds);
		else
			::snprintf(buf, 32, "%ld:%02ld", adjusted_hour, mMinutes);
		buf += (am ? " AM" : " PM");
	}
	else
	{
		if (with_seconds)
			::snprintf(buf, 32, "%02ld:%02ld:%02ld", mHours, mMinutes, mSeconds);
		else
			::snprintf(buf, 32, "%02ld:%02ld", mHours, mMinutes);
	}
	
	if (tzid)
	{
		cdstring desc = TimeZoneDescriptor();
		if (!desc.empty())
		{
			buf += " ";
			buf += desc;
		}
	}

	return buf;
}

cdstring CICalendarDateTime::GetLocaleDateTime(ELocaleDate locale, bool with_seconds, bool am_pm, bool tzid) const
{
	return GetLocaleDate(locale) + " " + GetTime(with_seconds, am_pm, tzid);
}

cdstring CICalendarDateTime::GetText() const
{
	cdstring buf;
	if (mDateOnly)
	{
		buf.reserve(10);
		::snprintf(buf, 10, "%04d%02d%02d", mYear, mMonth, mDay);
	}
	else
	{
		if (mTimezone.GetUTC())
		{
			buf.reserve(18);
			::snprintf(buf, 18, "%04d%02d%02dT%02d%02d%02dZ", mYear, mMonth, mDay, mHours, mMinutes, mSeconds);
		}
		else
		{
			buf.reserve(17);
			::snprintf(buf, 17, "%04d%02d%02dT%02d%02d%02d", mYear, mMonth, mDay, mHours, mMinutes, mSeconds);
		}
	}

	return buf;
}

void CICalendarDateTime::Parse(const cdstring& data)
{
	// parse format YYYYMMDD[THHMMSS[Z]]

	// Size must be at least 8
	if (data.length() < 8)
		return;

	// Get year
	char buf[256];
	::strncpy(buf, data.c_str(), 4);
	buf[4] = 0;
	mYear = strtol(buf, NULL, 10);

	// Get month
	::strncpy(buf, data.c_str() + 4, 2);
	buf[2] = 0;
	mMonth = strtol(buf, NULL, 10);

	// Get day
	::strncpy(buf, data.c_str() + 6, 2);
	buf[2] = 0;
	mDay = strtol(buf, NULL, 10);

	// Now look for more
	if ((data.length() >= 15) && (data[(cdstring::size_type)8] == 'T'))
	{
		// Get hours
		::strncpy(buf, data.c_str() + 9, 2);
		buf[2] = 0;
		mHours = strtol(buf, NULL, 10);

		// Get minutes
		::strncpy(buf, data.c_str() + 11, 2);
		buf[2] = 0;
		mMinutes = strtol(buf, NULL, 10);

		// Get seconds
		::strncpy(buf, data.c_str() + 13, 2);
		buf[2] = 0;
		mSeconds = strtol(buf, NULL, 10);

		mDateOnly = false;

		mTimezone.SetUTC(data[(cdstring::size_type)15] == 'Z');
	}
	else
		mDateOnly = true;

	// Always uncache posix time
	Changed();
}

void CICalendarDateTime::Generate(std::ostream& os) const
{
	os << GetText().c_str();
}

void CICalendarDateTime::GenerateRFC2822(std::ostream& os) const
{
	EDayOfWeek day = GetDayOfWeek();
	os << GetDayOfWeekText(day) << ", " << mDay << " " << GetMonthText() << " " << mYear;
	if (!mDateOnly)
	{
		os << " " << std::setfill('0') << std::setw(2) << mHours << ":" << std::setw(2) << mMinutes << ":" << std::setw(2) << mSeconds << std::setfill(' ');
		
		int32_t tzoffset = TimeZoneSecondsOffset();
		if (tzoffset < 0)
		{
			tzoffset = -tzoffset;
			os << " -";
		}
		else
			os << " +";
		os << std::setfill('0') << std::setw(2) << tzoffset / (60 * 60) << std::setw(2) << (tzoffset / 60) % 60;
	}
}
