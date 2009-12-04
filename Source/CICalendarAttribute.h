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
	CICalendarAttribute.h

	Author:
	Description:	<describe the CICalendarAttribute class here>
*/

#ifndef CICalendarAttribute_H
#define CICalendarAttribute_H

#include <map>
#include <vector>

#include "cdstring.h"


namespace iCal {

class CICalendarAttribute
{
public:
	CICalendarAttribute() {}
	CICalendarAttribute(const cdstring& name, const cdstring& value)
		{ mName = name; mValues.push_back(value); }
	CICalendarAttribute(const CICalendarAttribute& copy)
		{ _copy(copy); }
	virtual ~CICalendarAttribute() {}

	CICalendarAttribute& operator=(const CICalendarAttribute& copy)
		{ if (this != &copy) _copy(copy); return *this; }

	cdstring& GetName()
		{ return mName; }
	const cdstring& GetName() const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }

	const cdstring& GetFirstValue() const
		{ return mValues.front(); }
	cdstrvect& GetValues()
		{ return mValues; }
	const cdstrvect& GetValues() const
		{ return mValues; }
	void AddValue(const cdstring& value)
		{ mValues.push_back(value); }

	void Generate(std::ostream& os) const;

protected:
	cdstring		mName;
	cdstrvect		mValues;

private:
	void _copy(const CICalendarAttribute& copy)
		{ mName = copy.mName; mValues = copy.mValues; }
};

typedef std::multimap<cdstring, CICalendarAttribute> CICalendarAttributeMap;

}	// namespace iCal

#endif	// CICalendarAttribute_H
