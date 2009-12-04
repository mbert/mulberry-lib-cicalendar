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
	CICalendarVTimezoneElement.h

	Author:
	Description:	<describe the CICalendarVTimezoneElement class here>
*/

#ifndef CICalendarVTimezoneElement_H
#define CICalendarVTimezoneElement_H

#include "CICalendarVTimezone.h"

#include "CICalendarDateTime.h"
#include "CICalendarRecurrenceSet.h"

namespace iCal {

class CICalendarVTimezoneElement: public CICalendarVTimezone
{
public:
	static bool sort_dtstart(const CICalendarComponent* s1, const CICalendarComponent* s2);

	CICalendarVTimezoneElement(const CICalendarRef& calendar) :
		CICalendarVTimezone(calendar)
		{ mUTCOffset = 0; }
	CICalendarVTimezoneElement(const CICalendarRef& calendar, const CICalendarDateTime& dt, int32_t offset = 0) :
		CICalendarVTimezone(calendar)
		{ mStart = dt; mUTCOffset = offset; mCachedExpandBelow = mStart; }
	CICalendarVTimezoneElement(const CICalendarVTimezoneElement& copy) :
		CICalendarVTimezone(copy)
		{ _copy_CICalendarVTimezoneElement(copy); }
	virtual ~CICalendarVTimezoneElement() {}

	CICalendarVTimezoneElement& operator=(const CICalendarVTimezoneElement& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVTimezoneElement(copy);
				CICalendarVTimezone::operator=(copy);
			}
			return *this;
		}

	virtual void Finalise();

	const CICalendarDateTime& GetStart() const
		{ return mStart; }

	int32_t	GetUTCOffset() const
		{ return mUTCOffset; }

	const cdstring&	GetTZName() const
		{ return mTZName; }

	CICalendarDateTime ExpandBelow(const CICalendarDateTime& below) const;

	CICalendarRecurrenceSet* GetRecurrenceSet()
	{
		return &mRecurrences;
	}
	const CICalendarRecurrenceSet* GetRecurrenceSet() const
	{
		return &mRecurrences;
	}
	
protected:
	CICalendarDateTime		mStart;
	int32_t					mUTCOffset;
	cdstring				mTZName;
	CICalendarRecurrenceSet	mRecurrences;
	mutable CICalendarDateTime		mCachedExpandBelow;
	mutable CICalendarDateTimeList	mCachedExpandBelowItems;

private:
	void	_copy_CICalendarVTimezoneElement(const CICalendarVTimezoneElement& copy)
		{ mStart = copy.mStart; mUTCOffset = copy.mUTCOffset; mTZName = copy.mTZName; mRecurrences = copy.mRecurrences; mCachedExpandBelow = mStart; }
};

}	// namespace iCal

#endif	// CICalendarVTimezoneElement_H
