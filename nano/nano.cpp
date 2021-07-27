/*
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied.
 *  See the License for the specific language governing rights
 *  and limitations under the License.
 *
 *  The Original Code was created by Maxim Filatov for the 
 *  Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2021 Maxim Filatov <2chemist@mail.ru>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#include "nano.h"
#include <string.h>

namespace nano
{

	void fbPtr(char* cptr, int64_t iptr)
	{
		memcpy(cptr, &iptr, 8);
	}

	int64_t nativePtr(const char* cptr)
	{
		int64_t iptr = 0;
		memcpy(&iptr, cptr, 8);
		return iptr;
	}

	FB_BOOLEAN fbBool(bool value)
	{	
		return (value ? FB_TRUE : FB_FALSE);	
	}

	bool nativeBool(const FB_BOOLEAN value)
	{	
		return (value == FB_TRUE ? true : value == FB_FALSE ? false : throw "Invalid FB_BOOLEAN value.");
	}

	nanodbc::connection* connPtr(const char* cptr)
	{
		int64_t conn = 0;
		memcpy(&conn, cptr, 8);
		return (nanodbc::connection*)conn;
	}

	nanodbc::transaction* tnxPtr(const char* cptr)
	{
		int64_t tnx = 0;
		memcpy(&tnx, cptr, 8);
		return (nanodbc::transaction*)tnx;
	}

	nanodbc::statement* stmtPtr(const char* cptr)
	{
		int64_t stmt = 0;
		memcpy(&stmt, cptr, 8);
		return (nanodbc::statement*)stmt;
	}

	nanodbc::result* rsltPtr(const char* cptr)
	{
		int64_t rslt = 0;
		memcpy(&rslt, cptr, 8);
		return (nanodbc::result*)rslt;
	}

}

FB_UDR_IMPLEMENT_ENTRY_POINT