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
	CICalendarSync.h

	Author:
	Description:	Synchronise two iCalendars
*/

#ifndef CICalendarSync_H
#define CICalendarSync_H

#include "CICalendar.h"

#include "cdstring.h"

namespace iCal {

class CICalendarSync
{
	class CICalendarSyncData
	{
	public:
		CICalendarSyncData(const cdstring& mapkey, uint32_t seq, uint32_t orig = 0) :
			mMapKey(mapkey), mSeq(seq), mOriginalSeq(orig) {}
		CICalendarSyncData(const CICalendarSyncData& copy)
		{
			_copy_CICalendarSyncData(copy);
		}
		~CICalendarSyncData() {}
		
		CICalendarSyncData& operator=(const CICalendarSyncData& copy)
		{
			if (this != &copy) _copy_CICalendarSyncData(copy); return *this;
		}
		
		int operator==(const CICalendarSyncData& other) const
		{
			return mMapKey == other.mMapKey;
		}
	
		int operator<(const CICalendarSyncData& other) const
		{
			return mMapKey < other.mMapKey;
		}
	
		const cdstring& GetMapKey() const
		{
			return mMapKey;
		}
		
		uint32_t GetSeq() const
		{
			return mSeq;
		}
		
		uint32_t GetOriginalSeq() const
		{
			return mOriginalSeq;
		}

	private:
		cdstring mMapKey;
		uint32_t mSeq;
		uint32_t mOriginalSeq;
		
		void _copy_CICalendarSyncData(const CICalendarSyncData& copy)
		{
			mMapKey = copy.mMapKey; mSeq = copy.mSeq; mOriginalSeq = copy.mOriginalSeq;
		}
	};
	typedef std::vector<CICalendarSyncData> CICalendarSyncDataList;

public:
	CICalendarSync(CICalendar& src1, const CICalendar& src2)
		: mCal1(src1), mCal2(src2) {}
	~CICalendarSync() {}

	void Sync();

	static int CompareComponentVersions(const CICalendarComponent* comp1, const CICalendarComponent* comp2);

protected:
	CICalendar&				mCal1;
	const CICalendar&		mCal2;

	void GetAllKeys(const CICalendar& cal, CICalendarSyncDataList& keys);
	void GetKeys(const CICalendarComponentDB& db, CICalendarSyncDataList& keys);

	void RemoveKeys(CICalendarSyncDataList& keys, const CICalendarComponentRecordDB& recorded, unsigned long filter);
};

}	// namespace iCal

#endif	// CICalendarSync_H
