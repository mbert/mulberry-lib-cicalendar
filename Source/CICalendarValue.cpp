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
	CICalendarValue.cpp

	Author:
	Description:	<describe the CICalendarValue class here>
*/

#include "CICalendarValue.h"

#include "CICalendarCalAddressValue.h"
#include "CICalendarDateTimeValue.h"
#include "CICalendarDummyValue.h"
#include "CICalendarDurationValue.h"
#include "CICalendarIntegerValue.h"
#include "CICalendarPeriodValue.h"
#include "CICalendarRecurrenceValue.h"
#include "CICalendarTextValue.h"
#include "CICalendarURIValue.h"
#include "CICalendarUTCOffsetValue.h"

using namespace iCal;

CICalendarValue* CICalendarValue::CreateFromType(EICalValueType type)
{
	// Create the type
	switch(type)
	{
	case eValueType_Text:
		return new CICalendarTextValue;
	case eValueType_CalAddress:
		return new CICalendarCalAddressValue;
	case eValueType_URI:
		return new CICalendarURIValue;
	case eValueType_Integer:
		return new CICalendarIntegerValue;
	case eValueType_Date:
	case eValueType_DateTime:
		return new CICalendarDateTimeValue;
	case eValueType_Duration:
		return new CICalendarDurationValue;
	case eValueType_Period:
		return new CICalendarPeriodValue;
	case eValueType_Recur:
		return new CICalendarRecurrenceValue;
	case eValueType_UTC_Offset:
		return new CICalendarUTCOffsetValue;
	default:
		return new CICalendarDummyValue(type);
	}
}
