/*
    Copyright (c) 2007-2010 Cyrus Daboo. All rights reserved.
    
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
	CICalendarManager.cpp

	Author:
	Description:	<describe the CICalendarManager class here>
*/

#include "CICalendarManager.h"

#include "CICalendar.h"
#if defined(__MULBERRY) || defined(__MULBERRY_CONFIGURE)
#include "CConnectionManager.h"
#include "CLocalCommon.h"
#include "CPluginManager.h"
#else
#include "CLocalUtils.h"
#endif

#include "diriterator.h"
#include "cdfstream.h"

using namespace iCal;

CICalendarManager* CICalendarManager::sICalendarManager = NULL;

CICalendarManager::CICalendarManager()
{
	sICalendarManager = this;
}

CICalendarManager::~CICalendarManager()
{
	sICalendarManager = NULL;
}

const char* cTimezonesDir = "Timezones";

void CICalendarManager::InitManager()
{
#ifdef __MULBERRY
	// Need to have timezones cached before starting any UI work as timezone popup needs them
	for(cdstrvect::const_iterator iter = CPluginManager::sPluginManager.GetPluginDirs().begin(); iter != CPluginManager::sPluginManager.GetPluginDirs().end(); iter++)
	{
		cdstring tzpath = *iter;
		::addtopath(tzpath, cTimezonesDir);
		ScanDirectoryForTimezones(tzpath);
	}
	
	// Need to have timezones cached before starting any UI work as timezone popup needs them
	{
		cdstring tzpath = CConnectionManager::sConnectionManager.GetTimezonesDirectory();
		ScanDirectoryForTimezones(tzpath);
	}
	
	// Eventually we need to read these from prefs - for now they are hard-coded to my personal prefs!
	
	SetDefaultTimezone(CICalendarTimezone(false, "US/Eastern"));
#endif
}

void CICalendarManager::ScanDirectoryForTimezones(const cdstring& dir)
{
	diriterator iter(dir, true, ".ics");
	iter.set_return_hidden_files(false);
	const char* fname = NULL;
	while(iter.next(&fname))
	{
		// Get full path
		cdstring fpath = dir;
		::addtopath(fpath, fname);
		
		// Check for directory
		if (iter.is_dir())
		{
			// Scan more
			ScanDirectoryForTimezones(fpath);
		}
		else
		{
			cdifstream fin(fpath.c_str());
			iCal::CICalendar::sICalendar.Parse(fin);
		}
	}
}

void CICalendarManager::SetDefaultTimezoneID(const cdstring& tzid)
{
	// Check for UTC
	if (tzid == "UTC")
	{
		CICalendarTimezone temp(true);
		SetDefaultTimezone(temp);
	}
	else
	{
		CICalendarTimezone temp(false, tzid);
		SetDefaultTimezone(temp);
	}
}

cdstring CICalendarManager::GetDefaultTimezoneID() const
{
	if (mDefaultTimezone.GetUTC())
		return "UTC";
	else
		return mDefaultTimezone.GetTimezoneID();
}
