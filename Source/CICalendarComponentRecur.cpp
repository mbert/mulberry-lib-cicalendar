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
	CICalendarComponentRecur.cpp

	Author:
	Description:	Base class for iCal components that can have recurrence
*/

#include "CICalendarComponentRecur.h"

#include "CICalendar.h"
#include "CICalendarComponentExpanded.h"
#include "CICalendarDateTimeValue.h"
#include "CICalendarDefinitions.h"
#include "CICalendarDuration.h"
#include "CICalendarRecurrenceSet.h"

#include <algorithm>
#include <iterator>

using namespace iCal;

bool CICalendarComponentRecur::sort_by_dtstart_allday(CICalendarComponentRecur* e1, CICalendarComponentRecur* e2)
{
	if (e1->mStart.IsDateOnly() && e2->mStart.IsDateOnly())
		return e1->mStart < e2->mStart;
	else if (e1->mStart.IsDateOnly())
		return true;
	else if (e2->mStart.IsDateOnly())
		return false;
	else if (e1->mStart == e2->mStart)
	{
		if (e1->mEnd == e2->mEnd)
			// Put ones created earlier in earlier columns in day view
			return e1->mStamp < e2->mStamp;
		else
			// Put ones that end later in earlier columns in day view
			return e1->mEnd > e2->mEnd;
	}
	else
		return e1->mStart < e2->mStart;
}

bool CICalendarComponentRecur::sort_by_dtstart(CICalendarComponentRecur* e1, CICalendarComponentRecur* e2)
{
	if (e1->mStart == e2->mStart)
	{
		if (e1->mStart.IsDateOnly() ^ e2->mStart.IsDateOnly())
			return e1->mStart.IsDateOnly();
		else
			return false;
	}
	else
		return e1->mStart < e2->mStart;
}

CICalendarComponentRecur::CICalendarComponentRecur(const CICalendarRef& calendar) :
	CICalendarComponent(calendar)
{
	mMaster = this;
	mHasStamp = false;
	mHasStart = false;
	mHasEnd = false;
	mDuration = false;
	mHasRecurrenceID = false;
	mAdjustFuture = false;
	mAdjustPrior = false;
	mRecurrences = NULL;
}

CICalendarComponentRecur::CICalendarComponentRecur(const CICalendarComponentRecur& copy) :
	CICalendarComponent(copy)
{
	mRecurrences = NULL;
	_copy_CICalendarComponentRecur(copy);
}

void CICalendarComponentRecur::_copy_CICalendarComponentRecur(const CICalendarComponentRecur& copy)
{
	// Special determination of master
	if (copy.Recurring())
		mMaster = copy.mMaster;
	else
		mMaster = this;

	mMapKey = copy.mMapKey;

	mSummary = copy.mSummary;

	mStamp = copy.mStamp;
	mHasStamp = copy.mHasStamp;

	mStart = copy.mStart;
	mHasStart = copy.mHasStart;
	mEnd = copy.mEnd;
	mHasEnd = copy.mHasEnd;
	mDuration = copy.mDuration;

	mHasRecurrenceID = copy.mHasRecurrenceID;
	mAdjustFuture = copy.mAdjustFuture;
	mAdjustPrior = copy.mAdjustPrior;
	mRecurrenceID = copy.mRecurrenceID;
	
	if (copy.mRecurrences != NULL)
		mRecurrences = new CICalendarRecurrenceSet(*copy.mRecurrences);
}

void CICalendarComponentRecur::_tidy_CICalendarComponentRecur()
{
	delete mRecurrences;
	mRecurrences = NULL;
}

void CICalendarComponentRecur::Finalise()
{
	CICalendarComponent::Finalise();

	// Get DTSTAMP
	mHasStamp = LoadValue(cICalProperty_DTSTAMP, mStamp);

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

	// Get SUMMARY
	LoadValue(cICalProperty_SUMMARY, mSummary);

	// Get RECURRENCE-ID
	mHasRecurrenceID = LoadValue(cICalProperty_RECURRENCE_ID, mRecurrenceID);
	
	// Update the map key
	if (mHasRecurrenceID)
	{
		mMapKey = MapKey(mUID, mRecurrenceID.GetText());
		
		// Also get the RANGE attribute
		const CICalendarAttributeMap& attrs = (*GetProperties().find(cICalProperty_RECURRENCE_ID)).second.GetAttributes();
		CICalendarAttributeMap::const_iterator found = attrs.find(cICalAttribute_RANGE);
		if (found != attrs.end())
		{
			mAdjustFuture = ((*found).second.GetFirstValue() == cICalAttribute_RANGE_THISANDFUTURE);
			mAdjustPrior = ((*found).second.GetFirstValue() == cICalAttribute_RANGE_THISANDPRIOR);
		}
		else
		{
			mAdjustFuture = false;
			mAdjustPrior = false;
		}
	}
	else
		mMapKey = MapKey(mUID);
	
	// May need to create items
	if ((GetProperties().count(cICalProperty_RRULE) != 0) ||
		(GetProperties().count(cICalProperty_RDATE) != 0) ||
		(GetProperties().count(cICalProperty_EXRULE) != 0) ||
		(GetProperties().count(cICalProperty_EXDATE) != 0))
	{
		if (mRecurrences == NULL)
			mRecurrences = new CICalendarRecurrenceSet;

		// Get RRULEs
		LoadValueRRULE(cICalProperty_RRULE, *mRecurrences, true);
		
		// Get RDATEs
		LoadValueRDATE(cICalProperty_RDATE, *mRecurrences, true);
		
		// Get EXRULEs
		LoadValueRRULE(cICalProperty_EXRULE, *mRecurrences, false);
		
		// Get EXDATEs
		LoadValueRDATE(cICalProperty_EXDATE, *mRecurrences, false);
	}
}

void CICalendarComponentRecur::FixStartEnd()
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

cdstring CICalendarComponentRecur::MapKey(const cdstring& uid)
{
	return cdstring("u:") + uid;
}

cdstring CICalendarComponentRecur::MapKey(const cdstring& uid, const cdstring& rid)
{
	return cdstring("r:") + uid + rid;
}


void CICalendarComponentRecur::SetUID(const cdstring& uid)
{
	CICalendarComponent::SetUID(uid);

	// Update the map key
	if (mHasRecurrenceID)
	{
		mMapKey = MapKey(mUID, mRecurrenceID.GetText());
	}
	else
	{
		mMapKey = MapKey(mUID);
	}
}

cdstring CICalendarComponentRecur::GetMasterKey() const
{
	return MapKey(mUID);
}

void CICalendarComponentRecur::InitDTSTAMP()
{
	// Save new one
	CICalendarComponent::InitDTSTAMP();
	
	// Get the new one
	mHasStamp = LoadValue(cICalProperty_DTSTAMP, mStamp);
}

cdstring CICalendarComponentRecur::GetDescription() const
{
	// Get DESCRIPTION
	cdstring txt;
	LoadValue(cICalProperty_DESCRIPTION, txt);
	return txt;
}

cdstring CICalendarComponentRecur::GetLocation() const
{
	// Get LOCATION
	cdstring txt;
	LoadValue(cICalProperty_LOCATION, txt);
	return txt;
}

bool CICalendarComponentRecur::GetTransparent() const
{
	cdstring txt;
	if (LoadValue(cICalProperty_TRANSP, txt))
		return txt == cICalProperty_TRANSPARENT;
	else
		return false;
}

void CICalendarComponentRecur::SetMaster(CICalendarComponentRecur* master)
{
	mMaster = master;
	InitFromMaster();
}

void CICalendarComponentRecur::InitFromMaster()
{
	// Only if not master
	if (Recurring())
	{
		// Redo this to get cached values from master
		Finalise();
	
		// If this component does not have its own start property, use the recurrence id
		// i.e. the start time of this instance has not changed - something else has
		if (GetProperties().count(cICalProperty_DTSTART) == 0)
			mStart = mRecurrenceID;
	
		// If this component does not have its own end/duration property, the determine
		// the end from the master duration
		if ((GetProperties().count(cICalProperty_DTEND) == 0) && (GetProperties().count(cICalProperty_DURATION) == 0))
		{
			// End is based on original events settings
			mEnd = mStart + (mMaster->GetEnd() - mMaster->GetStart());
		}
		
		// If this instance has a duration, but no start of its own, then we need to readjust the end
		// to account for the start being changed to the recurrence id
		else if ((GetProperties().count(cICalProperty_DURATION) != 0) && (GetProperties().count(cICalProperty_DTSTART) == 0))
		{
			CICalendarDuration temp;
			LoadValue(cICalProperty_DURATION, temp);
			mEnd = mStart + temp;
		}
	}
}

void CICalendarComponentRecur::ExpandPeriod(const CICalendarPeriod& period, CICalendarExpandedComponents& list)
{
	// Check for recurrence and true master
	if ((mRecurrences != NULL) && mRecurrences->HasRecurrence() && !IsRecurrenceInstance())
	{
		// Expand recurrences within the range
		CICalendarDateTimeList items;
		mRecurrences->Expand(mStart, period, items);
		
		// Look for overridden recurrence items
		CICalendar* cal = CICalendar::GetICalendar(GetCalendar());
		if (cal != NULL)
		{
			// Remove recurrence instances from the list of items
			CICalendarDateTimeList recurs;
			cal->GetRecurrenceInstances(CICalendarComponent::eVEVENT, GetUID(), recurs);
			std::sort(recurs.begin(), recurs.end());
			if (recurs.size() != 0)
			{
				CICalendarDateTimeList temp;
				std::set_difference(items.begin(), items.end(), recurs.begin(), recurs.end(), back_inserter(temp));
				items = temp;

				// Now get actual instances
				CICalendarComponentRecurs instances;
				cal->GetRecurrenceInstances(CICalendarComponent::eVEVENT, GetUID(), instances);

				// Get list of each ones with RANGE
				CICalendarComponentRecurs prior;
				CICalendarComponentRecurs future;
				for(CICalendarComponentRecurs::const_iterator iter = instances.begin(); iter != instances.end(); iter++)
				{
					if ((*iter)->IsAdjustPrior())
						prior.push_back(*iter);
					if ((*iter)->IsAdjustFuture())
						future.push_back(*iter);
				}
				
				// Check for special behaviour
				if (prior.empty() && future.empty())
				{
					// Add each expanded item
					for(CICalendarDateTimeList::const_iterator iter = items.begin(); iter != items.end(); iter++)
					{
						list.push_back(CreateExpanded(this, *iter));
					}
				}
				else
				{
					// Sort each list first
					std::sort(prior.begin(), prior.end(), sort_by_dtstart);
					std::sort(future.begin(), future.end(), sort_by_dtstart);
					
					// Add each expanded item
					for(CICalendarDateTimeList::const_iterator iter1 = items.begin(); iter1 != items.end(); iter1++)
					{
						// Now step through each using the slave item instead of the master as appropriate
						CICalendarComponentRecur* slave = NULL;

						// Find most appropriate THISANDPRIOR item
						for(CICalendarComponentRecurs::reverse_iterator riter2 = prior.rbegin(); riter2 != prior.rend(); riter2++)
						{
							if ((*riter2)->GetStart() > *iter1)
							{
								slave = *riter2;
								break;
							}
						}
						
						// Find most appropriate THISANDFUTURE item
						for(CICalendarComponentRecurs::reverse_iterator riter2 = future.rbegin(); riter2 != future.rend(); riter2++)
						{
							if ((*riter2)->GetStart() < *iter1)
							{
								slave = *riter2;
								break;
							}
						}
						
						list.push_back(CreateExpanded(slave != NULL ? slave : this, *iter1));
					}
				}
			}
			else
			{
				// Add each expanded item
				for(CICalendarDateTimeList::const_iterator iter = items.begin(); iter != items.end(); iter++)
				{
					list.push_back(CreateExpanded(this, *iter));
				}
			}
		}
	}
	
	else if (WithinPeriod(period))
		list.push_back(CICalendarComponentExpandedShared(new CICalendarComponentExpanded(this, IsRecurrenceInstance() ? &mRecurrenceID : NULL)));
}

bool CICalendarComponentRecur::WithinPeriod(const CICalendarPeriod& period) const
{
	// Check for recurrence
	if ((mRecurrences != NULL) && mRecurrences->HasRecurrence())
	{
		CICalendarDateTimeList items;
		mRecurrences->Expand(mStart, period, items);
		return !items.empty();
	}
	else
	{
		// Does event span the period (assume mEnd > mStart)
		// Check start (inclusive) and end (exclusive)
		if ((mEnd <= period.GetStart()) || (mStart >= period.GetEnd()))
			return false;
		else
			return true;
	}
}

CICalendarComponentExpandedShared CICalendarComponentRecur::CreateExpanded(CICalendarComponentRecur* master, const CICalendarDateTime& recurid)
{
	return CICalendarComponentExpandedShared(new CICalendarComponentExpanded(master, &recurid));
}

bool CICalendarComponentRecur::IsRecurring() const
{
	return (mRecurrences != NULL) && mRecurrences->HasRecurrence();
}

// Clear out all cached recurrence data
void CICalendarComponentRecur::ChangedRecurrence()
{
	// Clear cached values
	if (mRecurrences != NULL)
		mRecurrences->Changed();
}

void CICalendarComponentRecur::EditSummary(const cdstring& summary)
{
	// Updated cached value
	mSummary = summary;

	// Remove existing items
	RemoveProperties(cICalProperty_SUMMARY);
	
	// Now create properties
	if (summary.length())
	{
		CICalendarProperty prop(cICalProperty_SUMMARY, summary);
		AddProperty(prop);
	}
}

void CICalendarComponentRecur::EditDetails(const cdstring& description, const cdstring& location)
{
	// Remove existing items
	RemoveProperties(cICalProperty_DESCRIPTION);
	RemoveProperties(cICalProperty_LOCATION);
	
	// Now create properties
	if (description.length())
	{
		cdstring convert(description);
		convert.ConvertEndl(eEndl_LF);
		CICalendarProperty prop(cICalProperty_DESCRIPTION, convert);
		AddProperty(prop);
	}
	if (location.length())
	{
		CICalendarProperty prop(cICalProperty_LOCATION, location);
		AddProperty(prop);
	}
}

void CICalendarComponentRecur::EditTransparent(bool transparent)
{
	// Remove existing items
	RemoveProperties(cICalProperty_TRANSP);

	// Only need to add if TRANSPARENT, as OPAQUE is default if property is not present.
	if (transparent)
	{
		CICalendarProperty prop(cICalProperty_TRANSP, cICalProperty_TRANSPARENT);
		AddProperty(prop);
	}
}

void CICalendarComponentRecur::EditTiming()
{
	// Updated cached values
	mHasStart = mHasEnd = mDuration = false;
	mStart.SetToday();
	mEnd.SetToday();

	// Remove existing DTSTART & DTEND & DURATION & DUE items
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);
	RemoveProperties(cICalProperty_DUE);
}

void CICalendarComponentRecur::EditTiming(const CICalendarDateTime& due)
{
	// Updated cached values
	mHasStart = false;
	mHasEnd = true;
	mDuration = false;
	mStart = due;
	mEnd = due;

	// Remove existing DUE & DTSTART & DTEND & DURATION items
	RemoveProperties(cICalProperty_DUE);
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);

	// Now create properties
	{
		CICalendarProperty prop(cICalProperty_DUE, due);
		AddProperty(prop);
	}
}

void CICalendarComponentRecur::EditTiming(const CICalendarDateTime& start, const CICalendarDateTime& end)
{
	// Updated cached values
	mHasStart = mHasEnd = true;
	mStart = start;
	mEnd = end;
	mDuration = false;
	FixStartEnd();

	// Remove existing DTSTART & DTEND & DURATION & DUE items
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);
	RemoveProperties(cICalProperty_DUE);
	
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

void CICalendarComponentRecur::EditTiming(const CICalendarDateTime& start, const CICalendarDuration& duration)
{
	// Updated cached values
	mHasStart = true;
	mHasEnd = false;
	mStart = start;
	mEnd = start + duration;
	mDuration = true;

	// Remove existing DTSTART & DTEND & DURATION & DUE items
	RemoveProperties(cICalProperty_DTSTART);
	RemoveProperties(cICalProperty_DTEND);
	RemoveProperties(cICalProperty_DURATION);
	RemoveProperties(cICalProperty_DUE);
	
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

void CICalendarComponentRecur::EditRecurrenceSet(const CICalendarRecurrenceSet& recurs)
{
	// Must have items
	if (mRecurrences == NULL)
		mRecurrences = new CICalendarRecurrenceSet;

	// Updated cached values
	*mRecurrences = recurs;
	
	// Remove existing RRULE, EXRULE, RDATE & EXDATE
	RemoveProperties(cICalProperty_RRULE);
	RemoveProperties(cICalProperty_EXRULE);
	RemoveProperties(cICalProperty_RDATE);
	RemoveProperties(cICalProperty_EXDATE);
	
	// Now create properties
	for(CICalendarRecurrenceList::const_iterator iter = mRecurrences->GetRules().begin(); iter != mRecurrences->GetRules().end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_RRULE, *iter);
		AddProperty(prop);
	}
	for(CICalendarRecurrenceList::const_iterator iter = mRecurrences->GetExrules().begin(); iter != mRecurrences->GetExrules().end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_EXRULE, *iter);
		AddProperty(prop);
	}
	for(CICalendarDateTimeList::const_iterator iter = mRecurrences->GetDates().begin(); iter != mRecurrences->GetDates().end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_RDATE, *iter);
		AddProperty(prop);
	}
	for(CICalendarDateTimeList::const_iterator iter = mRecurrences->GetExdates().begin(); iter != mRecurrences->GetExdates().end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_EXDATE, *iter);
		AddProperty(prop);
	}
}

void CICalendarComponentRecur::ExcludeRecurrence(const CICalendarDateTime& start)
{
	// Must have items
	if (mRecurrences == NULL)
		return;
	
	// Add to recurrence set and clear cache
	mRecurrences->Subtract(start);
	
	// Add property
	CICalendarProperty prop(cICalProperty_EXDATE, start);
	AddProperty(prop);
}

void CICalendarComponentRecur::ExcludeFutureRecurrence(const CICalendarDateTime& start)
{
	// Must have items
	if (mRecurrences == NULL)
		return;
	
	// Adjust RRULES to end before start
	mRecurrences->ExcludeFutureRecurrence(start);
	
	// Remove existing RRULE & RDATE
	RemoveProperties(cICalProperty_RRULE);
	RemoveProperties(cICalProperty_RDATE);
	
	// Now create properties
	for(CICalendarRecurrenceList::const_iterator iter = mRecurrences->GetRules().begin(); iter != mRecurrences->GetRules().end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_RRULE, *iter);
		AddProperty(prop);
	}
	for(CICalendarDateTimeList::const_iterator iter = mRecurrences->GetDates().begin(); iter != mRecurrences->GetDates().end(); iter++)
	{
		CICalendarProperty prop(cICalProperty_RDATE, *iter);
		AddProperty(prop);
	}
}

// Integers can be read from varios types of value
bool CICalendarComponentRecur::LoadValue(const char* value_name, int32_t& value, CICalendarValue::EICalValueType type) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValue(value_name, value, type);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValue(value_name, value, type);

	return result;
}

bool CICalendarComponentRecur::LoadValue(const char* value_name, cdstring& value) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValue(value_name, value);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValue(value_name, value);

	return result;
}

bool CICalendarComponentRecur::LoadValue(const char* value_name, CICalendarDateTime& value) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValue(value_name, value);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValue(value_name, value);

	return result;
}

bool CICalendarComponentRecur::LoadValue(const char* value_name, CICalendarDuration& value) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValue(value_name, value);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValue(value_name, value);

	return result;
}

bool CICalendarComponentRecur::LoadValue(const char* value_name, CICalendarPeriod& value) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValue(value_name, value);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValue(value_name, value);

	return result;
}

bool CICalendarComponentRecur::LoadValueRRULE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValueRRULE(value_name, value, add);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValueRRULE(value_name, value, add);

	return result;
}

bool CICalendarComponentRecur::LoadValueRDATE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const
{
	// Try to load from this component
	bool result = CICalendarComponent::LoadValueRDATE(value_name, value, add);
	
	// Try to load from master if we didn't get it from this component
	if (!result && (mMaster != NULL) && (mMaster != this))
		result = mMaster->LoadValueRDATE(value_name, value, add);

	return result;
}
