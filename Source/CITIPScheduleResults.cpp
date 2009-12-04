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
	CITIPScheduleResults.cpp

	Author:			
	Description:	<describe the CITIPScheduleResults class here>
*/

#include "CITIPScheduleResults.h"

#include "CICalendar.h"
#include "CICalendarProperty.h"
#include "CITIPProcessor.h"

using namespace iCal; 

CITIPScheduleResults::CITIPScheduleResults(const cdstring& recipient, const cdstring& status) :
	mRecipient(recipient), mStatus(status)
{
}

CITIPScheduleResults::CITIPScheduleResults(const cdstring& recipient,
										   const cdstring& status,
										   const iCal::CICalendar& cal) :
	mRecipient(recipient), mStatus(status)
{
	ParseCalendarData(cal);
}

CITIPScheduleResults::CITIPScheduleResults(const CITIPScheduleResults& copy)
{
	mRecipient = copy.mRecipient;
	mStatus = copy.mStatus;
	mAttendee = copy.mAttendee;
	mPeriods = copy.mPeriods;
}

CITIPScheduleResults::~CITIPScheduleResults()
{
}

void CITIPScheduleResults::ParseCalendarData(const iCal::CICalendar& cal)
{
	for(CICalendarComponentDB::const_iterator iter1 = cal.GetVFreeBusy().begin(); iter1 != cal.GetVFreeBusy().end(); iter1++)
	{
		std::pair<iCal::CICalendarPropertyMap::const_iterator, iCal::CICalendarPropertyMap::const_iterator> result = (*iter1).second->GetProperties().equal_range(iCal::cICalProperty_ATTENDEE);
		for(iCal::CICalendarPropertyMap::const_iterator iter2 = result.first; iter2 != result.second; iter2++)
		{
			mAttendee = iCal::CITIPProcessor::GetAttendeeDescriptor((*iter2).second);
			break;
		}
	}
	cal.GetFreeBusyOnly(mPeriods);
}
