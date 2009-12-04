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
	CICalendarVFreeBusy.h

	Author:
	Description:	<describe the CICalendarVFreeBusy class here>
*/

#ifndef CICalendarVFreeBusy_H
#define CICalendarVFreeBusy_H

#include "CICalendarComponent.h"

#include "CICalendarDateTime.h"
#include "CICalendarFreeBusy.h"
#include "CICalendarPeriod.h"
#include "CITIPDefinitions.h"

namespace iCal {

class CICalendarVFreeBusy: public CICalendarComponent
{
public:
	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVFreeBusy(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	CICalendarVFreeBusy(const CICalendarRef& calendar) :
		CICalendarComponent(calendar)
		{ _init_CICalendarVFreeBusy(); }
	CICalendarVFreeBusy(const CICalendarVFreeBusy& copy) :
		CICalendarComponent(copy)
		{ _copy_CICalendarVFreeBusy(copy); }
	virtual ~CICalendarVFreeBusy()
		{ _tidy_CICalendarVFreeBusy(); }

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVFreeBusy(*this);
	}

	CICalendarVFreeBusy& operator=(const CICalendarVFreeBusy& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVFreeBusy(copy);
				CICalendarComponent::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVFREEBUSY; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }
	virtual cdstring GetMimeComponentName() const
	{
		return cICalMIMEComponent_VFREEBUSY;
	}

	virtual void Finalise();

	const CICalendarDateTime& GetStart() const
		{ return mStart; }
	bool HasStart() const
		{ return mHasStart; }

	const CICalendarDateTime& GetEnd() const
		{ return mEnd; }
	bool HasEnd() const
		{ return mHasEnd; }

	bool UseDuration() const
		{ return mDuration; }

	const CICalendarPeriod& GetSpanPeriod() const
		{ return mSpanPeriod; }
	const CICalendarFreeBusyList* GetBusyTime() const
		{ return mBusyTime; }

	// Editing
	void EditTiming();
	void EditTiming(const CICalendarDateTime& start, const CICalendarDateTime& end);
	void EditTiming(const CICalendarDateTime& start, const CICalendarDuration& durtaion);

	// Generating info
	void ExpandPeriod(const CICalendarPeriod& period, CICalendarComponentList& list);
	void ExpandPeriod(const CICalendarPeriod& period, CICalendarFreeBusyList& list);
	void GetPeriod(CICalendarFreeBusyList& list);

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;

	CICalendarDateTime		mStart;
	bool					mHasStart;
	CICalendarDateTime		mEnd;
	bool					mHasEnd;
	bool					mDuration;

	bool					mCachedBusyTime;
	CICalendarPeriod		mSpanPeriod;
	CICalendarFreeBusyList*	mBusyTime;

private:
	void	_init_CICalendarVFreeBusy();
	void	_copy_CICalendarVFreeBusy(const CICalendarVFreeBusy& copy);
	void	_tidy_CICalendarVFreeBusy();
	
	void	FixStartEnd();
	
	void	CacheBusyTime();
};

}	// namespace iCal

#endif	// CICalendarVFreeBusy_H
