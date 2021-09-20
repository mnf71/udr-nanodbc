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

#include <iconv.h>
#include <string>

namespace nanoudr
{

//-----------------------------------------------------------------------------
// Initialize global class 
//

nanoudr::resours udr_resours;
nanoudr::helper udr_helper;

//-----------------------------------------------------------------------------
//

void helper::fb_ptr(char* cptr, int64_t iptr)
{
	memcpy(cptr, &iptr, POINTER_SIZE);
}

int64_t helper::native_ptr(const char* cptr)
{
	int64_t iptr = 0;
	memcpy(&iptr, cptr, POINTER_SIZE);
	return iptr;
}

nanoudr::connection* helper::conn_ptr(const char* cptr)
{
	int64_t conn = 0;
	memcpy(&conn, cptr, POINTER_SIZE);
	return (nanoudr::connection*)conn;
}

nanoudr::transaction* helper::tnx_ptr(const char* cptr)
{
	int64_t tnx = 0;
	memcpy(&tnx, cptr, POINTER_SIZE);
	return (nanoudr::transaction*)tnx;
}

nanoudr::statement* helper::stmt_ptr(const char* cptr)
{
	int64_t stmt = 0;
	memcpy(&stmt, cptr, POINTER_SIZE);
	return (nanoudr::statement*)stmt;
}

nanoudr::result* helper::rslt_ptr(const char* cptr)
{
	int64_t rslt = 0;
	memcpy(&rslt, cptr, POINTER_SIZE);
	return (nanoudr::result*)rslt;
}

//-----------------------------------------------------------------------------
//

FB_BOOLEAN helper::fb_bool(bool value)
{
	return (value ? FB_TRUE : FB_FALSE);
}

bool helper::native_bool(const FB_BOOLEAN value)
{
	return (value == FB_TRUE ? true : value == FB_FALSE ? false : throw "Invalid FB_BOOLEAN value.");
}

//-----------------------------------------------------------------------------
//

void helper::utf8_to_loc(char* dest, const char* src)
{
	try
	{
		iconv_t ic = iconv_open(udr_resours.locale(), "UTF-8");
		char* in = (char*) src;
		size_t in_length = strlen(in), buf_length = in_length;
		char* buf = new char[buf_length + 1], *out = buf;
		memset(buf, '\0', buf_length + 1);
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

void helper::loc_to_utf8(char* dest, const char* src)
{
	try 
	{
		iconv_t ic = iconv_open("UTF-8", udr_resours.locale());
		char* in = (char*) src;
		size_t in_length = strlen(in), buf_length = in_length * 4;
		char* buf = new char[buf_length + 1], *out = buf;
		memset(buf, '\0', buf_length + 1);
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

nanodbc::timestamp helper::set_timestamp(nanoudr::timestamp* tm)
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

nanodbc::date helper::set_date(nanoudr::date* d)
{
	nanodbc::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanodbc::time helper::set_time(nanoudr::time* t)
{
	nanodbc::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	return t_s;
}

nanoudr::timestamp helper::get_timestamp(nanodbc::timestamp* tm)
{
	nanoudr::timestamp tm_s;
	tm_s.year = tm->year;
	tm_s.month = tm->month;
	tm_s.day = tm->day;
	tm_s.hour = tm->hour;
	tm_s.min = tm->min;
	tm_s.sec = tm->sec;
	tm_s.fract = tm->fract;
	return tm_s;
}

nanoudr::date helper::get_date(nanodbc::date* d)
{
	nanoudr::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanoudr::time helper::get_time(nanodbc::time* t)
{
	nanoudr::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	return t_s;
}

} // namespace nanoudr

FB_UDR_IMPLEMENT_ENTRY_POINT