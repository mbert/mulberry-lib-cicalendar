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
	CICalendarURIValue.h

	Author:
	Description:	<describe the CICalendarURIValue class here>
*/

#ifndef CICalendarURIValue_H
#define CICalendarURIValue_H

#include "CICalendarPlainTextValue.h"


namespace iCal {

class CICalendarURIValue : public CICalendarPlainTextValue
{
public:
	CICalendarURIValue() {}
	CICalendarURIValue(const cdstring& value) :
		CICalendarPlainTextValue(value) {}
	CICalendarURIValue(const CICalendarURIValue& copy)
		 : CICalendarPlainTextValue(copy) {}
	virtual ~CICalendarURIValue() {}

	CICalendarURIValue& operator=(const CICalendarURIValue& copy)
		{ if (this != &copy) CICalendarPlainTextValue::operator=(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarURIValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_URI; }

};

}	// namespace iCal

#endif	// CICalendarURIValue_H
