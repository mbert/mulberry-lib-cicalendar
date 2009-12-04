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
	CICalendarCalAddressValue.h

	Author:
	Description:	<describe the CICalendarCalAddressValue class here>
*/

#ifndef CICalendarCalAddressValue_H
#define CICalendarCalAddressValue_H

#include "CICalendarPlainTextValue.h"


namespace iCal {

class CICalendarCalAddressValue : public CICalendarPlainTextValue
{
public:
	CICalendarCalAddressValue() {}
	CICalendarCalAddressValue(const cdstring& value) :
		CICalendarPlainTextValue(value) {}
	CICalendarCalAddressValue(const CICalendarCalAddressValue& copy)
		 : CICalendarPlainTextValue(copy) {}
	virtual ~CICalendarCalAddressValue() {}

	CICalendarCalAddressValue& operator=(const CICalendarCalAddressValue& copy)
		{ if (this != &copy) CICalendarPlainTextValue::operator=(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarCalAddressValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_CalAddress; }

};

}	// namespace iCal

#endif	// CICalendarCalAddressValue_H
