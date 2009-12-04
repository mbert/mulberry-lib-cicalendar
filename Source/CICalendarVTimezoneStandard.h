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
	CICalendarVTimezoneStandard.h

	Author:
	Description:	<describe the CICalendarVTimezoneStandard class here>
*/

#ifndef CICalendarVTimezoneStandard_H
#define CICalendarVTimezoneStandard_H

#include "CICalendarVTimezoneElement.h"

namespace iCal {

class CICalendarVTimezoneStandard: public CICalendarVTimezoneElement
{
public:
	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVTimezoneStandard(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	CICalendarVTimezoneStandard(const CICalendarRef& calendar) :
		CICalendarVTimezoneElement(calendar) {}
	CICalendarVTimezoneStandard(const CICalendarVTimezoneStandard& copy) :
		CICalendarVTimezoneElement(copy) {}
	virtual ~CICalendarVTimezoneStandard() {}

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVTimezoneStandard(*this);
	}

	CICalendarVTimezoneStandard& operator=(const CICalendarVTimezoneStandard& copy)
		{
			if (this != &copy)
			{
				CICalendarVTimezoneElement::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVTIMEZONESTANDARD; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;
};

}	// namespace iCal

#endif	// CICalendarVTimezoneStandard_H
