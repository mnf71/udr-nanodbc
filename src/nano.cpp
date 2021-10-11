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

nanoudr::resources udr_resources;
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

const ISC_USHORT helper::utf8_in(nanoudr::attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* utf8, const ISC_USHORT utf8_length)
{
	return utf8_converter(dest, dest_size, att_resources->locale(), utf8, utf8_length, "UTF-8");
}

const ISC_USHORT helper::utf8_out(nanoudr::attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* locale, const ISC_USHORT locale_length)
{
	return utf8_converter(dest, dest_size, "UTF-8", locale, locale_length, att_resources->locale());
}

const ISC_USHORT helper::utf8_converter(char* dest, const ISC_USHORT dest_size, const char* to,
	const char* src, const ISC_USHORT src_length, const char* from)
{
	char* in = (char*)(src);
	size_t in_length = src_length; // strlen() src
	char* converted = new char[dest_size + 1];
	memset(converted, '\0', dest_size + 1);
	char* out = converted;
	size_t out_indicator = dest_size; // sizeof() dest
	try
	{
		iconv_t ic = iconv_open(to, from);
		iconv(ic, &in, &in_length, &out, &out_indicator); 
		iconv_close(ic);
	}
	catch (...)
	{
		throw "iconv: UTF8 character conversion error.";
	}
	memset(dest, '\0', dest_size); // not null-term string, just buffer
	memcpy_s(dest, dest_size, converted, dest_size - out_indicator);
	delete[] converted;

	return (ISC_USHORT)(dest_size - out_indicator);
}

//-----------------------------------------------------------------------------
//

nanodbc::timestamp helper::set_timestamp(const nanoudr::timestamp* tm)
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

nanodbc::date helper::set_date(const nanoudr::date* d)
{
	nanodbc::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanodbc::time helper::set_time(const nanoudr::time* t)
{
	nanodbc::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	// = t->fract
	return t_s;
}

nanoudr::timestamp helper::get_timestamp(const nanodbc::timestamp* tm)
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

nanoudr::date helper::get_date(const nanodbc::date* d)
{
	nanoudr::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanoudr::time helper::get_time(const nanodbc::time* t)
{
	nanoudr::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	t_s.fract = 0;
	return t_s;
}

//std::vector<uint8_t> helper::get_blob(nanoudr::attachment_resources* att_resources, ISC_QUAD blob)
//{
//}

ISC_QUAD helper::put_blob(nanoudr::attachment_resources* att_resources, std::vector<uint8_t> blob)
{
	return {0,0};
}


} // namespace nanoudr

FB_UDR_IMPLEMENT_ENTRY_POINT