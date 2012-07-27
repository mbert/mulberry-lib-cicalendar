/*
    Copyright (c) 2007-2010 Cyrus Daboo. All rights reserved.
    
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
	CICalendar.h

	Author:
	Description:	<describe the CICalendar class here>
*/

#ifndef CICalendar_H
#define CICalendar_H

#include "CICalendarComponentBase.h"
#include "CBroadcaster.h"

#include "CICalendarComponentRecord.h"
#include "CICalendarComponent.h"
#include "CICalendarComponentDB.h"
#include "CICalendarFreeBusy.h"
#include "CICalendarPeriod.h"

#include "cdsharedptr.h"

#include <iostream>


namespace iCal {

class CICalendarProperty;
class CICalendarVEvent;
class CICalendarVTimezone;
class CICalendarVToDo;

class CICalendarComponentRecur;
typedef std::vector<CICalendarComponentRecur*> CICalendarComponentRecurs;

class CICalendarComponentExpanded;
typedef cdsharedptr<CICalendarComponentExpanded> CICalendarComponentExpandedShared;
typedef std::vector<CICalendarComponentExpandedShared> CICalendarExpandedComponents;

typedef uint32_t	CICalendarRef;	// Unique reference to object

class CICalendar : public CICalendarComponentBase, public CBroadcaster
{
public:
	// Messages for broadcast
	enum
	{
		// Regular messages
		eBroadcast_Changed = 'calc',
		eBroadcast_Edit = 'edit',
		eBroadcast_Closed = 'cald',
		
		// Feedback messages
		eBroadcast_AddedComponent = 'adde',
		eBroadcast_ChangedComponent = 'chan',
		eBroadcast_RemovedComponent = 'remo'
		
	};

	enum EFindComponent
	{
		eFindExact,
		eFindMaster
	};

	class CComponentAction
	{
	public:
		enum EOperation
		{
			eAdded,
			eChanged,
			eRemoved
		};

		CComponentAction(EOperation operation, CICalendar& calendar, const CICalendarComponent& component) :
			mOperation(operation), mCalendar(calendar), mComponent(component) {}
		
		~CComponentAction()  {}
		
		EOperation GetOperation() const
		{
			return mOperation;
		}
		
		CICalendar& GetCalendar() const
		{
			return mCalendar;
		}
		
		const CICalendarComponent& GetComponent() const
		{
			return mComponent;
		}
	
	private:
		EOperation					mOperation;
		CICalendar&					mCalendar;
		const CICalendarComponent&	mComponent;
	};

	static CICalendar&	getSICalendar();

	static CICalendar* GetICalendar(const CICalendarRef& ref);

	CICalendar();
	virtual ~CICalendar();

	const CICalendarRef& GetRef() const
		{ return mICalendarRef; }

	void Clear();

	cdstring& GetName()
		{ return mName; }
	const cdstring& GetName() const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }
	void EditName(const cdstring& name);

	cdstring& GetDescription()
		{ return mDescription; }
	const cdstring& GetDescription() const
		{ return mDescription; }
	void SetDescription(const cdstring& description)
		{ mDescription = description; }
	void EditDescription(const cdstring& description);

	cdstring GetMethod() const;

	virtual void Finalise();

	bool					Parse(std::istream& is);
	CICalendarComponent*	ParseComponent(std::istream& is, const cdstring& rurl, const cdstring& etag);
	virtual void			Generate(std::ostream& os, bool for_cache = false) const;
	virtual void			GenerateOne(std::ostream& os, const CICalendarComponent& comp) const;

	// Get components
	const CICalendarComponentDB& GetVEvents() const
	{
		return mVEvent;
	}
	const CICalendarComponentDB& GetVToDos() const
	{
		return mVToDo;
	}
	const CICalendarComponentDB& GetVJournals() const
	{
		return mVJournal;
	}
	const CICalendarComponentDB& GetVFreeBusy() const
	{
		return mVFreeBusy;
	}
	const CICalendarComponentDB& GetVTimezone() const
	{
		return mVTimezone;
	}

	void GetAllDBs(CICalendarComponentDBList& list) const
	{
		list.push_back(&mVEvent);
		list.push_back(&mVToDo);
		list.push_back(&mVJournal);
		list.push_back(&mVFreeBusy);
		list.push_back(&mVTimezone);
	}

	bool HasData() const;

	// Disconnected support
	const cdstring& GetETag() const
	{
		return mETag;
	}
	void SetETag(const cdstring& etag)
	{
		mETag = etag;
	}
	
	const cdstring& GetSyncToken() const
	{
		return mSyncToken;
	}
	void SetSyncToken(const cdstring& sync_token)
	{
		mSyncToken = sync_token;
	}
	
	const CICalendarComponentRecordDB& GetRecording() const
	{
		return mRecordDB;
	}
	void ClearRecording()
	{
		mRecordDB.clear();
	}
	bool NeedsSync() const
	{
		return !mRecordDB.empty();
	}
	void ClearSync();

	void	ParseCache(std::istream& is);
	void	GenerateCache(std::ostream& os) const;

	// Get expanded components
	void GetVEvents(const CICalendarPeriod& period, CICalendarExpandedComponents& list, bool all_day_at_top = true) const;
	void GetVToDos(bool only_due, bool all_dates, const CICalendarDateTime& upto_due_date, CICalendarExpandedComponents& list) const;
	void GetRecurrenceInstances(CICalendarComponent::EComponentType type, const cdstring& uid, CICalendarComponentRecurs& items) const;
	void GetRecurrenceInstances(CICalendarComponent::EComponentType type, const cdstring& uid, CICalendarDateTimeList& ids) const;

	// Freebusy generation
	void GetVFreeBusy(const CICalendarPeriod& period, CICalendarComponentList& list) const;
	void GetVFreeBusy(const CICalendarPeriod& period, CICalendarComponent& fb) const;
	void GetFreeBusy(const CICalendarPeriod& period, CICalendarFreeBusyList& fb) const;
	void GetFreeBusyOnly(CICalendarFreeBusyList& fb) const;
	
	// Timezone lookups
	void	MergeTimezones(const CICalendar& cal);
	int32_t GetTimezoneOffsetSeconds(const cdstring& timezone, const CICalendarDateTime& dt);
	cdstring GetTimezoneDescriptor(const cdstring& timezone, const CICalendarDateTime& dt);
	void	GetTimezones(cdstrvect& tzids) const;
	void	SortTimezones(cdstrvect& tzids) const;
	const CICalendarVTimezone* GetTimezone(const cdstring& tzid) const;
	bool ValidEDST(cdstrvect& tzids) const;
	void UpgradeEDST();

	// Add/remove components
	enum ERemoveRecurring
	{
		eRemoveAll = 0,
		eRemoveOnlyThis,
		eRemoveThisFuture
	};

	void	ChangedComponent(CICalendarComponent* comp);

	void	AddNewVEvent(CICalendarVEvent* vevent, bool moved = false);
	void	RemoveVEvent(CICalendarVEvent* vevent, bool delete_it = true);
	void	RemoveVEvent(const cdstring& uid, bool delete_it = true);
	void	RemoveRecurringVEvent(CICalendarComponentExpandedShared& vevent, ERemoveRecurring recur);

	void	AddNewVToDo(CICalendarVToDo* vtodo, bool moved = false);
	void	RemoveVToDo(CICalendarVToDo* vtodo, bool delete_it = true);

	CICalendarComponent* FindComponent(const CICalendarComponent* orig, EFindComponent find = eFindExact) const;
	void AddComponent(CICalendarComponent* comp);
	const CICalendarComponent* GetComponentByKey(const cdstring& mapkey) const;
	CICalendarComponent* GetComponentByKey(const cdstring& mapkey);
	void RemoveComponentByKey(const cdstring& mapkey);

	bool	IsReadOnly() const
	{
		return mReadOnly;
	}
	void	SetReadOnly(bool ro = true)
	{
		mReadOnly = ro;
	}

	// Change state
	bool	IsDirty() const
	{
		return mDirty;
	}
	void	SetDirty(bool dirty = true)
	{
		mDirty = dirty;
	}

	bool	IsTotalReplace() const
	{
		return mTotalReplace;
	}
	void	SetTotalReplace(bool replace = true)
	{
		mTotalReplace = replace;
	}

protected:
	typedef std::map<CICalendarRef, CICalendar*>	CICalendarRefMap;

	static CICalendarRefMap	sICalendars;
	static CICalendarRef 	sICalendarRefCtr;

	CICalendarRef			mICalendarRef;

	bool					mReadOnly;
	bool					mDirty;
	bool					mTotalReplace;

	cdstring				mName;
	cdstring				mDescription;
	
	CICalendarComponentDB		mVEvent;
	CICalendarComponentDB		mVToDo;
	CICalendarComponentDB		mVJournal;
	CICalendarComponentDB		mVFreeBusy;
	CICalendarComponentDB		mVTimezone;
	
	// Pseudo properties used for disconnected cache
	cdstring					mETag;
	cdstring					mSyncToken;
	CICalendarComponentRecordDB	mRecordDB;

	CICalendarComponentDB&	GetComponents(CICalendarComponent::EComponentType type);
	const CICalendarComponentDB& GetComponents(CICalendarComponent::EComponentType type) const
	{
		return const_cast<CICalendar*>(this)->GetComponents(type);
	}

	void	AddDefaultProperties();
	void	Generate(std::ostream& os, const CICalendarComponentDB& components, bool for_cache) const;
	void	Erase(CICalendarComponentDB& components);

	bool	ValidProperty(const CICalendarProperty& prop) const;
	bool	IgnoreProperty(const CICalendarProperty& prop) const;

	void	IncludeTimezones();
	void	IncludeTimezones(const CICalendarComponentDB& components, cdstrset& tzids);

	CICalendarComponent* FindComponent(const CICalendarComponentDB& db, const CICalendarComponent* orig, EFindComponent find = eFindExact) const;
	void AddComponent(CICalendarComponentDB& db, CICalendarComponent* comp);
	const CICalendarComponent* GetComponentByKey(const CICalendarComponentDB& db, const cdstring& mapkey) const;
	CICalendarComponent* GetComponentByKey(CICalendarComponentDB& db, const cdstring& mapkey);
	bool RemoveComponentByKey(CICalendarComponentDB& db, const cdstring& mapkey);

private:
	struct SComponentRegister
	{
		CICalendarComponent::CreateComponentPP	mCreatePP;
		CICalendarComponent::EComponentType 	mType;

		SComponentRegister(CICalendarComponent::CreateComponentPP proc, CICalendarComponent::EComponentType type)
			: mCreatePP(proc), mType(type) {}
	};
	typedef std::map<cdstring, SComponentRegister*> CComponentRegisterMap;
	static CComponentRegisterMap	sComponents;
	static CComponentRegisterMap	sEmbeddedComponents;

	void InitComponents();
	void InitDefaultTimezones();
};

typedef std::vector<CICalendar*> CICalendarList;

}	// namespace iCal

#endif	// CICalendar_H
