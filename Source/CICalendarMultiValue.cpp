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
	CICalendarMultiValue.cpp

	Author:
	Description:	<describe the CICalendarMultiValue class here>
*/

#include "CICalendarMultiValue.h"

using namespace iCal;

void CICalendarMultiValue::_copy_CICalendarMultiValue(const CICalendarMultiValue& copy)
{
	_tidy_CICalendarMultiValue();

	mType = copy.mType;
	for(CICalendarValueList::const_iterator iter = copy.mValues.begin(); iter != copy.mValues.end(); iter++)
		mValues.push_back((*iter)->clone());
}

void CICalendarMultiValue::_tidy_CICalendarMultiValue()
{
	// Delete all value in current list
	for(CICalendarValueList::iterator iter = mValues.begin(); iter != mValues.end(); iter++)
		delete *iter;
	mValues.clear();
}

void CICalendarMultiValue::Parse(const cdstring& data)
{
	// Tokenize on comma
	cdstring::size_type start_pos = 0;
	cdstring::size_type data_size = data.length();
	while(true)
	{
		// Find next token separator
		cdstring token;
		cdstring::size_type end_pos = data.find(",", start_pos);
		if (end_pos == cdstring::npos)
			token.assign(data, start_pos, cdstring::npos);
		else
			token.assign(data, start_pos, end_pos - start_pos);
		
		// Create single value, and parse data
		CICalendarValue* value = CICalendarValue::CreateFromType(mType);
		value->Parse(token);
		mValues.push_back(value);
		
		// Look for next bit
		if ((end_pos == cdstring::npos) || (end_pos == data_size))
			break;
		start_pos = end_pos + 1;
	}
}

void CICalendarMultiValue::Generate(std::ostream& os) const
{
	for(CICalendarValueList::const_iterator iter = mValues.begin(); iter != mValues.end(); iter++)
	{
		if (iter != mValues.begin())
			os << ',';
		(*iter)->Generate(os);
	}
}
