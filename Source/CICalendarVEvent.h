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
	CICalendarVEvent.h

	Author:
	Description:	<describe the CICalendarVEvent class here>
*/

#ifndef CICalendarVEvent_H
#define CICalendarVEvent_H

#include "CICalendarComponentRecur.h"

#include "CICalendarDefinitions.h"
#include "CITIPDefinitions.h"

namespace iCal {

class CICalendarVEvent;
typedef std::vector<CICalendarVEvent*> CICalendarVEventList;

class CICalendarVEvent: public CICalendarComponentRecur
{
public:
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	static CICalendarComponent* Create(const CICalendarRef& calendar);

	CICalendarVEvent(const CICalendarRef& calendar) :
		CICalendarComponentRecur(calendar)
		{ mStatus = eStatus_VEvent_None; }
	CICalendarVEvent(const CICalendarVEvent& copy) :
		CICalendarComponentRecur(copy)
		{ _copy_CICalendarVEvent(copy); }
	virtual ~CICalendarVEvent()
		{}

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVEvent(*this);
	}

	CICalendarVEvent& operator=(const CICalendarVEvent& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVEvent(copy);
				CICalendarComponentRecur::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVEVENT; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }
	virtual cdstring GetMimeComponentName() const
	{
		return cICalMIMEComponent_VEVENT;
	}

	virtual bool AddComponent(CICalendarComponent* comp);

	virtual EStatus_VEvent	GetStatus() const
		{ return mStatus; }
	void	SetStatus(EStatus_VEvent status)
		{ mStatus = status; }

	virtual void Finalise();

	// Editing
			void EditStatus(EStatus_VEvent status);

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;

	EStatus_VEvent		mStatus;

private:
	void	_copy_CICalendarVEvent(const CICalendarVEvent& copy)
		{ mStatus = copy.mStatus; }
};

}	// namespace iCal

#endif	// CICalendarVEvent_H
