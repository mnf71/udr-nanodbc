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

nano_helper helper;

//-----------------------------------------------------------------------------
//

template <class T> void nano_helper::fb_ptr(char* fb_pointer, const T* native_pointer)
{
	int64_t ptr = reinterpret_cast<const int64_t>(native_pointer);
	memcpy(fb_pointer, &(ptr), POINTER_SIZE);
}

// The following are the only supported instantiations of pointer.
template void  nano_helper::fb_ptr(char*, const nanoudr::connection*);
template void  nano_helper::fb_ptr(char*, const nanoudr::transaction*);
template void  nano_helper::fb_ptr(char*, const nanoudr::statement*);
template void nano_helper::fb_ptr(char*, const nanoudr::result*);

template <class T> T* nano_helper::native_ptr(const char* fb_pointer) const
{
	int64_t ptr = 0x0;
	memcpy(&ptr, fb_pointer, POINTER_SIZE);
	return reinterpret_cast<T*>(ptr);
}

// The following are the only supported instantiations of pointer.
template nanoudr::connection* nano_helper::native_ptr(const char*) const;
template nanoudr::transaction* nano_helper::native_ptr(const char*) const;
template nanoudr::statement* nano_helper::native_ptr(const char*) const;
template nanoudr::result* nano_helper::native_ptr(const char*) const;

//-----------------------------------------------------------------------------
//

FB_BOOLEAN nano_helper::fb_bool(bool value) const
{
	return (value ? FB_TRUE : FB_FALSE);
}

bool nano_helper::native_bool(const FB_BOOLEAN value) const
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

const ISC_USHORT nano_helper::utf8_in(attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* utf8, const ISC_USHORT utf8_length)
{
	return unicode_converter(dest, dest_size, att_resources->current_locale(), utf8, utf8_length, "UTF-8");
}

const ISC_USHORT nano_helper::utf8_out(attachment_resources* att_resources, char* dest, const ISC_USHORT dest_size,
	const char* locale, const ISC_USHORT locale_length)
{
	return unicode_converter(dest, dest_size, "UTF-8", locale, locale_length, att_resources->current_locale());
}

const ISC_USHORT nano_helper::unicode_converter(char* dest, const ISC_USHORT dest_size, const char* to,
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

nanodbc::timestamp nano_helper::set_timestamp(const nanoudr::timestamp* tm)
{
	return nanodbc::timestamp({
		static_cast<int16_t>(tm->d.year), static_cast<int16_t>(tm->d.month), static_cast<int16_t>(tm->d.day),
		static_cast<int16_t>(tm->t.hour), static_cast<int16_t>(tm->t.min), static_cast<int16_t>(tm->t.sec),
		static_cast<int32_t>(tm->t.fract * STD_TIME_SECONDS_PRECISION)
	});
}

nanodbc::date nano_helper::set_date(const nanoudr::date* d)
{
	return nanodbc::date({ 
		static_cast<int16_t>(d->year), static_cast<int16_t>(d->month), static_cast<int16_t>(d->day) 
	});
}

nanodbc::time nano_helper::set_time(const nanoudr::time* t)
{
	return nanodbc::time({ 
		static_cast<int16_t>(t->hour), static_cast<int16_t>(t->min), static_cast<int16_t>(t->sec) 
	});
}

nanoudr::timestamp nano_helper::get_timestamp(const nanodbc::timestamp* tm)
{
	return nanoudr::timestamp({
		static_cast<unsigned>(tm->year), static_cast<unsigned>(tm->month), static_cast<unsigned>(tm->day),
		static_cast<unsigned>(tm->hour), static_cast<unsigned>(tm->min), static_cast<unsigned>(tm->sec),
		static_cast<unsigned>(tm->fract / STD_TIME_SECONDS_PRECISION)
	});
}

nanoudr::date nano_helper::get_date(const nanodbc::date* d)
{
	return nanoudr::date({
		static_cast<unsigned>(d->year), static_cast<unsigned>(d->month), static_cast<unsigned>(d->day)
	});
}

nanoudr::time nano_helper::get_time(const nanodbc::time* t)
{
	return nanoudr::time({
		static_cast<unsigned>(t->hour), static_cast<unsigned>(t->min), static_cast<unsigned>(t->sec),
		static_cast<unsigned>(0)
	});
}

void nano_helper::read_blob(attachment_resources* att_resources, ISC_QUAD* in, class std::vector<uint8_t>* out)
{
	const attachment_snapshot* att_snapshot = att_resources->current_snapshot();

	AutoRelease<IAttachment> att;
	AutoRelease<IBlob> blob;

	try
	{
		AutoArrayDelete<unsigned char> buffer;
		unsigned read = 0;

		att.reset(att_snapshot->context->getAttachment(att_snapshot->status));
		blob.reset(att->openBlob(
			att_snapshot->status, const_cast<ITransaction*>(att_resources->current_transaction()), in, 0, NULL));
		buffer.reset(new unsigned char[FB_SEGMENT_SIZE]);
		out->clear();
		for (bool eof = false; !eof; )
		{
			switch (blob->getSegment(att_snapshot->status, FB_SEGMENT_SIZE, buffer, &read))
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
		blob->close(att_snapshot->status);
	}
	catch (...)
	{
		throw std::runtime_error("Error reading BLOB to stream.");
	}
}

void nano_helper::write_blob(attachment_resources* att_resources, class std::vector<uint8_t>* in, ISC_QUAD* out)
{
	write_blob(att_resources, reinterpret_cast<const unsigned char*>(in->data()), in->size(), out);
}

void nano_helper::write_blob(attachment_resources* att_resources, nanodbc::string* in, ISC_QUAD* out)
{
	write_blob(att_resources, reinterpret_cast<const unsigned char*>(in->c_str()), in->length(), out);
}

void nano_helper::write_blob(
	attachment_resources* att_resources, const unsigned char* in, const std::size_t in_length, ISC_QUAD* out)
{
	const attachment_snapshot* att_snapshot = att_resources->current_snapshot();

	AutoRelease<IAttachment> att;
	AutoRelease<IBlob> blob;

	try
	{
		unsigned char* stream = const_cast<unsigned char*>(in);
		std::size_t stream_size = in_length;
		unsigned write = 0;

		att.reset(att_snapshot->context->getAttachment(att_snapshot->status));
		blob.reset(att->createBlob(
			att_snapshot->status, const_cast<ITransaction*>(att_resources->current_transaction()), out, 0, NULL));
		while (stream_size > 0)
		{
			write = stream_size < FB_SEGMENT_SIZE ? static_cast<unsigned>(stream_size) : FB_SEGMENT_SIZE;
			blob->putSegment(att_snapshot->status, write, stream);
			stream_size = stream_size - write;
			stream += write;
		}
		blob->close(att_snapshot->status);
	}
	catch (...)
	{
		throw std::runtime_error("Error writing stream to BLOB.");
	}
}


} // namespace nanoudr

FB_UDR_IMPLEMENT_ENTRY_POINT