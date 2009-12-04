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
	CICalendarPlainTextValue.h

	Author:
	Description:	<describe the CICalendarPlainTextValue class here>
*/

#ifndef CICalendarPlainTextValue_H
#define CICalendarPlainTextValue_H

#include "CICalendarValue.h"


namespace iCal {

class CICalendarPlainTextValue : public CICalendarValue
{
public:
	CICalendarPlainTextValue() {}
	CICalendarPlainTextValue(const cdstring& value)
		{ mValue = value; }
	CICalendarPlainTextValue(const CICalendarPlainTextValue& copy)
		 : CICalendarValue(copy)
		{ _copy_CICalendarPlainTextValue(copy); }
	virtual ~CICalendarPlainTextValue() {}

	CICalendarPlainTextValue& operator=(const CICalendarPlainTextValue& copy)
		{ if (this != &copy) _copy_CICalendarPlainTextValue(copy); return *this; }

	virtual void Parse(const cdstring& data);
	virtual void Generate(std::ostream& os) const;

	cdstring& GetValue()
		{ return mValue; }
	const cdstring& GetValue() const
		{ return mValue; }
	void SetValue(const cdstring& value)
		{ mValue = value; }

protected:
	cdstring mValue;

private:
	void _copy_CICalendarPlainTextValue(const CICalendarPlainTextValue& copy)
		{ mValue = copy.mValue; }
};

}	// namespace iCal

#endif	// CICalendarPlainTextValue_H
