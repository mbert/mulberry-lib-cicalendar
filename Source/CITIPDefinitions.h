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
	CITIPDefinitions.h

	Author:
	Description:	constants from iTIP/iMIP documents
*/

#ifndef CITIPDefinitions_H
#define CITIPDefinitions_H

namespace iCal {

// 2446 3

extern const char* cICalMethod_PUBLISH;
extern const char* cICalMethod_REQUEST;
extern const char* cICalMethod_REFRESH;
extern const char* cICalMethod_CANCEL;
extern const char* cICalMethod_ADD;
extern const char* cICalMethod_REPLY;
extern const char* cICalMethod_COUNTER;
extern const char* cICalMethod_DECLINECOUNTER;

// 2447 2.4

extern const char* cICalMIMEMethod_PUBLISH;
extern const char* cICalMIMEMethod_REQUEST;
extern const char* cICalMIMEMethod_REFRESH;
extern const char* cICalMIMEMethod_CANCEL;
extern const char* cICalMIMEMethod_ADD;
extern const char* cICalMIMEMethod_REPLY;
extern const char* cICalMIMEMethod_COUNTER;
extern const char* cICalMIMEMethod_DECLINECOUNTER;

extern const char* cICalMIMEComponent_VEVENT;
extern const char* cICalMIMEComponent_VTODO;
extern const char* cICalMIMEComponent_VJOURNAL;
extern const char* cICalMIMEComponent_VFREEBUSY;

}	// namespace iCal

#endif	// CITIPDefinitions_H
