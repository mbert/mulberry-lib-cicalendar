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
	CICalendarManager.h

	Author:
	Description:	<describe the CICalendarManager class here>
*/

#ifndef CICalendarManager_H
#define CICalendarManager_H

#include "CICalendarTimezone.h"

#include "cdstring.h"


namespace iCal {

class CICalendarManager
{
public:
	static CICalendarManager* sICalendarManager;

	CICalendarManager();
	virtual ~CICalendarManager();

	void InitManager();

	void SetDefaultTimezoneID(const cdstring& tzid);
	void SetDefaultTimezone(const CICalendarTimezone& tzid)
	{
		mDefaultTimezone = tzid;
	}
	cdstring GetDefaultTimezoneID() const;
	const CICalendarTimezone& GetDefaultTimezone() const
	{
		return mDefaultTimezone;
	}
	
protected:	
	CICalendarTimezone					mDefaultTimezone;

};

}	// namespace iCal

#endif	// CICalendarManager_H
