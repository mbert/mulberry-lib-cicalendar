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
	CICalendarVTimezoneDaylight.h

	Author:
	Description:	<describe the CICalendarVTimezoneDaylight class here>
*/

#ifndef CICalendarVTimezoneDaylight_H
#define CICalendarVTimezoneDaylight_H

#include "CICalendarVTimezoneElement.h"

namespace iCal {

class CICalendarVTimezoneDaylight: public CICalendarVTimezoneElement
{
public:
	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVTimezoneDaylight(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	CICalendarVTimezoneDaylight(const CICalendarRef& calendar) :
		CICalendarVTimezoneElement(calendar) {}
	CICalendarVTimezoneDaylight(const CICalendarVTimezoneDaylight& copy) :
		CICalendarVTimezoneElement(copy) {}
	virtual ~CICalendarVTimezoneDaylight() {}

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVTimezoneDaylight(*this);
	}

	CICalendarVTimezoneDaylight& operator=(const CICalendarVTimezoneDaylight& copy)
		{
			if (this != &copy)
			{
				CICalendarVTimezoneElement::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVTIMEZONEDAYLIGHT; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;
};

}	// namespace iCal

#endif	// CICalendarVTimezoneDaylight_H
