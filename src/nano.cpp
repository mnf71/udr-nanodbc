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

char last_error_message[ERROR_MESSAGE_LENGTH] = { '\0' };

char udr_locale[20] = "cp1251"; // default locale

//-----------------------------------------------------------------------------
// package nano$rslt
//
// create function set_locale (
//	 locale varchar(20) character set none not null default 'cp1251',
//	) returns ty$nano_blank
//	external name 'nano!set_locale'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(set_locale)

	FB_UDR_MESSAGE(
		InMessage,
		(FB_VARCHAR(20), udr_locale)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->udr_localeNull)
		{
			out->blank = BLANK;
			try
			{
				strncpy_s(
					nanoudr::udr_locale, in->udr_locale.length + 1, in->udr_locale.str, _TRUNCATE
				); // initialize locale
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function err_msg 
//	returns varchar(512) character set utf8
//	external name 'nano!error_message'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(error_message)

	unsigned out_count;

	enum out : short {
		e_msg = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> out_metadata(metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		out_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			out_char_sets[i] = out_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(512 * 4), e_msg)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FB_STRING(out->e_msg, (std::string)(nanoudr::last_error_message));
		out->e_msgNull = FB_FALSE;
		UTF8_OUT(e_msg);
	}

FB_UDR_END_FUNCTION

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

nanoudr::connection* conn_ptr(const char* cptr)
{
	int64_t conn = 0;
	memcpy(&conn, cptr, 8);
	return (nanoudr::connection*)conn;
}

nanodbc::transaction* tnx_ptr(const char* cptr)
{
	int64_t tnx = 0;
	memcpy(&tnx, cptr, 8);
	return (nanodbc::transaction*)tnx;
}

nanoudr::statement* stmt_ptr(const char* cptr)
{
	int64_t stmt = 0;
	memcpy(&stmt, cptr, 8);
	return (nanoudr::statement*)stmt;
}

nanodbc::result* rslt_ptr(const char* cptr)
{
	int64_t rslt = 0;
	memcpy(&rslt, cptr, 8);
	return (nanodbc::result*)rslt;
}

//-----------------------------------------------------------------------------
//

FB_BOOLEAN fb_bool(bool value)
{
	return (value ? FB_TRUE : FB_FALSE);
}

bool native_bool(const FB_BOOLEAN value)
{
	return (value == FB_TRUE ? true : value == FB_FALSE ? false : throw "Invalid FB_BOOLEAN value.");
}

//-----------------------------------------------------------------------------
//

void utf8_to_loc(char* dest, const char* src)
{
	try
	{
		iconv_t ic = iconv_open(nanoudr::udr_locale, "UTF-8");
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

void loc_to_utf8(char* dest, const char* src)
{
	try 
	{
		iconv_t ic = iconv_open("UTF-8", nanoudr::udr_locale);
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

nanodbc::timestamp set_timestamp(nanoudr::timestamp* tm)
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

nanodbc::date set_date(nanoudr::date* d)
{
	nanodbc::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanodbc::time set_time(nanoudr::time* t)
{
	nanodbc::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	return t_s;
}

nanoudr::timestamp get_timestamp(nanodbc::timestamp* tm)
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

nanoudr::date get_date(nanodbc::date* d)
{
	nanoudr::date d_s;
	d_s.year = d->year;
	d_s.month = d->month;
	d_s.day = d->day;
	return d_s;
}

nanoudr::time get_time(nanodbc::time* t)
{
	nanoudr::time t_s;
	t_s.hour = t->hour;
	t_s.min = t->min;
	t_s.sec = t->sec;
	return t_s;
}

} // namespace nanoudr

FB_UDR_IMPLEMENT_ENTRY_POINT