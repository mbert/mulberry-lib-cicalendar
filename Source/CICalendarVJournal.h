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
	CICalendarVJournal.h

	Author:
	Description:	<describe the CICalendarVJournal class here>
*/

#ifndef CICalendarVJournal_H
#define CICalendarVJournal_H

#include "CICalendarComponentRecur.h"

#include "CICalendarDateTime.h"
#include "CITIPDefinitions.h"

namespace iCal {

class CICalendarVJournal: public CICalendarComponentRecur
{
public:
	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVJournal(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	CICalendarVJournal(const CICalendarRef& calendar) :
		CICalendarComponentRecur(calendar) {}
	CICalendarVJournal(const CICalendarVJournal& copy) :
		CICalendarComponentRecur(copy)
		{ _copy_CICalendarVJournal(copy); }
	virtual ~CICalendarVJournal() {}

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVJournal(*this);
	}

	CICalendarVJournal& operator=(const CICalendarVJournal& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVJournal(copy);
				CICalendarComponentRecur::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVJOURNAL; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }
	virtual cdstring GetMimeComponentName() const
	{
		return cICalMIMEComponent_VJOURNAL;
	}

	virtual void Finalise();

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;

private:
	void	_copy_CICalendarVJournal(const CICalendarVJournal& copy)
		{ }
};

}	// namespace iCal

#endif	// CICalendarVJournal_H
