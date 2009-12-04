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
	CICalendarValue.h

	Author:
	Description:	<describe the CICalendarValue class here>
*/

#ifndef CICalendarValue_H
#define CICalendarValue_H

#include <iostream>
#include <vector>

#include "cdstring.h"


namespace iCal {

class CICalendarValue
{
public:
	enum EICalValueType
	{
		eValueType_Binary,
		eValueType_Boolean,
		eValueType_CalAddress,
		eValueType_Date,
		eValueType_DateTime,
		eValueType_Duration,
		eValueType_Float,
		eValueType_Geo,
		eValueType_Integer,
		eValueType_Period,
		eValueType_Recur,
		eValueType_Text,
		eValueType_Time,
		eValueType_URI,
		eValueType_UTC_Offset,
		eValueType_MultiValue,
		eValueType_XName
	};

	static CICalendarValue* CreateFromType(EICalValueType type);

	CICalendarValue() {}
	virtual ~CICalendarValue() {}

	virtual CICalendarValue* clone() = 0;

	virtual EICalValueType GetType() const = 0;

	virtual void Parse(const cdstring& data) = 0;
	virtual void Generate(std::ostream& os) const = 0;
};

typedef std::vector<CICalendarValue*> CICalendarValueList;

}	// namespace iCal

#endif	// CICalendarValue_H
