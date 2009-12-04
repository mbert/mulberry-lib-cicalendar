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
	CICalendarComponent.h

	Author:
	Description:	<describe the CICalendarComponent class here>
*/

#ifndef CICalendarComponent_H
#define CICalendarComponent_H

#include "CICalendarComponentBase.h"

#include "CICalendarValue.h"

#include <stdint.h>
#include <iostream>
#include <vector>

#include "cdstring.h"
#include "cdsharedptr.h"


namespace iCal {

class CICalendarComponent;
typedef std::vector<CICalendarComponent*> CICalendarComponentList;

typedef uint32_t	CICalendarRef;	// Unique reference to object
class CICalendar;

class CICalendarComponent : public CICalendarComponentBase
{
public:
	enum EComponentType
	{
		eVEVENT,
		eVTODO,
		eVJOURNAL,
		eVFREEBUSY,
		eVTIMEZONE,
		eVALARM,

		// Pseudo components
		eVTIMEZONESTANDARD,
		eVTIMEZONEDAYLIGHT
	};

	typedef CICalendarComponent* (*CreateComponentPP)(const CICalendarRef& calendar);

	CICalendarComponent(const CICalendarRef& calendar)
		{ mCalendarRef = calendar; mSeq = 0; mOriginalSeq = 0; mEmbedder = NULL; mEmbedded = NULL; mChanged = false; }
	CICalendarComponent(const CICalendarComponent& copy) :
		CICalendarComponentBase(copy)
		{ mEmbedder = NULL; mEmbedded = NULL; mChanged = false; _copy_CICalendarComponent(copy); }
	virtual ~CICalendarComponent();

	CICalendarComponent& operator=(const CICalendarComponent& copy)
		{
			if (this != &copy)
			{
				_copy_CICalendarComponent(copy);
				CICalendarComponentBase::operator=(copy);
			}
			return *this;
		}

	virtual CICalendarComponent* clone() const = 0;

	virtual EComponentType GetType() const = 0;
	virtual const cdstring& GetBeginDelimiter() const = 0;
	virtual const cdstring& GetEndDelimiter() const = 0;
	virtual cdstring GetMimeComponentName() const = 0;

	virtual bool AddComponent(CICalendarComponent* comp);
			void RemoveComponent(CICalendarComponent* comp);
			bool HasEmbeddedComponent(EComponentType type) const;
			CICalendarComponent* GetFirstEmbeddedComponent(EComponentType type) const;
			void SetEmbedder(CICalendarComponent* embedder)
			{
				mEmbedder = embedder;
			}
			const CICalendarComponent* GetEmbedder() const
			{
				return mEmbedder;
			}

	void SetCalendar(const CICalendarRef& ref)
		{ mCalendarRef = ref; }
	const CICalendarRef& GetCalendar() const
		{ return mCalendarRef; }

	virtual const cdstring& GetMapKey() const
		{ return mUID; }
	virtual cdstring GetMasterKey() const
		{ return mUID; }

	cdstring& GetUID()
	{
		return mUID;
	}
	const cdstring& GetUID() const
	{
		return mUID;
	}
	virtual void SetUID(const cdstring& uid);

	int32_t& GetSeq()
	{
		return mSeq;
	}
	const int32_t& GetSeq() const
	{
		return mSeq;
	}
	void SetSeq(const int32_t& seq);

	const int32_t& GetOriginalSeq() const
	{
		return mOriginalSeq;
	}

	const cdstring& GetRURL() const
	{
		return mRURL;
	}
	void SetRURL(const cdstring& rurl)
	{
		mRURL = rurl;
	}
	void GenerateRURL();

	const cdstring& GetETag() const
	{
		return mETag;
	}
	void SetETag(const cdstring& etag)
	{
		mETag = etag;
	}

	bool GetChanged() const
	{
		return mChanged;
	}
	void SetChanged(bool changed)
	{
		mChanged = changed;
	}

	virtual void InitDTSTAMP();
	virtual void UpdateLastModified();

	virtual void Added();
	virtual void Removed();
	virtual void Duplicated();
	virtual void Changed();

	virtual void Finalise();

	virtual bool CanGenerateInstance() const
	{
		return true;
	}
	virtual void Generate(std::ostream& os, bool for_cache = false) const;

	virtual void GetTimezones(cdstrset& tzids) const;

protected:
	CICalendarRef				mCalendarRef;
	cdstring					mUID;
	int32_t						mSeq;
	int32_t						mOriginalSeq;
	CICalendarComponent*		mEmbedder;
	CICalendarComponentList*	mEmbedded;
	
	// CalDAV stuff
	cdstring					mRURL;
	cdstring					mETag;
	bool						mChanged;

private:
	void	_copy_CICalendarComponent(const CICalendarComponent& copy);
};

}	// namespace iCal

#endif	// CICalendarComponent_H
