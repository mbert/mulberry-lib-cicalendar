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
	CITIPScheduleResults.h

	Author:			
	Description:	<describe the CITIPScheduleResults class here>
*/

#ifndef CITIPScheduleResults_H
#define CITIPScheduleResults_H

#include "CICalendarFreeBusy.h"

#include "cdstring.h"

namespace iCal
{

class CICalendar;

class CITIPScheduleResults
{
public:
	CITIPScheduleResults(const cdstring& recipient, const cdstring& status);
	CITIPScheduleResults(const cdstring& recipient,
						 const cdstring& status,
						 const iCal::CICalendar& cal);
	CITIPScheduleResults(const CITIPScheduleResults& copy);
	virtual ~CITIPScheduleResults();
	
	const cdstring& GetRecipient() const
	{
		return mRecipient;
	}
	
	const cdstring& GetStatus() const
	{
		return mStatus;
	}
	
	const cdstring& GetAttendee() const
	{
		return mAttendee;
	}
	
	const iCal::CICalendarFreeBusyList& GetPeriods() const
	{
		return mPeriods;
	}

protected:
	cdstring						mRecipient;
	cdstring						mStatus;
	cdstring						mAttendee;
	iCal::CICalendarFreeBusyList	mPeriods;

	// Calendar data
	virtual void ParseCalendarData(const iCal::CICalendar& cal);

};

typedef std::vector<CITIPScheduleResults>	CITIPScheduleResultsList;

}	// namespace iCal

#endif	// CITIPScheduleResults_H
