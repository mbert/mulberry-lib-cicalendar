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
	CICalendarComponentRecur.h

	Author:
	Description:	Base class for iCal components that can have recurrence
*/

#ifndef CICalendarComponentRecur_H
#define CICalendarComponentRecur_H

#include "CICalendarComponent.h"

#include "CICalendarDateTime.h"

#include "cdsharedptr.h"
#include "ptrvector.h"

namespace iCal {

class CICalendarRecurrenceSet;

class CICalendarComponentExpanded;
typedef cdsharedptr<CICalendarComponentExpanded> CICalendarComponentExpandedShared;
typedef std::vector<CICalendarComponentExpandedShared> CICalendarExpandedComponents;

class CICalendarComponentRecur;
typedef std::vector<CICalendarComponentRecur*> CICalendarComponentRecurs;

class CICalendarComponentRecur : public CICalendarComponent
{
public:

	static cdstring MapKey(const cdstring& uid);
	static cdstring MapKey(const cdstring& uid, const cdstring& rid);

	static bool sort_by_dtstart_allday(CICalendarComponentRecur* e1, CICalendarComponentRecur* e2);
	static bool sort_by_dtstart(CICalendarComponentRecur* e1, CICalendarComponentRecur* e2);

	CICalendarComponentRecur(const CICalendarRef& calendar);
	CICalendarComponentRecur(const CICalendarComponentRecur& copy);
	virtual ~CICalendarComponentRecur()
	{
		_tidy_CICalendarComponentRecur();
	}

	CICalendarComponentRecur& operator=(const CICalendarComponentRecur& copy)
		{
			if (this != &copy)
			{
				_tidy_CICalendarComponentRecur();
				_copy_CICalendarComponentRecur(copy);
				CICalendarComponent::operator=(copy);
			}
			return *this;
		}

	virtual bool CanGenerateInstance() const
	{
		return !mHasRecurrenceID;
	}

	bool Recurring() const
	{
		return (mMaster != NULL) && (mMaster != this);
	}
	void SetMaster(CICalendarComponentRecur* master);
	CICalendarComponentRecur* GetMaster()
	{
		return mMaster;
	}
	const CICalendarComponentRecur* GetMaster() const
	{
		return mMaster;
	}

	virtual const cdstring& GetMapKey() const
	{
		return mMapKey;
	}
	virtual cdstring GetMasterKey() const;

	virtual void InitDTSTAMP();

	const CICalendarDateTime& GetStamp() const
		{ return mStamp; }
	bool HasStamp() const
		{ return mHasStamp; }

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

	bool IsRecurrenceInstance() const
	{
		return mHasRecurrenceID;
	}
	bool IsAdjustFuture() const
	{
		return mAdjustFuture;
	}
	bool IsAdjustPrior() const
	{
		return mAdjustPrior;
	}
	const CICalendarDateTime& GetRecurrenceID() const
	{
		return mRecurrenceID;
	}

	bool IsRecurring() const;

	CICalendarRecurrenceSet* GetRecurrenceSet()
	{
		return mRecurrences;
	}
	const CICalendarRecurrenceSet* GetRecurrenceSet() const
	{
		return mRecurrences;
	}

	virtual void SetUID(const cdstring& uid);

	const cdstring& GetSummary() const
		{ return mSummary; }
	void SetSummary(const cdstring& summary)
		{ mSummary = summary; }

	cdstring GetDescription() const;
	cdstring GetLocation() const;

	bool	 GetTransparent() const;

	virtual void Finalise();

			void ExpandPeriod(const CICalendarPeriod& period, CICalendarExpandedComponents& list);

			bool WithinPeriod(const CICalendarPeriod& period) const;

			void ChangedRecurrence();

	// Editing
			void EditSummary(const cdstring& summary);
			void EditDetails(const cdstring& description, const cdstring& location);
			void EditTransparent(bool transparent);
			void EditTiming();
			void EditTiming(const CICalendarDateTime& due);
			void EditTiming(const CICalendarDateTime& start, const CICalendarDateTime& end);
			void EditTiming(const CICalendarDateTime& start, const CICalendarDuration& durtaion);
			void EditRecurrenceSet(const CICalendarRecurrenceSet& recurs);
			void ExcludeRecurrence(const CICalendarDateTime& start);
			void ExcludeFutureRecurrence(const CICalendarDateTime& start);

protected:
	CICalendarComponentRecur*	mMaster;		// Component to inherit properties from if none exist in this component

	cdstring				mMapKey;

	cdstring				mSummary;

	CICalendarDateTime		mStamp;
	bool					mHasStamp;

	CICalendarDateTime		mStart;
	bool					mHasStart;
	CICalendarDateTime		mEnd;
	bool					mHasEnd;
	bool					mDuration;

	bool					mHasRecurrenceID;
	bool					mAdjustFuture;
	bool					mAdjustPrior;
	CICalendarDateTime		mRecurrenceID;

	CICalendarRecurrenceSet*			mRecurrences;

	// These are overridden to allow missing properties to come from the master component
	virtual bool	LoadValue(const char* value_name, int32_t& value, CICalendarValue::EICalValueType type = CICalendarValue::eValueType_Integer) const;
	virtual bool	LoadValue(const char* value_name, cdstring& value) const;
	virtual bool	LoadValue(const char* value_name, CICalendarDateTime& value) const;
	virtual bool	LoadValue(const char* value_name, CICalendarDuration& value) const;
	virtual bool	LoadValue(const char* value_name, CICalendarPeriod& value) const;
	virtual bool	LoadValueRRULE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const;
	virtual bool	LoadValueRDATE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const;

			void	InitFromMaster();

	CICalendarComponentExpandedShared	CreateExpanded(CICalendarComponentRecur* master, const CICalendarDateTime& recurid);

private:
	void	_copy_CICalendarComponentRecur(const CICalendarComponentRecur& copy);
	void	_tidy_CICalendarComponentRecur();
	
	void	FixStartEnd();
};

}	// namespace iCal

#endif	// CICalendarComponentRecur_H
