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
	CICalendarComponentBase.h

	Author:
	Description:	<describe the CICalendarComponentBase class here>
*/

#ifndef CICalendarComponentBase_H
#define CICalendarComponentBase_H

#include "CICalendarProperty.h"

#include <stdint.h>
#include <iostream>
#include <map>

#include "cdstring.h"


namespace iCal {

class CICalendarDateTime;
class CICalendarDuration;
class CICalendarPeriod;
class CICalendarRecurrenceSet;

class CICalendarComponentBase
{
public:
	CICalendarComponentBase() {}
	CICalendarComponentBase(const CICalendarComponentBase& copy)
		{ _copy_CICalendarComponentBase(copy); }
	virtual ~CICalendarComponentBase() {}

	CICalendarComponentBase& operator=(const CICalendarComponentBase& copy)
		{ if (this != &copy) _copy_CICalendarComponentBase(copy); return *this; }

	CICalendarPropertyMap& GetProperties();
	const CICalendarPropertyMap& GetProperties() const;
	void SetProperties(const CICalendarPropertyMap& props);

	void AddProperty(const CICalendarProperty& prop);
	bool HasProperty(const cdstring& prop) const;
	uint32_t CountProperty(const cdstring& prop) const;
	void RemoveProperties(const cdstring& prop);

	bool GetProperty(const cdstring& prop, int32_t& value, CICalendarValue::EICalValueType type = CICalendarValue::eValueType_Integer) const
	{
		return LoadValue(prop.c_str(), value, type);
	}
	bool GetProperty(const cdstring& prop, cdstring& value) const
	{
		return LoadValue(prop.c_str(), value);
	}
	bool GetProperty(const cdstring& prop, CICalendarDateTime& value) const
	{
		return LoadValue(prop.c_str(), value);
	}

	virtual void Finalise() = 0;

	virtual void Generate(std::ostream& os, bool for_cache = false) const = 0;

#ifdef _GCC_2_95
public:
#else
protected:
#endif
	CICalendarPropertyMap		mProperties;

	virtual bool	LoadValue(const char* value_name, int32_t& value, CICalendarValue::EICalValueType type = CICalendarValue::eValueType_Integer) const;
	virtual bool	LoadValue(const char* value_name, cdstring& value) const;
	virtual bool	LoadValue(const char* value_name, CICalendarDateTime& value) const;
	virtual bool	LoadValue(const char* value_name, CICalendarDuration& value) const;
	virtual bool	LoadValue(const char* value_name, CICalendarPeriod& value) const;
	virtual bool	LoadValueRRULE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const;
	virtual bool	LoadValueRDATE(const char* value_name, CICalendarRecurrenceSet& value, bool add) const;
	
	void	WriteProperties(std::ostream& os) const;

	bool	LoadPrivateValue(const char* value_name, cdstring& value);
	void	WritePrivateProperty(std::ostream& os, const cdstring& key, const cdstring& value) const;

private:
	void	_copy_CICalendarComponentBase(const CICalendarComponentBase& copy)
		{ mProperties = copy.mProperties; }
};

}	// namespace iCal

#endif	// CICalendarComponentBase_H
