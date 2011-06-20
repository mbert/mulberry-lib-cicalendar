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
	CICalendarVFreeBusy.cpp

	Author:
	Description:	<describe the CICalendarVFreeBusy class here>
*/

#include "CICalendarVFreeBusy.h"

#include "CICalendarDefinitions.h"
#include "CICalendarDuration.h"
#include "CICalendarMultiValue.h"
#include "CICalendarPeriodValue.h"

#include <algorithm>
#include <iterator>

using namespace iCal;

cdstring CICalendarVFreeBusy::sBeginDelimiter(cICalComponent_BEGINVFREEBUSY);
cdstring CICalendarVFreeBusy::sEndDelimiter(cICalComponent_ENDVFREEBUSY);

void CICalendarVFreeBusy::_init_CICalendarVFreeBusy()
{
	mHasStart = false;
	mHasEnd = false;
	mDuration = false;
	mCachedBusyTime = false;
	mBusyTime = NULL;
}

void CICalendarVFreeBusy::_copy_CICalendarVFreeBusy(const CICalendarVFreeBusy& copy)
{
	mStart = copy.mStart;
	mHasStart = copy.mHasStart;
	mEnd = copy.mEnd;
	mHasEnd = copy.mHasEnd;
	mDuration = copy.mDuration;
	
	mCachedBusyTime = false;
	mBusyTime = NULL;
}

void CICalendarVFreeBusy::_tidy_CICalendarVFreeBusy()
{
	delete mBusyTime;
	mBusyTime = NULL;
}

void CICalendarVFreeBusy::Finalise()
{
	// Do inherited
	CICalendarComponent::Finalise();

	// Get DTSTART
	mHasStart = LoadValue(cICalProperty_DTSTART, mStart);

	// Get DTEND
	if (!LoadValue(cICalProperty_DTEND, mEnd))
	{
		// Try DURATION instead
		CICalendarDuration temp;
		if (LoadValue(cICalProperty_DURATION, temp))
		{
			mEnd = mStart + temp;
			mDuration = true;
		}
		else
		{
			// Force end to start, which will then be fixed to sensible value later
			mEnd = mStart;
		}
	}
	else
	{
		mHasEnd = true;
		mDuration = false;
	}
	
	// Make sure start/end values are sensible
	FixStartEnd();
}

void CICalendarVFreeBusy::FixStartEnd()
{
	// End is always greater than start if start exists
	if (mHasStart && (mEnd <= mStart))
	{
		// Use the start
		mEnd = mStart;
		mDuration = false;
		
		// Adjust to approriate non-inclusive end point
		if (mStart.IsDateOnly())
		{
			mEnd.OffsetDay(1);
			
			// For all day events it makes sense to use duration
			mDuration = true;
		}
		else
		{
			// USe end of current day
			mEnd.OffsetDay(1);
			mEnd.SetHHMMSS(0, 0, 0);
		}
	}
}

void CICalendarVFreeBusy::EditTiming()
{
	// Updated cached values
	mHasStart = mHasEnd = mDuration = false;
	mStart.SetToday();
	mEnd.SetToday();

	// Remove existing DTSTART & DTEND & DURATION items
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);
}

void CICalendarVFreeBusy::EditTiming(const CICalendarDateTime& start, const CICalendarDateTime& end)
{
	// Updated cached values
	mHasStart = mHasEnd = true;
	mStart = start;
	mEnd = end;
	mDuration = false;
	FixStartEnd();

	// Remove existing DTSTART & DTEND & DURATION items
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);
	
	// Now create properties
	{
		CICalendarProperty prop(cICalProperty_DTSTART, start);
		AddProperty(prop);
	}
	
	// If its an all day event and the end one day after the start, ignore it
	CICalendarDateTime temp(start);
	temp.OffsetDay(1);
	if (!start.IsDateOnly() || (end != temp))
	{
		CICalendarProperty prop(cICalProperty_DTEND, end);
		AddProperty(prop);
	}
}

void CICalendarVFreeBusy::EditTiming(const CICalendarDateTime& start, const CICalendarDuration& duration)
{
	// Updated cached values
	mHasStart = true;
	mHasEnd = false;
	mStart = start;
	mEnd = start + duration;
	mDuration = true;

	// Remove existing DTSTART & DTEND & DURATION items
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);
	
	// Now create properties
	{
		CICalendarProperty prop(cICalProperty_DTSTART, start);
		AddProperty(prop);
	}

	// If its an all day event and the duration is one day, ignore it
	if (!start.IsDateOnly() || (duration.GetWeeks() != 0) || (duration.GetDays() > 1))
	{
		CICalendarProperty prop(cICalProperty_DURATION, duration);
		AddProperty(prop);
	}
}

void CICalendarVFreeBusy::ExpandPeriod(const CICalendarPeriod& period, CICalendarComponentList& list)
{
	// Cache the busy-time details if not done already
	if (!mCachedBusyTime)
		CacheBusyTime();
	
	// See if period intersects the busy time span range
	if ((mBusyTime != NULL) && period.IsPeriodOverlap(mSpanPeriod))
	{
		list.push_back(this);
	}
}

void CICalendarVFreeBusy::ExpandPeriod(const CICalendarPeriod& period, CICalendarFreeBusyList& list)
{
	// Cache the busy-time details if not done already
	if (!mCachedBusyTime)
		CacheBusyTime();
	
	// See if period intersects the busy time span range
	if ((mBusyTime != NULL) && period.IsPeriodOverlap(mSpanPeriod))
	{
		std::copy(mBusyTime->begin(), mBusyTime->end(), back_inserter(list));
	}
}

void CICalendarVFreeBusy::GetPeriod(CICalendarFreeBusyList& list)
{
	// Cache the busy-time details if not done already
	if (!mCachedBusyTime)
		CacheBusyTime();
	
	// See if period intersects the busy time span range
	if (mBusyTime != NULL)
	{
		std::copy(mBusyTime->begin(), mBusyTime->end(), back_inserter(list));
	}
}

void CICalendarVFreeBusy::CacheBusyTime()
{
	// Clear out any existing cache
	if (mBusyTime != NULL)
		delete mBusyTime;
	mBusyTime = new CICalendarFreeBusyList();

	// Get all FREEBUSY items and add those that are BUSY
	CICalendarDateTime	min_start;
	CICalendarDateTime	max_end;
	const iCal::CICalendarPropertyMap& props = GetProperties();
	std::pair<iCal::CICalendarPropertyMap::const_iterator, iCal::CICalendarPropertyMap::const_iterator> result = props.equal_range(iCal::cICalProperty_FREEBUSY);
	for(iCal::CICalendarPropertyMap::const_iterator iter = result.first; iter != result.second; iter++)
	{
		// Check the properties FBTYPE attribute
		CICalendarFreeBusy::EBusyType type;
		bool is_busy = false;
		if ((*iter).second.HasAttribute(iCal::cICalAttribute_FBTYPE))
		{
			const cdstring& fbyype = (*iter).second.GetAttributeValue(iCal::cICalAttribute_FBTYPE);
			if ((fbyype.compare(iCal::cICalAttribute_FBTYPE_BUSY, true) == 0))
			{
				is_busy = true;
				type = CICalendarFreeBusy::eBusy;
			}
			else if (fbyype.compare(iCal::cICalAttribute_FBTYPE_BUSYUNAVAILABLE, true) == 0)
			{
				is_busy = true;
				type = CICalendarFreeBusy::eBusyUnavailable;
			}
			else if (fbyype.compare(iCal::cICalAttribute_FBTYPE_BUSYTENTATIVE, true) == 0)
			{
				is_busy = true;
				type = CICalendarFreeBusy::eBusyTentative;
			}
			else
			{
				is_busy = false;
				type = CICalendarFreeBusy::eFree;
			}
		}
		else
		{
			// Default is busy when no attribute
			is_busy = true;
			type = CICalendarFreeBusy::eBusy;
		}

		// Add this period
		if (is_busy)
		{
			const CICalendarMultiValue* multi = (*iter).second.GetMultiValue();
			if ((multi != NULL) && (multi->GetType() == CICalendarValue::eValueType_Period))
			{
				for(CICalendarValueList::const_iterator iter2 = multi->GetValues().begin(); iter2 != multi->GetValues().end(); iter2++)
				{
					// Double-check type
					CICalendarPeriodValue* period = dynamic_cast<CICalendarPeriodValue*>(*iter2);
					if (period != NULL)
					{
						mBusyTime->push_back(CICalendarFreeBusy(type, period->GetValue()));
						
						if (mBusyTime->size() == 1)
						{
							min_start = period->GetValue().GetStart();
							max_end = period->GetValue().GetEnd();
						}
						else
						{
							if (min_start > period->GetValue().GetStart())
								min_start = period->GetValue().GetStart();
							if (max_end < period->GetValue().GetEnd())
								max_end = period->GetValue().GetEnd();
						}
					}
				}
			}
		}
	}

	// If nothing present, empty the list
	if (mBusyTime->size() == 0)
	{
		delete mBusyTime;
		mBusyTime = NULL;
	}
	else
	{
	
		// Sort the list by period
		std::sort(mBusyTime->begin(), mBusyTime->end());

		// Determine range
		CICalendarDateTime	start;
		CICalendarDateTime	end;
		if (mHasStart)
			start = mStart;
		else
			start = min_start;
		if (mHasEnd)
			end = mEnd;
		else
			end = max_end;
		
		mSpanPeriod = CICalendarPeriod(start, end);
	}
	
	mCachedBusyTime = true;
}
