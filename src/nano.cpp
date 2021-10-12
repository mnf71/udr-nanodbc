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

const ISC_USHORT helper::utf8_in(attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* utf8, const ISC_USHORT utf8_length)
{
	return utf8_converter(dest, dest_size, att_resources->locale(), utf8, utf8_length, "UTF-8");
}

const ISC_USHORT helper::utf8_out(attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
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

void helper::read_blob(attachment_resources* att_resources, ISC_QUAD* in, class std::vector<uint8_t>* out)
{
	const resources_context* att_context = att_resources->context();

	FB_UDR_STATUS_TYPE* status = att_context->status;
	FB_UDR_CONTEXT_TYPE* context = att_context->context;

	AutoRelease<IAttachment> att;
	AutoRelease<ITransaction> tra;
	AutoRelease<IBlob> blob;

	AutoArrayDelete<unsigned char> buffer;
	out->clear();
	try
	{
		unsigned read = 0;
		att.reset(context->getAttachment(status));
		tra.reset(context->getTransaction(status));
		blob.reset(att->openBlob(status, tra, in, 0, NULL));
		buffer.reset(new unsigned char[FB_SEGMENT_SIZE]);
		for (bool eof = false; !eof; )
		{
			switch (blob->getSegment(status, FB_SEGMENT_SIZE, buffer, &read))
			{
				case IStatus::RESULT_OK: 
				case IStatus::RESULT_SEGMENT: 
				{
					out->insert(out->end(), reinterpret_cast<uint8_t*>(static_cast<unsigned char*>(buffer)),
						reinterpret_cast<uint8_t*>(static_cast<unsigned char*>(buffer)) + (read * sizeof(unsigned char)) / sizeof(uint8_t)
					);
					break;
				}
				default:
				{
					eof = true;
					continue;
				}
			}
		}
		blob->close(status);
	}
	catch (...)
	{
		throw;
	}
}

void helper::write_blob(attachment_resources* att_resources, class std::vector<uint8_t>* in, ISC_QUAD* out)
{
	write_blob(att_resources, 
		reinterpret_cast<unsigned char*>(in->data()), (in->size() * sizeof(uint8_t)) / sizeof(unsigned char), out);
}

void helper::write_blob(attachment_resources* att_resources, nanodbc::string* in, ISC_QUAD* out)
{
	write_blob(att_resources, (unsigned char*)(in->c_str()), in->length(), out);
}

void helper::write_blob(
	attachment_resources* att_resources, const unsigned char* in, const std::size_t in_length, ISC_QUAD* out)
{
	const resources_context* att_context = att_resources->context();

	FB_UDR_STATUS_TYPE* status = att_context->status;
	FB_UDR_CONTEXT_TYPE* context = att_context->context;

	AutoRelease<IAttachment> att;
	AutoRelease<ITransaction> tra;
	AutoRelease<IBlob> blob;

	unsigned char* stream = (unsigned char*)(in);
	std::size_t stream_size = (std::size_t)(in_length);
	try
	{
		unsigned write = 0;
		att.reset(context->getAttachment(status));
		tra.reset(context->getTransaction(status));
		blob.reset(att->createBlob(status, tra, out, 0, NULL));
		while (stream_size > 0)
		{
			write = stream_size < FB_SEGMENT_SIZE ? (unsigned)(stream_size) : FB_SEGMENT_SIZE;
			blob->putSegment(status, write, stream);
			stream_size = stream_size - write;
			stream += write;
		}
		blob->close(status);
	}
	catch (...)
	{
		throw;
	}
}


} // namespace nanoudr

FB_UDR_IMPLEMENT_ENTRY_POINT