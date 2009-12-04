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
	CICalendarVJournal.cpp

	Author:
	Description:	<describe the CICalendarVJournal class here>
*/

#include "CICalendarVJournal.h"

#include "CICalendarDefinitions.h"

using namespace iCal;

cdstring CICalendarVJournal::sBeginDelimiter(cICalComponent_BEGINVJOURNAL);
cdstring CICalendarVJournal::sEndDelimiter(cICalComponent_ENDVJOURNAL);

void CICalendarVJournal::Finalise()
{
	// Do inherited
	CICalendarComponentRecur::Finalise();
}
