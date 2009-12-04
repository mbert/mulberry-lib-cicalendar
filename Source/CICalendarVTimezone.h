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
	CICalendarVTimezone.h

	Author:
	Description:	<describe the CICalendarVTimezone class here>
*/

#ifndef CICalendarVTimezone_H
#define CICalendarVTimezone_H

#include "CICalendarComponent.h"

namespace iCal {

class CICalendarDateTime;
class CICalendarVTimezoneElement;

class CICalendarVTimezone: public CICalendarComponent
{
public:
	static CICalendarComponent* Create(const CICalendarRef& calendar)
		{ return new CICalendarVTimezone(calendar); }
	static const cdstring& GetVBegin()
		{ return sBeginDelimiter; }
	static const cdstring& GetVEnd()
		{ return sEndDelimiter; }

	CICalendarVTimezone(const CICalendarRef& calendar) :
		CICalendarComponent(calendar)
		{ mSortKey = 1; }
	CICalendarVTimezone(const CICalendarVTimezone& copy) :
		CICalendarComponent(copy)
		{ _copy_CICalendarVTimezone(copy); }
	virtual ~CICalendarVTimezone();

	virtual CICalendarComponent* clone() const
	{
		return new CICalendarVTimezone(*this);
	}

	CICalendarVTimezone& operator=(const CICalendarVTimezone& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarVTimezone(copy);
				CICalendarComponent::operator=(copy);
			}
			return *this;
		}

	virtual EComponentType GetType() const
		{ return eVTIMEZONE; }

	virtual const cdstring& GetBeginDelimiter() const
		{ return sBeginDelimiter; }
	virtual const cdstring& GetEndDelimiter() const
		{ return sEndDelimiter; }
	virtual cdstring GetMimeComponentName() const
	{
		// Cannot be sent as a separate MIME object
		return cdstring::null_str;
	}

	virtual bool AddComponent(CICalendarComponent* comp);

	virtual const cdstring& GetMapKey() const
		{ return mID; }

	virtual void Finalise();

	const cdstring& GetID() const
		{ return mID; }

	int32_t GetSortKey() const;

	int32_t GetTimezoneOffsetSeconds(const CICalendarDateTime& dt);
	cdstring GetTimezoneDescriptor(const CICalendarDateTime& dt);
	
	void	MergeTimezone(const CICalendarVTimezone& tz);

	bool	ValidEDST() const;
	void	UpgradeEDST();

protected:
	static cdstring		sBeginDelimiter;
	static cdstring		sEndDelimiter;

	cdstring				mID;
	mutable int32_t			mSortKey;

	const CICalendarVTimezoneElement*	FindTimezoneElement(const CICalendarDateTime& dt);

	void	AddEDST(int32_t offset_std);

private:
	void	_copy_CICalendarVTimezone(const CICalendarVTimezone& copy);
};

}	// namespace iCal

#endif	// CICalendarVTimezone_H
