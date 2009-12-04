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
	CICalendarComponentBase.cpp

	Author:
	Description:	<describe the CICalendarComponentBase class here>
*/

#include "CICalendarComponentBase.h"

#include "CICalendarDateTimeValue.h"
#include "CICalendarDurationValue.h"
#include "CICalendarIntegerValue.h"
#include "CICalendarMultiValue.h"
#include "CICalendarPeriodValue.h"
#include "CICalendarRecurrenceSet.h"
#include "CICalendarRecurrenceValue.h"
#include "CICalendarPlainTextValue.h"
#include "CICalendarUTCOffsetValue.h"

using namespace iCal;

// Integers can be read from varios types of value
bool CICalendarComponentBase::LoadValue(const char* value_name, int32_t& value, CICalendarValue::EICalValueType type) const
{
	if (GetProperties().count(value_name))
	{
		switch(type)
		{
		case CICalendarValue::eValueType_Integer:
		{
			const CICalendarIntegerValue* ivalue = (*GetProperties().find(value_name)).second.GetIntegerValue();
			if (ivalue != NULL)
			{
				value = ivalue->GetValue();
				return true;
			}
			break;
		}
		case CICalendarValue::eValueType_UTC_Offset:
		{
			const CICalendarUTCOffsetValue* uvalue = (*GetProperties().find(value_name)).second.GetUTCOffsetValue();
			if (uvalue != NULL)
			{
				value = uvalue->GetValue();
				return true;
			}
			break;
		}
		default:;
		}
	}

	return false;
}

bool CICalendarComponentBase::LoadValue(const char* value_name, cdstring& value) const
{
	if (GetProperties().count(value_name))
	{
		const CICalendarPlainTextValue* tvalue = (*GetProperties().find(value_name)).second.GetTextValue();
		if (tvalue != NULL)
		{
			value = tvalue->GetValue();
			return true;
		}
	}

	return false;
}

bool CICalendarComponentBase::LoadValue(const char* value_name, CICalendarDateTime& value) const
{
	if (GetProperties().count(value_name))
	{
		const CICalendarDateTimeValue* dtvalue = (*GetProperties().find(value_name)).second.GetDateTimeValue();
		if (dtvalue != NULL)
		{
			value = dtvalue->GetValue();
			return true;
		}
	}

	return false;
}

bool CICalendarComponentBase::LoadValue(const char* value_name, CICalendarDuration& value) const
{
	if (GetProperties().count(value_name))
	{
		const CICalendarDurationValue* dvalue = (*GetProperties().find(value_name)).second.GetDurationValue();
		if (dvalue != NULL)
		{
			value = dvalue->GetValue();
			return true;
		}
	}

	return false;
}

bool CICalendarComponentBase::LoadValue(const char* value_name, CICalendarPeriod& value) const
{
	if (GetProperties().count(value_name))
	{
		const CICalendarPeriodValue* pvalue = (*GetProperties().find(value_name)).second.GetPeriodValue();
		if (pvalue != NULL)
		{
			value = pvalue->GetValue();
			return true;
		}
	}

	return false;
}

bool CICalendarComponentBase::LoadValueRRULE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const
{
	// Get RRULEs
	if (GetProperties().count(value_name))
	{
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> result = GetProperties().equal_range(value_name);
		for(CICalendarPropertyMap::const_iterator iter = result.first; iter != result.second; iter++)
		{
			const CICalendarRecurrenceValue* rvalue = (*iter).second.GetRecurrenceValue();
			if (rvalue != NULL)
			{
				if (add)
					value.Add(rvalue->GetValue());
				else
					value.Subtract(rvalue->GetValue());
			}
		}
		return true;
	}
	else
		return false;
}

bool CICalendarComponentBase::LoadValueRDATE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const
{
	// Get RDATEs
	if (GetProperties().count(value_name))
	{
		std::pair<CICalendarPropertyMap::const_iterator, CICalendarPropertyMap::const_iterator> result = GetProperties().equal_range(value_name);
		for(CICalendarPropertyMap::const_iterator iter = result.first; iter != result.second; iter++)
		{
			const CICalendarMultiValue* mvalue = (*iter).second.GetMultiValue();
			if (mvalue != NULL)
			{
				for(CICalendarValueList::const_iterator iter = mvalue->GetValues().begin(); iter != mvalue->GetValues().end(); iter++)
				{
					// cast to date-time
					CICalendarDateTimeValue* dtv = dynamic_cast<CICalendarDateTimeValue*>(*iter);
					CICalendarPeriodValue* pv = dynamic_cast<CICalendarPeriodValue*>(*iter);
					if (dtv != NULL)
					{
						if (add)
							value.Add(dtv->GetValue());
						else
							value.Subtract(dtv->GetValue());
					}
					else if (pv != NULL)
					{
						if (add)
							value.Add(pv->GetValue().GetStart());
						else
							value.Subtract(pv->GetValue().GetStart());
					}
				}
			}
		}
		return true;
	}
	else
		return false;
}

void CICalendarComponentBase::WriteProperties(std::ostream& os) const
{
	for(CICalendarPropertyMap::const_iterator iter = mProperties.begin(); iter != mProperties.end(); iter++)
		(*iter).second.Generate(os);
}

bool CICalendarComponentBase::LoadPrivateValue(const char* value_name, cdstring& value)
{
	// Read it in from properties list and then delete the property from the main list
	bool result = LoadValue(value_name, value);
	if (result)
		RemoveProperties(value_name);
	return result;
}

void CICalendarComponentBase::WritePrivateProperty(std::ostream& os, const cdstring& key, const cdstring& value) const
{
	CICalendarProperty prop(key, value);
	prop.Generate(os);
}

CICalendarPropertyMap& CICalendarComponentBase::GetProperties()
{
	return mProperties;
}
const CICalendarPropertyMap& CICalendarComponentBase::GetProperties() const
{
	return mProperties;
}
void CICalendarComponentBase::SetProperties(const CICalendarPropertyMap& props)
{
	 mProperties = props;
}

void CICalendarComponentBase::AddProperty(const CICalendarProperty& prop)
{
	CICalendarPropertyMap::iterator result = mProperties.insert(CICalendarPropertyMap::value_type(prop.GetName(), prop));
}

bool CICalendarComponentBase::HasProperty(const cdstring& prop) const
{
	return mProperties.count(prop) > 0;
}

uint32_t CICalendarComponentBase::CountProperty(const cdstring& prop) const
{
	return mProperties.count(prop);
}

void CICalendarComponentBase::RemoveProperties(const cdstring& prop)
{
	mProperties.erase(prop);
}
