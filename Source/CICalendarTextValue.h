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
	CICalendarTextValue.h

	Author:
	Description:	<describe the CICalendarTextValue class here>
*/

#ifndef CICalendarTextValue_H
#define CICalendarTextValue_H

#include "CICalendarPlainTextValue.h"


namespace iCal {

class CICalendarTextValue : public CICalendarPlainTextValue
{
public:
	CICalendarTextValue() {}
	CICalendarTextValue(const cdstring& value) :
		CICalendarPlainTextValue(value) {}
	CICalendarTextValue(const CICalendarTextValue& copy)
		 : CICalendarPlainTextValue(copy) {}
	virtual ~CICalendarTextValue() {}

	CICalendarTextValue& operator=(const CICalendarTextValue& copy)
		{ if (this != &copy) CICalendarPlainTextValue::operator=(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarTextValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_Text; }

	virtual void Parse(const cdstring& data);
	virtual void Generate(std::ostream& os) const;

};

}	// namespace iCal

#endif	// CICalendarValue_H
