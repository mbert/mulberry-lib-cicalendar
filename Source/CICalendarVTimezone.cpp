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
	CICalendarVTimezone.cpp

	Author:
	Description:	<describe the CICalendarVTimezone class here>
*/

#include "CICalendarVTimezone.h"

#include "CICalendarDefinitions.h"
#include "CICalendarTextValue.h"
#include "CICalendarVTimezoneElement.h"
#include "CICalendarVTimezoneStandard.h"
#include "CICalendarVTimezoneDaylight.h"

#include <algorithm>
#include <cstdio>

using namespace iCal;

cdstring CICalendarVTimezone::sBeginDelimiter(cICalComponent_BEGINVTIMEZONE);
cdstring CICalendarVTimezone::sEndDelimiter(cICalComponent_ENDVTIMEZONE);

CICalendarVTimezone::~CICalendarVTimezone()
{
}

void CICalendarVTimezone::_copy_CICalendarVTimezone(const CICalendarVTimezone& copy)
{
	mID = copy.mID;
	mSortKey = copy.mSortKey;
}

bool CICalendarVTimezone::AddComponent(CICalendarComponent* comp)
{
	// We can embed the timezone components only
	if ((comp->GetType() == eVTIMEZONESTANDARD) || (comp->GetType() == eVTIMEZONEDAYLIGHT))
	{
		if (mEmbedded == NULL)
			mEmbedded = new CICalendarComponentList;
		mEmbedded->push_back(comp);
		mEmbedded->back()->SetEmbedder(this);
		return true;
	}
	else
		return false;
}

void CICalendarVTimezone::Finalise()
{
	// Get TZID
	LoadValue(cICalProperty_TZID, mID);

	// Sort sub-components by DTSTART
	if (mEmbedded != NULL)
		sort(mEmbedded->begin(), mEmbedded->end(), CICalendarVTimezoneElement::sort_dtstart);

	// Do inherited
	CICalendarComponent::Finalise();
}

int32_t CICalendarVTimezone::GetSortKey() const
{
	if (mSortKey == 1)
	{
		// Take time from first element
		if ((mEmbedded != NULL) && (mEmbedded->size() > 0))
		{
			// Initial offset provides the primary key
			int32_t utc_offset1 = static_cast<const CICalendarVTimezoneElement*>(mEmbedded->at(0))->GetUTCOffset();
			
			// Presence of secondary is the next key
			int32_t utc_offset2 = utc_offset1;
			if (mEmbedded->size() > 1)
			{
				utc_offset2 = static_cast<const CICalendarVTimezoneElement*>(mEmbedded->at(1))->GetUTCOffset();
			}

			// Create key
			mSortKey = (utc_offset1 + utc_offset2) / 2;
		}
		else
			mSortKey = 0;
	}
	
	return mSortKey;
}

int32_t CICalendarVTimezone::GetTimezoneOffsetSeconds(const CICalendarDateTime& dt)
{
	// Get the closet matching element to the time
	const CICalendarVTimezoneElement* found = FindTimezoneElement(dt);

	// Return it
	if (found == NULL)
		return 0;
	else
	{
		// Get its offset
		return found->GetUTCOffset();
	}
}

cdstring CICalendarVTimezone::GetTimezoneDescriptor(const CICalendarDateTime& dt)
{
	cdstring result;

	// Get the closet matching element to the time
	const CICalendarVTimezoneElement* found = FindTimezoneElement(dt);

	// Get it
	if (found != NULL)
	{
		if (found->GetTZName().empty())
		{
			result.reserve(32);
			int32_t tzoffset = found->GetUTCOffset();
			bool negative = false;
			if (tzoffset < 0)
			{
				tzoffset = -tzoffset;
				negative = true;
			}
			::snprintf(result, 32, "%s%02ld%02ld", negative ? "-" : "+", tzoffset / (60 * 60), (tzoffset / 60) % 60);
		}
		else
		{
			result = "(";
			result += found->GetTZName();
			result += ")";
		}
	}
	
	return result;
}

const CICalendarVTimezoneElement* CICalendarVTimezone::FindTimezoneElement(const CICalendarDateTime& dt)
{
	// Need to make the incoming date-time relative to the DTSTART in the timezone component for proper comparison.
	// This means making the incoming date-time a floating (no timezone) item
	CICalendarDateTime temp(dt);
	temp.GetTimezone().SetTimezoneID("");

	// New scheme to avoid unneccessary timezone recurrence expansion:

	// Look for the standard and daylight components with a start time just below the requested date-time
	const CICalendarVTimezoneElement* found_std = NULL;
	const CICalendarVTimezoneElement* found_day = NULL;

	if (mEmbedded != NULL)
	{
		for(CICalendarComponentList::const_iterator iter = mEmbedded->begin(); iter != mEmbedded->end(); iter++)
		{
			const CICalendarVTimezoneElement* item = static_cast<const CICalendarVTimezoneElement*>(*iter);
			if (item->GetStart() < temp)
			{
				if (item->GetType() == eVTIMEZONESTANDARD)
					found_std = item;
				else
					found_day = item;
			}
		}		
	}
	
	// Now do the expansion for each one found and pick the lowest
	const CICalendarVTimezoneElement* found = NULL;
	CICalendarDateTime dt_found;

	if (found_std)
	{
		CICalendarDateTime dt_item = found_std->ExpandBelow(temp);
		if (temp >= dt_item)
		{
			found = found_std;
			dt_found = dt_item;
		}
	}
	
	if (found_day)
	{
		CICalendarDateTime dt_item = found_day->ExpandBelow(temp);
		if (temp >= dt_item)
		{
			if (found != NULL)
			{
				// Compare with the one previously cached and switch to this one if newer
				if (dt_item > dt_found)
				{
					found = found_day;
					dt_found = dt_item;
				}
			}
			else
			{
				found = found_day;
				dt_found = dt_item;
			}
		}
	}
	
	return found;
}

void CICalendarVTimezone::MergeTimezone(const CICalendarVTimezone& tz)
{
}

bool CICalendarVTimezone::ValidEDST() const
{
	// Look for -4 -> -8 offsets (standard) occurring first Sunday in April in 2006,
	// and verify they have shifted for 2007+
	{
		CICalendarDateTime std_1_2007(2007, 3, 11, 0, 0, 0);
		CICalendarDateTime day_1_2007(2007, 3, 11, 12, 0, 0);
		
		int32_t offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_1_2007);
		int32_t offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_1_2007);
		if ((offset_std == offset_day) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
		{
			CICalendarDateTime std_2_2007(2007, 4, 1, 0, 0, 0);
			CICalendarDateTime day_2_2007(2007, 4, 1, 12, 0, 0);
			offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_2_2007);
			offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_2_2007);
			if ((offset_std - offset_day == -1*60*60) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
			{
				return false;
			}
		}
	}
	
	{
		CICalendarDateTime std_1_2007(2007, 11, 4, 0, 0, 0);
		CICalendarDateTime day_1_2007(2007, 11, 4, 12, 0, 0);
		
		int32_t offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_1_2007);
		int32_t offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_1_2007);
		if ((offset_std == offset_day) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
		{
			CICalendarDateTime std_2_2007(2007, 10, 28, 0, 0, 0);
			CICalendarDateTime day_2_2007(2007, 10, 28, 12, 0, 0);
			offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_2_2007);
			offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_2_2007);
			if ((offset_std - offset_day == -1*60*60) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
			{
				return false;
			}
		}
	}
	
	return true;
}

void CICalendarVTimezone::UpgradeEDST()
{
	// Look for -4 -> -8 offsets (standard) occurring first Sunday in April in 2006,
	// and verify they have shifted for 2007+
	{
		CICalendarDateTime std_1_2007(2007, 3, 11, 0, 0, 0);
		CICalendarDateTime day_1_2007(2007, 3, 11, 12, 0, 0);
		
		int32_t offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_1_2007);
		int32_t offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_1_2007);
		if ((offset_std == offset_day) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
		{
			CICalendarDateTime std_2_2007(2007, 4, 1, 0, 0, 0);
			CICalendarDateTime day_2_2007(2007, 4, 1, 12, 0, 0);
			offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_2_2007);
			offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_2_2007);
			if ((offset_std - offset_day == -1*60*60) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
			{
				AddEDST(offset_std);
				return;
			}
		}
	}
	
	{
		CICalendarDateTime std_1_2007(2007, 11, 4, 0, 0, 0);
		CICalendarDateTime day_1_2007(2007, 11, 4, 12, 0, 0);
		
		int32_t offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_1_2007);
		int32_t offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_1_2007);
		if ((offset_std == offset_day) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
		{
			CICalendarDateTime std_2_2007(2007, 10, 28, 0, 0, 0);
			CICalendarDateTime day_2_2007(2007, 10, 28, 12, 0, 0);
			offset_std = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(std_2_2007);
			offset_day = const_cast<CICalendarVTimezone*>(this)->GetTimezoneOffsetSeconds(day_2_2007);
			if ((offset_std - offset_day == -1*60*60) && (offset_std >= -8*60*60) && (offset_std <= -4*60*60))
			{
				AddEDST(offset_std);
				return;
			}
		}
	}
}

void CICalendarVTimezone::AddEDST(int32_t offset_std)
{
	CICalendarVTimezoneStandard* std = NULL;
	CICalendarVTimezoneDaylight* day = NULL;
	for(CICalendarComponentList::const_iterator iter = mEmbedded->begin(); iter != mEmbedded->end(); iter++)
	{
		if ((*iter)->GetType() == eVTIMEZONESTANDARD)
		{
			std = static_cast<CICalendarVTimezoneStandard*>(*iter);
		}
		else if ((*iter)->GetType() == eVTIMEZONEDAYLIGHT)
		{
			day = static_cast<CICalendarVTimezoneDaylight*>(*iter);
		}
	}
	
	if (day != NULL)
	{
		CICalendarVTimezoneDaylight* newday = static_cast<CICalendarVTimezoneDaylight*>(day->clone());
		newday->RemoveProperties(cICalProperty_DTSTART);
		newday->RemoveProperties(cICalProperty_RRULE);
		newday->RemoveProperties(cICalProperty_EXRULE);
		newday->RemoveProperties(cICalProperty_RDATE);
		newday->RemoveProperties(cICalProperty_EXDATE);
		
		{
			CICalendarTimezone floating(false);
			CICalendarDateTime dtstart(2007, 3, 11, 2, 0, 0, &floating);
			CICalendarProperty prop(cICalProperty_DTSTART, dtstart);
			newday->AddProperty(prop);
		}
		
		{
			CICalendarRecurrence rrule;
			rrule.SetFreq(eRecurrence_YEARLY);
			rrule.SetInterval(1);
			
			std::vector<int32_t> by_month;
			by_month.push_back(3);
			rrule.SetByMonth(by_month);
			
			std::vector<iCal::CICalendarRecurrence::CWeekDayNum> by_day;
			by_day.push_back(iCal::CICalendarRecurrence::CWeekDayNum(2, CICalendarDateTime::eSunday));
			rrule.SetByDay(by_day);
			
			CICalendarProperty prop(cICalProperty_RRULE, rrule);
			newday->AddProperty(prop);
		}
		
		AddComponent(newday);
		
		// Now adjust the old one
		CICalendarRecurrenceSet* rset = day->GetRecurrenceSet();
		for(CICalendarRecurrenceList::const_iterator iter = rset->GetRules().begin(); iter != rset->GetRules().end(); iter++)
		{
			const_cast<CICalendarRecurrence&>(*iter).SetUseUntil(true);
			CICalendarTimezone utc(true);
			CICalendarDateTime until(2006, 4, 2, 7, 0, 0, &utc);
			const_cast<CICalendarRecurrence&>(*iter).SetUntil(until);
		}

		// Remove existing RRULE & RDATE
		day->RemoveProperties(cICalProperty_RRULE);
		day->RemoveProperties(cICalProperty_RDATE);
		
		// Now create properties
		for(CICalendarRecurrenceList::const_iterator iter = rset->GetRules().begin(); iter != rset->GetRules().end(); iter++)
		{
			CICalendarProperty prop(cICalProperty_RRULE, *iter);
			day->AddProperty(prop);
		}
		for(CICalendarDateTimeList::const_iterator iter = rset->GetDates().begin(); iter != rset->GetDates().end(); iter++)
		{
			CICalendarProperty prop(cICalProperty_RDATE, *iter);
			day->AddProperty(prop);
		}
	}
	
	if (std != NULL)
	{
		CICalendarVTimezoneDaylight* newstd = static_cast<CICalendarVTimezoneDaylight*>(std->clone());
		newstd->RemoveProperties(cICalProperty_DTSTART);
		newstd->RemoveProperties(cICalProperty_RRULE);
		newstd->RemoveProperties(cICalProperty_EXRULE);
		newstd->RemoveProperties(cICalProperty_RDATE);
		newstd->RemoveProperties(cICalProperty_EXDATE);
		
		{
			CICalendarTimezone floating(false);
			CICalendarDateTime dtstart(2007, 11, 4, 2, 0, 0, &floating);
			CICalendarProperty prop(cICalProperty_DTSTART, dtstart);
			newstd->AddProperty(prop);
		}
		
		{
			CICalendarRecurrence rrule;
			rrule.SetFreq(eRecurrence_YEARLY);
			rrule.SetInterval(1);
			
			std::vector<int32_t> by_month;
			by_month.push_back(11);
			rrule.SetByMonth(by_month);
			
			std::vector<iCal::CICalendarRecurrence::CWeekDayNum> by_day;
			by_day.push_back(iCal::CICalendarRecurrence::CWeekDayNum(1, CICalendarDateTime::eSunday));
			rrule.SetByDay(by_day);
			
			CICalendarProperty prop(cICalProperty_RRULE, rrule);
			newstd->AddProperty(prop);
		}
		
		AddComponent(newstd);
		
		// Now adjust the old one
		CICalendarRecurrenceSet* rset = std->GetRecurrenceSet();
		for(CICalendarRecurrenceList::const_iterator iter = rset->GetRules().begin(); iter != rset->GetRules().end(); iter++)
		{
			const_cast<CICalendarRecurrence&>(*iter).SetUseUntil(true);
			CICalendarTimezone utc(true);
			CICalendarDateTime until(2006, 10, 29, 6, 0, 0, &utc);
			const_cast<CICalendarRecurrence&>(*iter).SetUntil(until);
		}
		
		// Remove existing RRULE & RDATE
		std->RemoveProperties(cICalProperty_RRULE);
		std->RemoveProperties(cICalProperty_RDATE);
		
		// Now create properties
		for(CICalendarRecurrenceList::const_iterator iter = rset->GetRules().begin(); iter != rset->GetRules().end(); iter++)
		{
			CICalendarProperty prop(cICalProperty_RRULE, *iter);
			std->AddProperty(prop);
		}
		for(CICalendarDateTimeList::const_iterator iter = rset->GetDates().begin(); iter != rset->GetDates().end(); iter++)
		{
			CICalendarProperty prop(cICalProperty_RDATE, *iter);
			std->AddProperty(prop);
		}
	}
}
