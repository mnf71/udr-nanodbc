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

namespace nanoudr
{

//-----------------------------------------------------------------------------
//

helper udr_helper;

template <class T> void helper::fb_ptr(char* fb_pointer, const T* native_pointer)
{
	int64_t ptr = reinterpret_cast<const int64_t>(native_pointer);
	memcpy(fb_pointer, &(ptr), POINTER_SIZE);
}

// The following are the only supported instantiations of pointer.
template void  helper::fb_ptr(char*, const nanoudr::connection*);
template void  helper::fb_ptr(char*, const nanoudr::transaction*);
template void  helper::fb_ptr(char*, const nanoudr::statement*);
template void helper::fb_ptr(char*, const nanoudr::result*);

template <class T> T* helper::native_ptr(const char* fb_pointer) const
{
	int64_t ptr = 0x0;
	memcpy(&ptr, fb_pointer, POINTER_SIZE);
	return reinterpret_cast<T*>(ptr);
}

// The following are the only supported instantiations of pointer.
template nanoudr::connection* helper::native_ptr(const char*) const;
template nanoudr::transaction* helper::native_ptr(const char*) const;
template nanoudr::statement* helper::native_ptr(const char*) const;
template nanoudr::result* helper::native_ptr(const char*) const;

//-----------------------------------------------------------------------------
//

FB_BOOLEAN helper::fb_bool(bool value) const
{
	return (value ? FB_TRUE : FB_FALSE);
}

bool helper::native_bool(const FB_BOOLEAN value) const
{
	return (
		value == FB_TRUE ? true 
		: value == FB_FALSE ? false 
		: throw std::runtime_error("Invalid FB_BOOLEAN value.")
	);
}

//-----------------------------------------------------------------------------
//

#include <iconv.h>

const ISC_USHORT helper::utf8_in(attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* utf8, const ISC_USHORT utf8_length)
{
	return unicode_converter(dest, dest_size, att_resources->locale(), utf8, utf8_length, "UTF-8");
}

const ISC_USHORT helper::utf8_out(attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* locale, const ISC_USHORT locale_length)
{
	return unicode_converter(dest, dest_size, "UTF-8", locale, locale_length, att_resources->locale());
}

const ISC_USHORT helper::unicode_converter(char* dest, const ISC_USHORT dest_size, const char* to,
	const char* src, const ISC_USHORT src_length, const char* from)
{
	char* in = const_cast<char*>(src);
	size_t in_length = src_length; // strlen() src
	char* converted = new char[dest_size];
	char* out = converted;
	size_t out_indicator = dest_size; // sizeof() dest
	try
	{
		iconv_t ic = iconv_open(to, from);
		if (ic == (iconv_t)-1)
			throw std::runtime_error("iconv: Unsuccesful conversion descriptor.");
		else 
		{
			iconv(ic, &in, &in_length, &out, &out_indicator);
			iconv_close(ic);
		}
	}
	catch (std::runtime_error const& e)
	{	
		if (!strlen(e.what()))
			throw std::runtime_error("iconv: Character conversion error.");
		else
			throw std::runtime_error(e.what());
	}
	ISC_USHORT converted_size = dest_size - static_cast<ISC_USHORT>(out_indicator);
	if (dest_size > converted_size) dest[converted_size] = '\0'; // null-term if oversize
	memcpy(dest, converted, converted_size);
	delete[] converted;

	return converted_size;
}

//-----------------------------------------------------------------------------
//

nanodbc::timestamp helper::set_timestamp(const nanoudr::timestamp* tm)
{
	return nanodbc::timestamp({
		static_cast<int16_t>(tm->d.year), static_cast<int16_t>(tm->d.month), static_cast<int16_t>(tm->d.day),
		static_cast<int16_t>(tm->t.hour), static_cast<int16_t>(tm->t.min), static_cast<int16_t>(tm->t.sec),
		static_cast<int32_t>(tm->t.fract * STD_TIME_SECONDS_PRECISION)
	});
}

nanodbc::date helper::set_date(const nanoudr::date* d)
{
	return nanodbc::date({ 
		static_cast<int16_t>(d->year), static_cast<int16_t>(d->month), static_cast<int16_t>(d->day) 
	});
}

nanodbc::time helper::set_time(const nanoudr::time* t)
{
	return nanodbc::time({ 
		static_cast<int16_t>(t->hour), static_cast<int16_t>(t->min), static_cast<int16_t>(t->sec) 
	});
}

nanoudr::timestamp helper::get_timestamp(const nanodbc::timestamp* tm)
{
	return nanoudr::timestamp({
		static_cast<unsigned>(tm->year), static_cast<unsigned>(tm->month), static_cast<unsigned>(tm->day),
		static_cast<unsigned>(tm->hour), static_cast<unsigned>(tm->min), static_cast<unsigned>(tm->sec),
		static_cast<unsigned>(tm->fract / STD_TIME_SECONDS_PRECISION)
	});
}

nanoudr::date helper::get_date(const nanodbc::date* d)
{
	return nanoudr::date({
		static_cast<unsigned>(d->year), static_cast<unsigned>(d->month), static_cast<unsigned>(d->day)
	});
}

nanoudr::time helper::get_time(const nanodbc::time* t)
{
	return nanoudr::time({
		static_cast<unsigned>(t->hour), static_cast<unsigned>(t->min), static_cast<unsigned>(t->sec),
		static_cast<unsigned>(0)
	});
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
		if (att_context->autonomous_transaction)
			blob.reset(att->openBlob(status, att_context->autonomous_transaction, in, 0, NULL));
		else
		{
			tra.reset(context->getTransaction(status));
			blob.reset(att->openBlob(status, tra, in, 0, NULL));
		}
		buffer.reset(new unsigned char[FB_SEGMENT_SIZE]);
		for (bool eof = false; !eof; )
		{
			switch (blob->getSegment(status, FB_SEGMENT_SIZE, buffer, &read))
			{
				case IStatus::RESULT_OK: 
				case IStatus::RESULT_SEGMENT: 
				{
					out->insert(out->end(), 
						reinterpret_cast<uint8_t*>(static_cast<unsigned char*>(buffer)),
						reinterpret_cast<uint8_t*>(static_cast<unsigned char*>(buffer)) + read);
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
		throw std::runtime_error("Error reading BLOB to stream.");
	}
}

void helper::write_blob(attachment_resources* att_resources, class std::vector<uint8_t>* in, ISC_QUAD* out)
{
	write_blob(att_resources, reinterpret_cast<const unsigned char*>(in->data()), in->size(), out);
}

void helper::write_blob(attachment_resources* att_resources, nanodbc::string* in, ISC_QUAD* out)
{
	write_blob(att_resources, reinterpret_cast<const unsigned char*>(in->c_str()), in->length(), out);
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

	unsigned char* stream = const_cast<unsigned char*>(in);
	std::size_t stream_size = in_length;
	try
	{
		unsigned write = 0;
		att.reset(context->getAttachment(status));
		if (att_context->autonomous_transaction)
			blob.reset(att->createBlob(status, att_context->autonomous_transaction, out, 0, NULL));
		else
		{
			tra.reset(context->getTransaction(status));
			blob.reset(att->createBlob(status, tra, out, 0, NULL));
		}
		while (stream_size > 0)
		{
			write = stream_size < FB_SEGMENT_SIZE ? static_cast<unsigned>(stream_size) : FB_SEGMENT_SIZE;
			blob->putSegment(status, write, stream);
			stream_size = stream_size - write;
			stream += write;
		}
		blob->close(status);
	}
	catch (...)
	{
		throw std::runtime_error("Error writing stream to BLOB.");
	}
}


} // namespace nanoudr

FB_UDR_IMPLEMENT_ENTRY_POINT