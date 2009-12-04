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
	CICalendarFreeBusy.h

	Author:
	Description:	<describe the CICalendarFreeBusy class here>
*/

#ifndef CICalendarFreeBusy_H
#define CICalendarFreeBusy_H

#include "CICalendarPeriod.h"

namespace iCal {

class CICalendarFreeBusy
{
public:
	enum EBusyType
	{
		eFree = 0,
		eBusyTentative,
		eBusyUnavailable,
		eBusy
	};

	CICalendarFreeBusy()
	{
		mType = eFree;
	}
	CICalendarFreeBusy(EBusyType type)
	{
		mType = type;
	}
	CICalendarFreeBusy(EBusyType type, const iCal::CICalendarPeriod& period)
	{
		mType = type;
		mPeriod = period;
	}
	CICalendarFreeBusy(const CICalendarFreeBusy& copy)
		{ _copy_CICalendarFreeBusy(copy); }
	~CICalendarFreeBusy() {}

	CICalendarFreeBusy& operator=(const CICalendarFreeBusy& copy)
		{ if (this != &copy) _copy_CICalendarFreeBusy(copy); return *this; }

	int operator<(const CICalendarFreeBusy& comp) const
		{ return (mPeriod < comp.mPeriod); }

	void SetType(EBusyType type)								// Set type
		{ mType = type; }
	EBusyType	GetType() const									// Get type
		{ return mType; }

	void	SetPeriod(const iCal::CICalendarPeriod& period)		// Set period
		{ mPeriod = period; }
	const iCal::CICalendarPeriod& GetPeriod() const				// Get periods
		{ return mPeriod; }

	bool IsPeriodOverlap(const CICalendarPeriod& period) const
		{ return mPeriod.IsPeriodOverlap(period); }

	static void ResolveOverlaps(std::vector<CICalendarFreeBusy>& fb);
	//void Merge(const CICalendarFreeBusy& fb);

protected:
	EBusyType				mType;							// Type of busy
	CICalendarPeriod		mPeriod;						// Period for busy time

private:
	void _copy_CICalendarFreeBusy(const CICalendarFreeBusy& copy)
		{ mType = copy.mType; mPeriod = copy.mPeriod; }

};

typedef std::vector<CICalendarFreeBusy> CICalendarFreeBusyList;

}	// namespace iCal

#endif	// CICalendarFreeBusy_H
