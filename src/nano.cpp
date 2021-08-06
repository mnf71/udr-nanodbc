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

#include <string>
#include <codecvt>
#include <locale>

namespace nano
{

char odbc_locale[10] = ".1251"; // default

//-----------------------------------------------------------------------------
void fb_ptr(char* cptr, int64_t iptr)
{
	memcpy(cptr, &iptr, 8);
}

int64_t native_ptr(const char* cptr)
{
	int64_t iptr = 0;
	memcpy(&iptr, cptr, 8);
	return iptr;
}

//-----------------------------------------------------------------------------
FB_BOOLEAN fb_bool(bool value)
{	
	return (value ? FB_TRUE : FB_FALSE);	
}

bool native_bool(const FB_BOOLEAN value)
{	
	return (value ? true : !value ? false : throw "Invalid FB_BOOLEAN value.");
}

//-----------------------------------------------------------------------------
nanodbc::connection* conn_ptr(const char* cptr)
{
	int64_t conn = 0;
	memcpy(&conn, cptr, 8);
	return (nanodbc::connection*)conn;
}

nanodbc::transaction* tnx_ptr(const char* cptr)
{
	int64_t tnx = 0;
	memcpy(&tnx, cptr, 8);
	return (nanodbc::transaction*)tnx;
}

nanodbc::statement* stmt_ptr(const char* cptr)
{
	int64_t stmt = 0;
	memcpy(&stmt, cptr, 8);
	return (nanodbc::statement*)stmt;
}

nanodbc::result* rslt_ptr(const char* cptr)
{
	int64_t rslt = 0;
	memcpy(&rslt, cptr, 8);
	return (nanodbc::result*)rslt;
}

//-----------------------------------------------------------------------------
void utf8_to_odbc_locale(char* dest, const char* utf)
{
	// UTF-8 to wstring
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wc;
	std::wstring ws = wc.from_bytes(utf);
	// wstring to ANSI
	std::locale loc(nano::odbc_locale);
	std::use_facet<std::ctype<wchar_t>>(loc).narrow(ws.data(), ws.data() + ws.length() + 1, '?', dest);
}

void odbc_locale_to_utf8(char* dest, const char* ansi)
{
	// ANSII to wstring
	std::string s = ansi;
	std::wstring ws(s.size(), 0);
	std::locale loc(nano::odbc_locale);
	std::use_facet<std::ctype<wchar_t>>(loc).widen(s.data(), s.data() + s.size(), &ws[0]);
	// wstring to UTF-8
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wc;
	s = wc.to_bytes(ws);
	dest = s.data();
}

} // namespace nano

FB_UDR_IMPLEMENT_ENTRY_POINT