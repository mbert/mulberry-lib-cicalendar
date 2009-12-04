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
	CICalendarComponentExpanded.h

	Author:
	Description:	Base class for iCal recurrence expanded components.
	                This is a template based wrapper that contains a pointer back to the
	                master/slave component that generated this recurrence instance
*/

#ifndef CICalendarComponentExpanded_H
#define CICalendarComponentExpanded_H

#include "cdsharedptr.h"

#include "CICalendarDateTime.h"
#include "CICalendarComponentRecur.h"

namespace iCal {

class CICalendarComponentExpanded;
typedef cdsharedptr<CICalendarComponentExpanded> CICalendarComponentExpandedShared;
typedef std::vector<CICalendarComponentExpandedShared> CICalendarExpandedComponents;

class CICalendarComponentExpanded
{
public:

	static bool sort_by_dtstart_allday(const CICalendarComponentExpandedShared& e1, const CICalendarComponentExpandedShared& e2);
	static bool sort_by_dtstart(const CICalendarComponentExpandedShared& e1, const CICalendarComponentExpandedShared& e2);

	CICalendarComponentExpanded(CICalendarComponentRecur* owner, const CICalendarDateTime* rid)
	{
		mOwner = owner;
		InitFromOwner(rid);
	}
	CICalendarComponentExpanded(const CICalendarComponentExpanded& copy)
	{
		_copy_CICalendarComponentExpanded(copy);
	}
	virtual ~CICalendarComponentExpanded() {}

	CICalendarComponentExpanded& operator=(const CICalendarComponentExpanded& copy)
	{
		if (this != &copy)
		{
			_copy_CICalendarComponentExpanded(copy);
		}
		return *this;
	}

	CICalendarComponentRecur* GetOwner()
	{
		return mOwner;
	}
	const CICalendarComponentRecur* GetOwner() const
	{
		return mOwner;
	}

	template<class T> T* GetMaster()
	{
		return static_cast<T*>(mOwner);
	}
	template<class T> const T* GetMaster() const
	{
		return static_cast<const T*>(mOwner);
	}

	template<class T> T* GetTrueMaster()
	{
		return static_cast<T*>(mOwner->GetMaster());
	}
	template<class T> const T* GetTrueMaster() const
	{
		return static_cast<const T*>(mOwner->GetMaster());
	}

	const CICalendarDateTime& GetInstanceStart() const
		{ return mInstanceStart; }

	const CICalendarDateTime& GetInstanceEnd() const
		{ return mInstanceEnd; }

	bool Recurring() const
	{
		return mRecurring;
	}

	bool IsNow() const;

protected:
	CICalendarComponentRecur*	mOwner;

	CICalendarDateTime			mInstanceStart;
	CICalendarDateTime			mInstanceEnd;

	bool						mRecurring;

			void	InitFromOwner(const CICalendarDateTime* rid);

private:
	void	_copy_CICalendarComponentExpanded(const CICalendarComponentExpanded& copy)
	{
		mOwner = copy.mOwner;
		mInstanceStart = copy.mInstanceStart;
		mInstanceEnd = copy.mInstanceEnd;
		mRecurring = copy.mRecurring;
	}
};

}	// namespace iCal

#endif	// CICalendarComponentExpanded_H
