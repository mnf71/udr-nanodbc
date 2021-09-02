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

#include <iconv.h>

namespace nano
{

//-----------------------------------------------------------------------------
//
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
//
FB_BOOLEAN fb_bool(bool value)
{	
	return (value ? FB_TRUE : FB_FALSE);	
}

bool native_bool(const FB_BOOLEAN value)
{	
	return (value ? true : !value ? false : throw "Invalid FB_BOOLEAN value.");
}

//-----------------------------------------------------------------------------
//
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
//
char udr_locale[20] = "cp1251"; // default


void utf8_to_loc(char* dest, const char* src)
{
	try
	{
		iconv_t ic = iconv_open(nano::udr_locale, "UTF-8");
		char* in = (char*) src;
		size_t in_length = strlen(in);
		char* buf = new char[in_length + 1], * out = buf;
		memset(buf, '\0', in_length + 1);
		size_t out_length = in_length;
		iconv(ic, &in, &in_length, &out, &out_length);
		strcpy(dest, buf);
		iconv_close(ic);
		delete[] buf; 
	}
	catch (...)
	{
		throw "iconv: cannot convert to ODBC locale.";
	}
}

void loc_to_utf8(char* dest, const char* src)
{
	try 
	{
		iconv_t ic = iconv_open("UTF-8", nano::udr_locale);
		char* in = (char*) src;
		size_t in_length = strlen(in);
		char* buf = new char[(in_length * 4) + 1], * out = buf;
		memset(buf, '\0', (in_length * 4) + 1);
		size_t out_length = in_length * 4;
		// todo: иногда строка возвращается не полностью
		iconv(ic, &in, &in_length, &out, &out_length);
		strcpy(dest, buf);
		iconv_close(ic);
		delete[] buf;
	}
	catch (...)
	{
		throw "iconv: cannot convert to UTF-8.";
	}
}

//-----------------------------------------------------------------------------
//

nanodbc::timestamp set_timestamp(nano::timestamp* tm)
{
	nanodbc::timestamp tm_s;
	tm_s.year = tm->year;
	tm_s.month = tm->month;
	tm_s.day = tm->day;
	tm_s.hour = tm->hour;
	tm_s.min = tm->min;
	tm_s.sec = tm->sec;
	tm_s.fract = tm->fract;
	return tm_s;
}

nanodbc::date set_date(nano::date* d)
{
	nanodbc::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanodbc::time set_time(nano::time* t)
{
	nanodbc::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	return t_s;
}

nano::timestamp get_timestamp(nanodbc::timestamp* tm)
{
	nano::timestamp tm_s;
	tm_s.year = tm->year;
	tm_s.month = tm->month;
	tm_s.day = tm->day;
	tm_s.hour = tm->hour;
	tm_s.min = tm->min;
	tm_s.sec = tm->sec;
	tm_s.fract = tm->fract;
	return tm_s;
}

nano::date get_date(nanodbc::date* d)
{
	nano::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nano::time get_time(nanodbc::time* t)
{
	nano::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	return t_s;
}

} // namespace nano

FB_UDR_IMPLEMENT_ENTRY_POINT