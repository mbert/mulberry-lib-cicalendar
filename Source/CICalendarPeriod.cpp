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
	CICalendarPeriod.cpp

	Author:
	Description:	<describe the CICalendarPeriod class here>
*/

#include "CICalendarPeriod.h"

#include "CXStringResources.h"

using namespace iCal;

void CICalendarPeriod::Parse(const cdstring& data)
{
	cdstring::size_type slash_pos = data.find('/');
	if (slash_pos != cdstring::npos)
	{
		cdstring start(data, 0, slash_pos);
		cdstring end(data, slash_pos + 1);

		mStart.Parse(start);
		if (end[0UL] == 'P')
		{
			mDuration.Parse(end);
			mUseDuration = true;
			mEnd = mStart + mDuration;
		}
		else
		{
			mEnd.Parse(end);
			mUseDuration = false;
			mDuration = mEnd - mStart;
		}
	}
}

void CICalendarPeriod::Generate(std::ostream& os) const
{
	mStart.Generate(os);
	os << '/';
	if (mUseDuration)
		mDuration.Generate(os);
	else
		mEnd.Generate(os);
}

cdstring CICalendarPeriod::DescribeDuration() const
{
	cdstring result;

	// Break period down into ww::dd::hh::mm::ss
	uint32_t weeks = mDuration.GetWeeks();
	uint32_t days = mDuration.GetDays();
	uint32_t hours = mDuration.GetHours();
	uint32_t mins = mDuration.GetMinutes();
	uint32_t secs = mDuration.GetSeconds();
	
	if (weeks != 0)
	{
		result += cdstring(weeks);
		result += " ";
		result += rsrc::GetString("Duration::Weeks::Long");
	}
	else
	{
		if (days != 0)
		{
			result += cdstring(days);
			result += " ";
			result += rsrc::GetString(days == 1 ? "Duration::Day::Long" : "Duration::Days::Long");
		}
		
		if ((hours != 0) || (mins != 0) || (secs != 0))
		{
			if (!result.empty())
				result += ", ";
			result += cdstring(hours);
			result += " ";
			result += rsrc::GetString(hours == 1 ? "Duration::Hour::Long" : "Duration::Hours::Long");
		}
		
		if ((mins != 0) || (secs != 0))
		{
			if (!result.empty())
				result += ", ";
			result += cdstring(mins);
			result += " ";
			result += rsrc::GetString(mins == 1 ? "Duration::Minute::Long" : "Duration::Minutes::Long");
		}
		
		if (secs != 0)
		{
			if (!result.empty())
				result += ", ";
			result += cdstring(secs);
			result += " ";
			result += rsrc::GetString(secs == 1 ? "Duration::Second::Long" : "Duration::Seconds::Long");
		}
	}
	
	return result;
}
