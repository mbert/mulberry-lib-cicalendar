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
	CICalendarVToDo.h

	Author:
	Description:	<describe the CICalendarVToDo class here>
*/

#ifndef CICalendarVToDo_H
#define CICalendarVToDo_H

#include "CICalendarComponentRecur.h"

#include "CICalendarDefinitions.h"
#include "CICalendarDateTime.h"
#include "CITIPDefinitions.h"

#include "cdsharedptr.h"

#include <vector>


namespace iCal {

class CICalendarComponentExpanded;
typedef cdsharedptr<CICalendarComponentExpanded> CICalendarComponentExpandedShared;

class CICalendarVToDo: public CICalendarComponentRecur
{
public:
	enum ECompletedStatus
	{
		eOverdue = 0,
		eDueNow,
		eDueLater,
		eDone,
		eCancelled
	};

	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVToDo(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	// std::sort comparitors
	static bool sort_for_display(const CICalendarComponentExpandedShared& s1, const CICalendarComponentExpandedShared& s2);

	CICalendarVToDo(const CICalendarRef& calendar) :
		CICalendarComponentRecur(calendar)
		{ _init_CICalendarVToDo(); }
	CICalendarVToDo(const CICalendarVToDo& copy) :
		CICalendarComponentRecur(copy)
		{ _copy_CICalendarVToDo(copy); }
	virtual ~CICalendarVToDo() {}

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVToDo(*this);
	}

	CICalendarVToDo& operator=(const CICalendarVToDo& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVToDo(copy);
				CICalendarComponentRecur::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVTODO; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }
	virtual cdstring GetMimeComponentName() const
	{
		return cICalMIMEComponent_VTODO;
	}

	virtual bool AddComponent(CICalendarComponent* comp);

	EStatus_VToDo	GetStatus() const
		{ return mStatus; }
	void	SetStatus(EStatus_VToDo status)
		{ mStatus = status; }
	cdstring GetStatusText() const;

	ECompletedStatus GetCompletionState() const;

	int32_t	GetPriority() const
		{ return mPriority; }
	void	SetPriority(int32_t priority)
		{ mPriority = priority; }

	const CICalendarDateTime& GetCompleted() const
		{ return mCompleted; }
	bool HasCompleted() const
		{ return mHasCompleted; }

	virtual void Finalise();

	// Editing
			void EditStatus(EStatus_VToDo status);
			void EditCompleted(const CICalendarDateTime& completed);

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;

	int32_t					mPriority;
	EStatus_VToDo			mStatus;
	int32_t					mPercentComplete;
	CICalendarDateTime		mCompleted;
	bool					mHasCompleted;

private:
	void	_init_CICalendarVToDo();
	void	_copy_CICalendarVToDo(const CICalendarVToDo& copy);
};

typedef std::vector<CICalendarVToDo*> CICalendarVToDoList;

}	// namespace iCal

#endif	// CICalendarVToDo_H
