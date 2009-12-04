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
	CITIPDefinitions_H.cpp

	Author:
	Description:	constants from iTIP/iMIP documents
*/

#include "CITIPDefinitions.h"

namespace iCal
{

// 2446 3

const char* cICalMethod_PUBLISH = "PUBLISH";
const char* cICalMethod_REQUEST = "REQUEST";
const char* cICalMethod_REFRESH = "REFRESH";
const char* cICalMethod_CANCEL = "CANCEL";
const char* cICalMethod_ADD = "ADD";
const char* cICalMethod_REPLY = "REPLY";
const char* cICalMethod_COUNTER = "COUNTER";
const char* cICalMethod_DECLINECOUNTER = "DECLINECOUNTER";

// 2447 2.4
const char* cICalMIMEMethod_PUBLISH = "publish";
const char* cICalMIMEMethod_REQUEST = "request";
const char* cICalMIMEMethod_REFRESH = "refresh";
const char* cICalMIMEMethod_CANCEL = "cancel";
const char* cICalMIMEMethod_ADD = "add";
const char* cICalMIMEMethod_REPLY = "reply";
const char* cICalMIMEMethod_COUNTER = "counter";
const char* cICalMIMEMethod_DECLINECOUNTER = "declinecounter";

const char* cICalMIMEComponent_VEVENT = "vevent";
const char* cICalMIMEComponent_VTODO = "vtodo";
const char* cICalMIMEComponent_VJOURNAL = "vjournal";
const char* cICalMIMEComponent_VFREEBUSY = "vfreebusy";

};
