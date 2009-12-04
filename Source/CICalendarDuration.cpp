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
	CICalendarDuration.cpp

	Author:
	Description:	<describe the CICalendarDuration class here>
*/

#include "CICalendarDuration.h"

#include <cerrno>
#include <cstdlib>

using namespace iCal;

CICalendarDuration::CICalendarDuration()
{
	mForward = true;

	mWeeks = 0;
	mDays = 0;

	mHours = 0;
	mMinutes = 0;
	mSeconds = 0;
}

void CICalendarDuration::_copy_CICalendarDuration(const CICalendarDuration& copy)
{
	mForward = copy.mForward;

	mWeeks = copy.mWeeks;
	mDays = copy.mDays;

	mHours = copy.mHours;
	mMinutes = copy.mMinutes;
	mSeconds = copy.mSeconds;
}

int64_t CICalendarDuration::GetTotalSeconds() const
{
	return (mForward ? 1LL : -1LL) * (mSeconds + (mMinutes + (mHours + (mDays + (mWeeks * 7LL)) * 24LL) * 60LL) * 60LL);
}

void CICalendarDuration::SetDuration(const int64_t seconds)
{
	mForward = seconds >= 0;

	int64_t remainder = seconds;
	if (remainder < 0)
		remainder = -remainder;

	// Is it an exact number of weeks - if so use the weeks value, otherwise days, hours, minutes, seconds
	if (remainder % (7 * 24 * 60 * 60) == 0)
	{
		mWeeks = remainder / (7 * 24 * 60 * 60);
		mDays = 0;

		mHours = 0;
		mMinutes = 0;
		mSeconds = 0;
	}
	else
	{
		mSeconds = remainder % 60;
		remainder -= mSeconds;
		remainder /= 60;

		mMinutes = remainder % 60;
		remainder -= mMinutes;
		remainder /= 60;

		mHours = remainder % 24;
		remainder -= mHours;

		mDays = remainder / 24;

		mWeeks = 0;
	}
}

void CICalendarDuration::Parse(const cdstring& data)
{
	// parse format ([+]/-) "P" (dur-date / dur-time / dur-week)

	const char* p = data.c_str();

	// Look for +/-
	mForward = true;
	if (*p == '-')
	{
		mForward = false;
		p++;
	}
	else if (*p == '+')
	{
		p++;
	}

	// Must have a 'P'
	if (*p++ != 'P')
		return;

	// Look for time
	if (*p != 'T')
	{
		// Must have a number
		uint32_t num = strtoul(p, const_cast<char**>(&p), 10);
		if ((errno == EINVAL) || (errno == ERANGE))
			return;

		// Now look at character
		if (*p == 'W')
		{
			// Have a number of weeks
			mWeeks = num;

			// There cannot bew anything else after this so just exit
			return;
		}
		else if (*p == 'D')
		{
			// Have a number of days
			mDays = num;

			p++;

			// Look for time - exit if none
			if (*p != 'T')
				return;
		}
		else
			// Error in format
			return;
	}

	// Have time
	p++;
	uint32_t num = strtoul(p, const_cast<char**>(&p), 10);
	if ((errno == EINVAL) || (errno == ERANGE))
		return;

	// Look for hour
	if (*p == 'H')
	{
		// Get hours
		mHours = num;

		// Look for more data - exit if none
		p++;
		if (!*p)
			return;

		// Parse the next number
		num = strtoul(p, const_cast<char**>(&p), 10);
		if ((errno == EINVAL) || (errno == ERANGE))
			return;
	}

	// Look for minute
	if (*p == 'M')
	{
		// Get hours
		mMinutes = num;

		// Look for more data - exit if none
		p++;
		if (!*p)
			return;

		// Parse the next number
		num = strtoul(p, const_cast<char**>(&p), 10);
		if ((errno == EINVAL) || (errno == ERANGE))
			return;
	}

	// Look for seconds
	if (*p == 'S')
	{
		// Get hours
		mSeconds = num;
	}
}

void CICalendarDuration::Generate(std::ostream& os) const
{
	if (!mForward && ((mWeeks != 0) || (mDays != 0) || (mHours != 0) || (mMinutes != 0) || (mSeconds != 0)))
	{
		os << '-';
	}
	os << 'P';

	if (mWeeks != 0)
	{
		os << mWeeks << 'W';
	}
	else
	{
		if (mDays)
		{
			os << mDays << 'D';
		}

		if ((mHours != 0) || (mMinutes != 0) || (mSeconds != 0))
		{
			os << 'T';
			if (mHours != 0)
			{
				os << mHours << 'H';
			}

			if ((mMinutes != 0) || ((mHours != 0) && (mSeconds != 0)))
			{
				os << mMinutes << 'M';
			}

			if (mSeconds != 0)
			{
				os << mSeconds << 'S';
			}
		}
		else if (mDays == 0)
		{
			os << "T0S";
		}
	}
}
