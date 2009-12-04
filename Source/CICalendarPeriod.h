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
	CICalendarPeriod.h

	Author:
	Description:	<describe the CICalendarPeriod class here>
*/

#ifndef CICalendarPeriod_H
#define CICalendarPeriod_H

#include "CICalendarDateTime.h"
#include "CICalendarDuration.h"

namespace iCal {

class CICalendarPeriod
{
public:
	CICalendarPeriod()
		{ mUseDuration = false; }
	CICalendarPeriod(const CICalendarDateTime& start, const CICalendarDateTime& end)
		{ mStart = start; mEnd = end; mDuration = mEnd - mStart; mUseDuration = false; }
	CICalendarPeriod(const CICalendarDateTime& start, const CICalendarDuration& duration)
		{ mStart = start, mDuration = duration; mEnd = mStart + mDuration; mUseDuration = true; }
	CICalendarPeriod(const CICalendarPeriod& copy)
		{ _copy_CICalendarPeriod(copy); }
	virtual ~CICalendarPeriod() {}

	CICalendarPeriod& operator=(const CICalendarPeriod& copy)
		{ if (this != &copy) _copy_CICalendarPeriod(copy); return *this; }

	int operator==(const CICalendarPeriod& comp) const
		{ return (mStart == comp.mStart) && (mEnd == comp.mEnd) ? 1 : 0; }
	int operator>(const CICalendarDateTime& comp) const
		{ return mStart > comp; }
	int operator<(const CICalendarPeriod& comp) const
		{ return (mStart < comp.mStart) || ((mStart == comp.mStart) && (mEnd < comp.mEnd)); }

	void Parse(const cdstring& data);
	void Generate(std::ostream& os) const;

	const CICalendarDateTime& GetStart() const
		{ return mStart; }

	const CICalendarDateTime& GetEnd() const
		{ return mEnd; }

	const CICalendarDuration& GetDuration() const
		{ return mDuration; }

	bool IsDateWithinPeriod(const CICalendarDateTime& dt) const
	{
		// Inclusive start, exclusive end
		return (dt >= mStart) && (dt < mEnd);
	}
	bool IsDateBeforePeriod(const CICalendarDateTime& dt) const
	{
		// Inclusive start
		return dt < mStart;
	}
	bool IsDateAfterPeriod(const CICalendarDateTime& dt) const
	{
		// Exclusive end
		return dt >= mEnd;
	}

	bool IsPeriodOverlap(const CICalendarPeriod& p) const
	{
		// Inclusive start, exclusive end
		return !((mStart >= p.mEnd) || (mEnd <= p.mStart));
	}

	cdstring DescribeDuration() const;

protected:
	CICalendarDateTime	mStart;
	CICalendarDateTime	mEnd;
	CICalendarDuration	mDuration;
	bool				mUseDuration;

private:
	void _copy_CICalendarPeriod(const CICalendarPeriod& copy)
		{ mStart = copy.mStart; mEnd = copy.mEnd; mDuration = copy.mDuration; mUseDuration = copy.mUseDuration; }

};

typedef std::vector<CICalendarPeriod> CICalendarPeriodList;

}	// namespace iCal

#endif	// CICalendarPeriod_H
