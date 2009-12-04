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
	CICalendarVTimezoneElement.cpp

	Author:
	Description:	<describe the CICalendarVTimezoneElement class here>
*/

#include "CICalendarVTimezoneElement.h"

#include "CICalendarDefinitions.h"

#include <algorithm>

using namespace iCal;

bool CICalendarVTimezoneElement::sort_dtstart(const CICalendarComponent* s1, const CICalendarComponent* s2)
{
	const CICalendarVTimezoneElement* e1 = static_cast<const CICalendarVTimezoneElement*>(s1);
	const CICalendarVTimezoneElement* e2 = static_cast<const CICalendarVTimezoneElement*>(s2);
	return (e1->mStart < e2->mStart);
}

void CICalendarVTimezoneElement::Finalise()
{
	// Get DTSTART
	LoadValue(cICalProperty_DTSTART, mStart);
	mCachedExpandBelow = mStart;

	// Get TZOFFSETTO
	LoadValue(cICalProperty_TZOFFSETTO, mUTCOffset, CICalendarValue::eValueType_UTC_Offset);

	// Get TZNAME
	LoadValue(cICalProperty_TZNAME, mTZName);

	// Get RRULEs
	LoadValueRRULE(cICalProperty_RRULE, mRecurrences, true);
	
	// Get RDATEs
	LoadValueRDATE(cICalProperty_RDATE, mRecurrences, true);
	
	// Do inherited
	CICalendarComponent::Finalise();
}

// Find the newest expanded date-time for this element that is older than the requested one
CICalendarDateTime CICalendarVTimezoneElement::ExpandBelow(const CICalendarDateTime& below) const
{
	// Look for recurrences
	if (!mRecurrences.HasRecurrence() || (mStart > below))
		// Return DTSTART even if it is newer
		return mStart;
	else
	{
		// We want to allow recurrence calculation caching to help us here as this method
		// gets called a lot - most likely for ever increasing dt values (which will therefore
		// invalidate the recurrence cache).
		//
		// What we will do is round up the date-time to the next year so that the recurrence
		// cache is invalidated less frequently
		
		CICalendarDateTime temp(below);
		temp.SetMonth(1);
		temp.SetDay(1);
		temp.SetHHMMSS(0, 0, 0);
		temp.OffsetYear(1);

		// Use cache of expansion		
		if (temp > mCachedExpandBelow)
		{
			mCachedExpandBelowItems.clear();
			CICalendarPeriod period(mStart, temp);
			mRecurrences.Expand(mStart, period, mCachedExpandBelowItems);
			mCachedExpandBelow = temp;
		}
		
		if (mCachedExpandBelowItems.size() != 0)
		{
			// List comes back sorted so we pick the element just less than the dt value we want
			CICalendarDateTimeList::const_iterator found = std::lower_bound(mCachedExpandBelowItems.begin(), mCachedExpandBelowItems.end(), below);
			if (found != mCachedExpandBelowItems.begin())
				return *(found - 1);
		}

		return mStart;
	}
}

