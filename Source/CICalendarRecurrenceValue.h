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
	CICalendarRecurrenceValue.h

	Author:
	Description:	<describe the CICalendarRecurrenceValue class here>
*/

#ifndef CICalendarRecurrenceValue_H
#define CICalendarRecurrenceValue_H

#include "CICalendarValue.h"

#include "CICalendarRecurrence.h"


namespace iCal {

class CICalendarRecurrenceValue : public CICalendarValue
{
public:
	CICalendarRecurrenceValue() {}
	CICalendarRecurrenceValue(const CICalendarRecurrence& value)
		{ mValue = value; }
	CICalendarRecurrenceValue(const CICalendarRecurrenceValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarRecurrenceValue(copy); }
	virtual ~CICalendarRecurrenceValue() {}

	CICalendarRecurrenceValue& operator=(const CICalendarRecurrenceValue& copy)
		{ if (this != &copy) _copy_CICalendarRecurrenceValue(copy); return *this; }

	virtual CICalendarValue* clone()
		{ return new CICalendarRecurrenceValue(*this); }

	virtual EICalValueType GetType() const
		{ return eValueType_Recur; }

	virtual void Parse(const cdstring& data)
		{ mValue.Parse(data); }
	virtual void Generate(std::ostream& os) const
		{ mValue.Generate(os); }

	CICalendarRecurrence& GetValue()
		{ return mValue; }
	const CICalendarRecurrence& GetValue() const
		{ return mValue; }
	void SetValue(const CICalendarRecurrence& value)
		{ mValue = value; }

protected:
	CICalendarRecurrence mValue;

private:
	void _copy_CICalendarRecurrenceValue(const CICalendarRecurrenceValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarRecurrenceValue_H
