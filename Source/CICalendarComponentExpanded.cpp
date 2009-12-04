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
	CICalendarComponentExpanded.cpp

	Author:
	Description:	Base class for iCal recurrence expanded components.
	                This is a template based wrapper that contains a pointer back to the
	                master/slave component that generated this recurrence instance
*/

#include "CICalendarComponentExpanded.h"

#include "CICalendarComponentRecur.h"
#include "CICalendarDuration.h"

using namespace iCal;

bool CICalendarComponentExpanded::sort_by_dtstart_allday(const CICalendarComponentExpandedShared& e1, const CICalendarComponentExpandedShared& e2)
{
	if (e1->mInstanceStart.IsDateOnly() && e2->mInstanceStart.IsDateOnly())
		return e1->mInstanceStart < e2->mInstanceStart;
	else if (e1->mInstanceStart.IsDateOnly())
		return true;
	else if (e2->mInstanceStart.IsDateOnly())
		return false;
	else if (e1->mInstanceStart == e2->mInstanceStart)
	{
		if (e1->mInstanceEnd == e2->mInstanceEnd)
			// Put ones created earlier in earlier columns in day view
			return e1->GetOwner()->GetStamp() < e2->GetOwner()->GetStamp();
		else
			// Put ones that end later in earlier columns in day view
			return e1->mInstanceEnd > e2->mInstanceEnd;
	}
	else
		return e1->mInstanceStart < e2->mInstanceStart;
}

bool CICalendarComponentExpanded::sort_by_dtstart(const CICalendarComponentExpandedShared& e1, const CICalendarComponentExpandedShared& e2)
{
	if (e1->mInstanceStart == e2->mInstanceStart)
	{
		if (e1->mInstanceStart.IsDateOnly() ^ e2->mInstanceStart.IsDateOnly())
			return e1->mInstanceStart.IsDateOnly();
		else
			return false;
	}
	else
		return e1->mInstanceStart < e2->mInstanceStart;
}

void CICalendarComponentExpanded::InitFromOwner(const CICalendarDateTime* rid)
{
	// There are four possibilities here:
	//
	// 1: this instance is the instance for the master component
	//
	// 2: this instance is an expanded instance derived directly from the master component
	//
	// 3: This instance is the instance for a slave (overridden recurrence instance)
	//
	// 4: This instance is the expanded instance for a slave with a RANGE parameter
	//

	// rid is not set if the owner is the master (case 1)
	if (rid == NULL)
	{
		// Just get start/end from owner
		mInstanceStart = mOwner->GetStart();
		mInstanceEnd = mOwner->GetEnd();
		mRecurring = false;
	}

	// If the owner is not a recurrence instance then it is case 2
	else if (!mOwner->IsRecurrenceInstance())
	{
		// Derive start/end from rid and duration of master

		// Start of the recurrence instance is the recurrence id
		mInstanceStart = *rid;
		
		// End is based on original events settings
		mInstanceEnd = mInstanceStart + (mOwner->GetEnd() - mOwner->GetStart());
		
		mRecurring = true;
	}
		
	// If the owner is a recurrence item and the passed in rid is the same as the component rid we have case 3
	else if (*rid == mOwner->GetRecurrenceID())
	{
		// Derive start/end directly from the owner
		mInstanceStart = mOwner->GetStart();
		mInstanceEnd = mOwner->GetEnd();
		
		mRecurring = true;
	}
	
	// case 4 - the complicated one!
	else
	{
		// We need to use the rid as the starting point, but adjust it by the offset between the slave's
		// rid and its start
		mInstanceStart = *rid + (mOwner->GetStart() - mOwner->GetRecurrenceID());
		
		// End is based on duration of owner
		mInstanceEnd = mInstanceStart + (mOwner->GetEnd() - mOwner->GetStart());
		
		mRecurring = true;
	}
}

bool CICalendarComponentExpanded::IsNow() const
{
	// Check instance start/end against current date-time
	CICalendarDateTime now = CICalendarDateTime::GetNowUTC();
	return (mInstanceStart <= now) && (mInstanceEnd > now);
}
