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
	CICalendarComponentRecord.h

	Author:
	Description:	records changes in components in a cached calendar
*/

#ifndef CICalendarComponentRecord_H
#define CICalendarComponentRecord_H

#include <map>
#include <vector>

#include "cdstring.h"


namespace xmllib 
{
class XMLDocument;
class XMLNode;
};

namespace iCal {

class CICalendarComponentRecord;
typedef std::map<cdstring, CICalendarComponentRecord> CICalendarComponentRecordDB;

typedef uint32_t	CICalendarRef;	// Unique reference to object
class CICalendar;

class CICalendarComponent;

class CICalendarComponentRecord
{
public:
	enum ERecordAction
	{
		eAdded	 		= 1L << 0,
		eChanged		= 1L << 1,
		eRemoved		= 1L << 2,	// Removed a component not previously recorded as being added
		eRemovedAdded	= 1L << 3	// Removed a component previously recorded as being new
	};

	static void RecordAction(CICalendarComponentRecordDB& recorder, const CICalendarComponent* item, ERecordAction action);

	CICalendarComponentRecord(ERecordAction action, const cdstring& uid, uint32_t seq, const cdstring& rid, const cdstring& rurl, const cdstring& etag)
	{
		mAction = action;
		mUID = uid;
		mSeq = seq;
		mRID = rid;
		mRURL = rurl;
		mETag = etag;
	}
	CICalendarComponentRecord(const CICalendarComponent* item, ERecordAction action);
	CICalendarComponentRecord(const CICalendarComponentRecord& copy)
	{
		_copy_CICalendarComponentRecord(copy);
	}
	virtual ~CICalendarComponentRecord();

	CICalendarComponentRecord& operator=(const CICalendarComponentRecord& copy)
	{
		if (this != &copy)
		{
			_copy_CICalendarComponentRecord(copy);
		}
		return *this;
	}
	
	ERecordAction GetAction() const
	{
		return mAction;
	}
	void SetAction(ERecordAction action)
	{
		mAction = action;
	}

	const cdstring& GetUID() const
	{
		return mUID;
	}
	uint32_t		GetSeq() const
	{
		return mSeq;
	}
	const cdstring& GetRecurrenceID() const
	{
		return mRID;
	}
	const cdstring& GetRURL() const
	{
		return mRURL;
	}
	const cdstring& GetETag() const
	{
		return mETag;
	}

	void WriteXML(xmllib::XMLDocument* doc, xmllib::XMLNode* parent, const cdstring& mapkey) const;
	static void ReadXML(CICalendarComponentRecordDB& recorder, const xmllib::XMLNode* node);

protected:
	ERecordAction				mAction;
	cdstring					mUID;
	int32_t						mSeq;
	cdstring					mRID;
	cdstring					mRURL;
	cdstring					mETag;

private:
	void	_copy_CICalendarComponentRecord(const CICalendarComponentRecord& copy)
	{
		mAction = copy.mAction; mUID = copy.mUID; mSeq = copy.mSeq; mRID = copy.mRID; mRURL = copy.mRURL; mETag = copy.mETag;
	}
};

}	// namespace iCal

#endif	// CICalendarComponentRecord_H
