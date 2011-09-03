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
	CICalendarVToDo.cpp

	Author:
	Description:	<describe the CICalendarVToDo class here>
*/

#include "CICalendarVToDo.h"

#include "CICalendarComponentExpanded.h"
#include "CICalendarDuration.h"

#include <strstream>

using namespace iCal;

cdstring CICalendarVToDo::sBeginDelimiter(cICalComponent_BEGINVTODO);
cdstring CICalendarVToDo::sEndDelimiter(cICalComponent_ENDVTODO);

void CICalendarVToDo::_init_CICalendarVToDo()
{
	mPriority = 0;
	mStatus = eStatus_VToDo_None;
	mPercentComplete = 0;
	mHasCompleted = false;
}

void CICalendarVToDo::_copy_CICalendarVToDo(const CICalendarVToDo& copy)
{
	mPriority = copy.mPriority;
	mStatus = copy.mStatus;
	mPercentComplete = copy.mPercentComplete;
	mCompleted = copy.mCompleted;
	mHasCompleted = copy.mHasCompleted;
}

bool CICalendarVToDo::sort_for_display(const CICalendarComponentExpandedShared& e1, const CICalendarComponentExpandedShared& e2)
{
	const CICalendarVToDo* s1 = e1->GetMaster<CICalendarVToDo>();
	const CICalendarVToDo* s2 = e2->GetMaster<CICalendarVToDo>();

	// Check status first (convert None -> Needs action for tests)
	EStatus_VToDo status1 = s1->mStatus;
	EStatus_VToDo status2 = s2->mStatus;
	if (status1 == eStatus_VToDo_None)
		status1 = eStatus_VToDo_NeedsAction;
	if (status2 == eStatus_VToDo_None)
		status2 = eStatus_VToDo_NeedsAction;
    if (s1->HasCompleted())
        status1 = eStatus_VToDo_Completed;
    if (s2->HasCompleted())
        status2 = eStatus_VToDo_Completed;
	if (status1 != status2)
	{
		// More important ones at the top
		return status1 < status2;
	}

	// At this point the status of each is the same

	// If status is cancelled sort by start time
	if (s1->mStatus == eStatus_VToDo_Cancelled)
	{
		// Older ones at the bottom
		return s1->mStart > s2->mStart;
	}

	// If status is completed sort by completion time
	if (s1->mStatus == eStatus_VToDo_Completed)
	{
		// Older ones at the bottom
		return s1->mCompleted > s2->mCompleted;
	}

	// Check due date exists
	if (s1->mHasEnd != s2->mHasEnd)
	{
		CICalendarDateTime now;
		now.SetToday();
		
		// Ones with due dates after today below ones without due dates
		if (s1->HasEnd())
			return s1->mEnd <= now;
		else if (s2->HasEnd())
			return now < s2->mEnd;
	}

	// Check due dates if present
	if (s1->mHasEnd)
	{
		if (s1->mEnd != s2->mEnd)
		{
			// Soonest dues dates above later ones
			return s1->mEnd < s2->mEnd;
		}
	}

	// Check priority next
	if (s1->mPriority != s2->mPriority)
	{
		// Higher priority above lower ones
		return s1->mPriority < s2->mPriority;
	}

	// Just use start time - older ones at the top
	return s1->mStart < s2->mStart;
}

bool CICalendarVToDo::AddComponent(CICalendarComponent* comp)
{
	// We can embed the alarm components only
	if (comp->GetType() == eVALARM)
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

void CICalendarVToDo::Finalise()
{
	// Do inherited
	CICalendarComponentRecur::Finalise();

	// Get DUE
	if (!LoadValue(cICalProperty_DUE, mEnd))
	{
		// Try DURATION instead
		CICalendarDuration temp;
		if (LoadValue(cICalProperty_DURATION, temp))
		{
			mEnd = mStart + temp;
			mHasEnd = true;
		}
		else
			mHasEnd = false;
	}
	else
		mHasEnd = true;

	// Get PRIORITY
	LoadValue(cICalProperty_PRIORITY, mPriority);

	// Get STATUS
	{
		cdstring temp;
		if (LoadValue(cICalProperty_STATUS, temp))
		{
			if (temp == cICalProperty_STATUS_NEEDS_ACTION)
			{
				mStatus = eStatus_VToDo_NeedsAction;
			}
			else if (temp == cICalProperty_STATUS_COMPLETED)
			{
				mStatus = eStatus_VToDo_Completed;
			}
			else if (temp == cICalProperty_STATUS_IN_PROCESS)
			{
				mStatus = eStatus_VToDo_InProcess;
			}
			else if (temp == cICalProperty_STATUS_CANCELLED)
			{
				mStatus = eStatus_VToDo_Cancelled;
			}
		}
	}

	// Get PERCENT-COMPLETE
	LoadValue(cICalProperty_PERCENT_COMPLETE, mPercentComplete);

	// Get COMPLETED
	mHasCompleted = LoadValue(cICalProperty_COMPLETED, mCompleted);
}

cdstring CICalendarVToDo::GetStatusText() const
{
	std::ostrstream sout;
	
    if (HasCompleted())
    {
        sout << "Completed: " << GetCompleted().GetLocaleDate(CICalendarDateTime::eNumericDate);
    }
    else
    {
        switch(mStatus)
        {
        case eStatus_VToDo_NeedsAction:
        case eStatus_VToDo_InProcess:
        default:
            if (HasEnd())
            {
                // Check due date
                CICalendarDateTime today;
                today.SetToday();
                if (GetEnd() > today)
                {
                    sout << "Due: ";
                    CICalendarDuration whendue = GetEnd() - today;
                    if ((whendue.GetDays() > 0) && (whendue.GetDays() <= 7))
                        sout << whendue.GetDays() << " days";
                    else
                        sout << GetEnd().GetLocaleDate(CICalendarDateTime::eNumericDate);
                }
                else if (GetEnd() == today)
                    sout << "Due today";
                else
                {
                    sout << "Overdue: ";
                    CICalendarDuration overdue = today - GetEnd();
                    if (overdue.GetWeeks() != 0)
                        sout << overdue.GetWeeks() << " weeks";
                    else
                        sout << overdue.GetDays() + 1 << " days";
                }
            }
            else
                sout << "Not Completed";
            break;
        case eStatus_VToDo_Completed:
            if (HasCompleted())
            {
                sout << "Completed: " << GetCompleted().GetLocaleDate(CICalendarDateTime::eNumericDate);
            }
            else
                sout << "Completed";
            break;
        case eStatus_VToDo_Cancelled:
            sout << "Cancelled";
            break;
        }
    }
	
	sout << std::ends;
	cdstring result;
	result.steal(sout.str());
	return result;
}

CICalendarVToDo::ECompletedStatus CICalendarVToDo::GetCompletionState() const
{
    if (HasCompleted())
        return eDone;

	switch(mStatus)
	{
	case eStatus_VToDo_NeedsAction:
	case eStatus_VToDo_InProcess:
	default:
		if (HasEnd())
		{
			// Check due date
			CICalendarDateTime today;
			today.SetToday();
			if (GetEnd() > today)
				return eDueLater;
			else if (GetEnd() == today)
				return eDueNow;
			else
				return eOverdue;
		}
		else
			return eDueNow;
	case eStatus_VToDo_Completed:
		return eDone;
	case eStatus_VToDo_Cancelled:
		return eCancelled;
	}
}

void CICalendarVToDo::EditStatus(EStatus_VToDo status)
{
	// Only if it is different
	if (mStatus != status)
	{
		// Updated cached values
		mStatus = status;

		// Remove existing STATUS & COMPLETED items
		RemoveProperties(cICalProperty_STATUS);
		RemoveProperties(cICalProperty_COMPLETED);
		mHasCompleted = false;

		// Now create properties
		{
			const char* value = NULL;
			switch(status)
			{
			case eStatus_VToDo_NeedsAction:
			default:
				value = cICalProperty_STATUS_NEEDS_ACTION;
				break;
			case eStatus_VToDo_Completed:
				value = cICalProperty_STATUS_COMPLETED;
				{
					// Add the completed item
					mCompleted.SetNowUTC();
					mHasCompleted = true;
					CICalendarProperty prop(cICalProperty_STATUS_COMPLETED, mCompleted);
					AddProperty(prop);
				}
				break;
			case eStatus_VToDo_InProcess:
				value = cICalProperty_STATUS_IN_PROCESS;
				break;
			case eStatus_VToDo_Cancelled:
				value = cICalProperty_STATUS_CANCELLED;
				break;
			}
			CICalendarProperty prop(cICalProperty_STATUS, value);
			AddProperty(prop);
		}
	}
}

void CICalendarVToDo::EditCompleted(const CICalendarDateTime& completed)
{
	// Remove existing COMPLETED item
	RemoveProperties(cICalProperty_COMPLETED);
	mHasCompleted = false;

	// Always UTC
	mCompleted = completed;
	mCompleted.AdjustToUTC();
	mHasCompleted = true;
	CICalendarProperty prop(cICalProperty_STATUS_COMPLETED, mCompleted);
	AddProperty(prop);
}
