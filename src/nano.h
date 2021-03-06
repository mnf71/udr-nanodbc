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

#ifndef NANO_H
#define NANO_H

#define FB_UDR_STATUS_TYPE ::Firebird::ThrowStatusWrapper
#define FB_UDR_CONTEXT_TYPE ::Firebird::IExternalContext

#define NANOUDR_FETCH_PROCEDURE	\
	FB_BOOLEAN fetch(FB_UDR_STATUS_TYPE* status)	\
	{	\
		FB_BOOLEAN suspend;	\
		internalFetch(suspend, status);	\
		return suspend;	\
	}	\
	\
	void internalFetch(FB_BOOLEAN& suspend, FB_UDR_STATUS_TYPE* status)

#include <ibase.h>
#include <UdrCppEngine.h>

#include <nanodbc.h>

using namespace Firebird;

#include <string.h>

#include "conn.h"
#include "tnx.h"
#include "stmt.h"
#include "rslt.h"
#include "ctlg.h"

#include "rsrs.h"

namespace nanoudr
{

//-----------------------------------------------------------------------------
//

#define POINTER_SIZE	8	
#define	NANO_POINTER	FB_CHAR(POINTER_SIZE) // domain types

#define	NANO_BLANK		FB_INTEGER // domain types
#define	BLANK			-1 // void function emulation

#define FB_SEGMENT_SIZE	32768 // BLOB segment size

//-----------------------------------------------------------------------------
//

#define	DECLARE_RESOURCE	\
	const ISC_UINT64 current_att_id = 0;	\
	attachment_resources* current_att_resources = nullptr; 
/* DECLARE_RESOURCE */

#define	INITIALIZE_RESORCES	\
	if((current_att_resources =	pool.current_resources(status, context)) != nullptr)	\
		const_cast<ISC_UINT64&>(current_att_id) = current_att_resources->current_attachment_id();	\
	else	\
	{	\
		ISC_LONG exception_number = pool.exceptions.number;	\
		if (exception_number == 0)	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		else	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_except, isc_arg_number, exception_number,	\
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		return; \
	}	\
/* INITIALIZE_RESORCES */ 

#define FUNCTION_RESOURCES	\
	if ((current_att_resources = pool.current_resources(current_att_id)) == nullptr)	\
	{	\
		ISC_LONG exception_number = pool.exceptions.number;	\
		if (exception_number == 0)	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		else	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_except, isc_arg_number, exception_number,	\
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		return; \
	}	\
	attachment_resources* att_resources = current_att_resources;	\
	att_resources->current_snapshot(status, context);	\
	att_resources->current_transaction();
/* FUNCTION_RESOURCES */

#define PROCEDURE_RESOURCES	\
	if ((procedure->current_att_resources = pool.current_resources(procedure->current_att_id)) == nullptr)	\
	{	\
		ISC_LONG exception_number = pool.exceptions.number;	\
		if (exception_number == 0)	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		else	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_except, isc_arg_number, exception_number,	\
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(pool.exceptions.message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		return; \
	}	\
	attachment_resources* att_resources = procedure->current_att_resources;		\
	att_resources->current_snapshot(status, context);	\
	att_resources->current_transaction();
/* PROCEDURE_RESOURCES */

#define FETCH_RESOURCES	\
	attachment_resources* att_resources = procedure->current_att_resources;	\
	unsigned* out_char_sets = procedure->fetch_char_sets;
/* FETCH_RESOURCES */

#define	FINALIZE_RESORCES	\
	pool.finalize_attachment(current_att_id);
/* FINALIZE_RESORCES */

//-----------------------------------------------------------------------------
//

#ifdef _MSC_VER // Microsoft compilers

#	define EXPAND(x) x
#	define __NARGS(_1, _2, _3, N, ...) N
#	define __NARGS_EXPAND(...) EXPAND(__NARGS(__VA_ARGS__, NANOUDR_THROW_SPECIAL, NANOUDR_THROW_DEFAULT, ERROR))

#	define AUGMENTER(...) unused, __VA_ARGS__
#	define NANOUDR_THROW(...) __NARGS_EXPAND(AUGMENTER(__VA_ARGS__))(__VA_ARGS__)

#else // Non-Microsoft compilers

#	define __NARGS(_0, _1, _2, N, ...) N
#	define __NARGS_EXPAND(...) __NARGS(0, ## __VA_ARGS__, NANOUDR_THROW_SPECIAL, NANOUDR_THROW_DEFAULT, ERROR)
#	define NANOUDR_THROW(...) __NARGS_EXPAND(__VA_ARGS__)(__VA_ARGS__)

#endif

#define	NANOUDR_THROW_DEFAULT(exception_message)	\
{	\
	if (att_resources == nullptr)	\
	{	\
		ISC_STATUS_ARRAY vector = {	\
			isc_arg_gds,	\
			isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(NANOUDR_ERROR),	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>((exception_message)),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	else	\
	{	\
		const ISC_LONG exception_number = att_resources->exception_number(NANOUDR_ERROR);	\
		att_resources->current_error_message((exception_message));	\
		if (exception_number == 0)	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(NANOUDR_ERROR),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>((exception_message)),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		else	\
		{	\
			ISC_STATUS_ARRAY vector = { \
				isc_arg_gds,	\
				isc_except, isc_arg_number, exception_number,	\
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(NANOUDR_ERROR),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>((exception_message)),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
	}	\
	return; \
} /* NANOUDR_THROW_DEFAULT */	

#define	NANOUDR_THROW_SPECIAL(...)	\
{	\
	const char* special_exception[] = { __VA_ARGS__ };	\
	const char* exception_message = special_exception[1];	\
	const char* exception_name = special_exception[0];	\
	if (att_resources == nullptr)	\
	{	\
		ISC_STATUS_ARRAY vector = { \
			isc_arg_gds,	\
			isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(exception_name),	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>((exception_message)),	\
			isc_arg_end };	\
			status->setErrors(vector);	\
			}	\
	else	\
	{	\
		const ISC_LONG exception_number = att_resources->exception_number(exception_name);		\
		att_resources->current_error_message(exception_message);	\
		if (exception_number == 0)	\
		{	\
			ISC_STATUS vector[] = { \
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(exception_name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(exception_message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
		else	\
		{	\
			ISC_STATUS vector[] = { \
				isc_arg_gds,	\
				isc_except, isc_arg_number, exception_number,	\
				isc_arg_gds,	\
				isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(exception_name),	\
				isc_arg_gds,	\
				isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>(exception_message),	\
				isc_arg_end };	\
			status->setErrors(vector);	\
		}	\
	}	\
	return; \
} /* NANOUDR_THROW_SPECIAL */

#define	NANODBC_THROW(exception_message)	\
{	\
	const ISC_LONG exception_number = att_resources->exception_number(NANODBC_ERROR);	\
	att_resources->current_error_message((exception_message));	\
	if (exception_number == 0)	\
	{	\
		ISC_STATUS_ARRAY vector = {	\
			isc_arg_gds,	\
			isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(NANODBC_ERROR),	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>((exception_message)),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	else	\
	{	\
		ISC_STATUS_ARRAY vector = {	\
			isc_arg_gds,	\
			isc_except, isc_arg_number, exception_number,	\
			isc_arg_gds,	\
			isc_exception_name, isc_arg_string, reinterpret_cast<ISC_STATUS>(NANODBC_ERROR),	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, reinterpret_cast<ISC_STATUS>((exception_message)),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	return; \
}	/* NANODBC_THROW */

//-----------------------------------------------------------------------------
//

#define U8_VARIYNG(message, param)	\
{	\
	if (message##_char_sets[message::param] == fb_char_set::CS_UTF8)	\
	{	\
		try {	\
			helper.utf8_##message(	\
				att_resources, message->param.str, static_cast<ISC_USHORT>(sizeof(message->param.str)),	\
				message->param.str, message->param.length);	\
		} catch (std::runtime_error const& e) {	\
			NANOUDR_THROW(e.what())	\
		}	\
	}	 \
	/* trunc to one byte coding for dual purpose parameter */	\
	else \
	{	\
		ISC_USHORT trunc_length = static_cast<ISC_USHORT>(sizeof(message->param.str) / 4);	\
		if (message->param.length > trunc_length)	\
		{	\
			*(message->param.str + trunc_length) = '\0';	\
			message->param.length = trunc_length;	\
		}	\
	}	\
}
//	/* U8_VARIYNG */

#define U8_STRING(message, param)	\
{	\
	if (message##_char_sets[message::param] == fb_char_set::CS_UTF8)	\
	{	\
		ISC_USHORT param##_length = static_cast<ISC_USHORT>(sizeof(message->param.str));	\
		try {	\
			helper.utf8_##message(	\
				att_resources, message->param.str, param##_length, message->param.str, \
				/* with char spaces */ param##_length);	\
		} catch (std::runtime_error const& e) {	\
			NANOUDR_THROW(e.what())	\
		}	\
	}	\
	/* trunc to one byte coding for dual purpose parameter */	\
	else \
	{	\
		ISC_USHORT trunc_length = static_cast<ISC_USHORT>(sizeof(message->param.str) / 4);	\
		if (message->param.length > trunc_length)	\
		{	\
			*(message->param.str + trunc_length) = '\0';	\
			message->param.length = trunc_length;	\
		}	\
	}	\
}
/* U8_STRING */

//-----------------------------------------------------------------------------
//

#define FB_VARIYNG(fb_varchar, string)	\
{	\
	(fb_varchar).length = static_cast<ISC_USHORT>(sizeof((fb_varchar).str));	\
	ISC_USHORT string_length = static_cast<ISC_USHORT>((string).length());	\
	memcpy((fb_varchar).str, (string).c_str(), std::min((fb_varchar).length, string_length));	\
	if ((fb_varchar).length > string_length) (fb_varchar).length = string_length;	\
}	/* FB_VARIYNG */

#define FB_STRING(fb_char, string)	\
{	\
	ISC_USHORT fb_char_length = static_cast<ISC_USHORT>(sizeof((fb_char).str));	\
	ISC_USHORT string_length = static_cast<ISC_USHORT>((string).length());	\
	memcpy((fb_char).str, (string).c_str(), std::min(fb_char_length, string_length));	\
	if (fb_char_length > string_length)	\
		memset((fb_char).str + string_length, ' ', fb_char_length - string_length);	\
}	/* FB_STRING */

//-----------------------------------------------------------------------------
//

enum fb_char_set
{
	CS_NONE = 0,		// No Character Set

	CS_BINARY = 1,		// BINARY BYTES
	CS_ASCII = 2,		// ASCII
	CS_UNICODE_FSS = 3, // UNICODE in FSS format	- 3b
	CS_UTF8 = 4,		// UTF-8	- 4b
//	CS_SJIS = 5,		// SJIS		- 2b
//	CS_EUCJ = 6,		// EUC-J	- 2b

//	CS_JIS_0208 = 7,		// JIS 0208; 1990
//	CS_UNICODE_UCS2 = 8,	// UNICODE v 1.10

//	CS_DOS_737 = 9,
//	CS_DOS_437 = 10,	// DOS CP 437
//	CS_DOS_850 = 11,	// DOS CP 850
//	CS_DOS_865 = 12,	// DOS CP 865
//	CS_DOS_860 = 13,	// DOS CP 860
//	CS_DOS_863 = 14,	// DOS CP 863

//	CS_DOS_775 = 15,
//	CS_DOS_858 = 16,
//	CS_DOS_862 = 17,
//	CS_DOS_864 = 18,

//	CS_NEXT = 19,		// NeXTSTEP OS native charset

//	CS_ISO8859_1 = 21,	// ISO-8859.1
//	CS_ISO8859_2 = 22,	// ISO-8859.2
//	CS_ISO8859_3 = 23,	// ISO-8859.3
//	CS_ISO8859_4 = 34,	// ISO-8859.4
//	CS_ISO8859_5 = 35,	// ISO-8859.5
//	CS_ISO8859_6 = 36,	// ISO-8859.6
//	CS_ISO8859_7 = 37,	// ISO-8859.7
//	CS_ISO8859_8 = 38,	// ISO-8859.8
//	CS_ISO8859_9 = 39,	// ISO-8859.9
//	CS_ISO8859_13 = 40,	// ISO-8859.13

//	CS_KSC5601 = 44,	// KOREAN STANDARD 5601	- 2b

//	CS_DOS_852 = 45,	// DOS CP 852
//	CS_DOS_857 = 46,	// DOS CP 857
//	CS_DOS_861 = 47,	// DOS CP 861

//	CS_DOS_866 = 48,
//	CS_DOS_869 = 49,

//	CS_CYRL = 50,
//	CS_WIN1250 = 51,	// Windows cp 1250
//	CS_WIN1251 = 52,	// Windows cp 1251
//	CS_WIN1252 = 53,	// Windows cp 1252
//	CS_WIN1253 = 54,	// Windows cp 1253
//	CS_WIN1254 = 55,	// Windows cp 1254

//	CS_BIG5 = 56,		// Big Five unicode cs
//	CS_GB2312 = 57,		// GB 2312-80 cs	- 2b

//	CS_WIN1255 = 58,	// Windows cp 1255
//	CS_WIN1256 = 59,	// Windows cp 1256
//	CS_WIN1257 = 60,	// Windows cp 1257

//	CS_UTF16 = 61,		// UTF-16
//	CS_UTF32 = 62,		// UTF-32

//	CS_KOI8R = 63,		// Russian KOI8R
//	CS_KOI8U = 64,		// Ukrainian KOI8U

//	CS_WIN1258 = 65,	// Windows cp 1258

//	CS_TIS620 = 66,		// TIS620
//	CS_GBK = 67,		// GBK	- 2b	 
//	CS_CP943C = 68,		// CP943C	- 2b

//	CS_GB18030 = 69,	// GB18030	- 4b

	CS_LOCALE = -1		// UDR local
};

//-----------------------------------------------------------------------------
//

struct date
{
	unsigned year; unsigned month; unsigned day;
};

// Default standart size of fraction it's 9 number  
// It's 9 = ISC_TIME_SECONDS_PRECISION * STD_TIME_SECONDS_PRECISION
#define STD_TIME_SECONDS_PRECISION	100000L

struct time
{
	unsigned hour; unsigned min; unsigned sec; unsigned fract;
};

struct timestamp
{
	date d;
	time t;
};

class nano_helper
{
public:
	template <class T> void fb_ptr(char* fb_pointer, const T* native_pointer);
	template <class T> T* native_ptr(const char* fb_pointer) const;

	FB_BOOLEAN fb_bool(bool value) const;
	bool native_bool(const ISC_UCHAR value) const;

	const ISC_USHORT utf8_in(attachment_resources* att_resources, char* in, const ISC_USHORT in_length,
		const char* utf8, const ISC_USHORT utf8_length);
	const ISC_USHORT utf8_out(attachment_resources* att_resources, char* out, const ISC_USHORT out_length,
		const char* locale, const ISC_USHORT locale_length);

	const ISC_USHORT unicode_converter(char* dest, const ISC_USHORT dest_length, const char* to,
		const char* src, const ISC_USHORT src_length, const char* from);

	nanodbc::timestamp set_timestamp(const nanoudr::timestamp* tm);
	nanodbc::date set_date(const nanoudr::date* d);
	nanodbc::time set_time(const nanoudr::time* t);

	nanoudr::timestamp get_timestamp(const nanodbc::timestamp* tm);
	nanoudr::date get_date(const nanodbc::date* d);
	nanoudr::time get_time(const nanodbc::time* t);

	void read_blob(attachment_resources* att_resources, ISC_QUAD* in, class std::vector<uint8_t>* out);
	
	void write_blob(attachment_resources* att_resources, class std::vector<uint8_t>* in, ISC_QUAD* out);
	void write_blob(attachment_resources* att_resources, nanodbc::string* in, ISC_QUAD* out);

private:
	void write_blob(attachment_resources* att_resources, const unsigned char* in, const std::size_t in_length,
		ISC_QUAD* out);
};

extern nano_helper helper;

} // namespace nanoudr

/*
 *  The Original Code was created by Adriano dos Santos Fernandes
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2015 Adriano dos Santos Fernandes <adrianosf@gmail.com>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#include <assert.h>
#include <stdio.h>

namespace
{
	template <typename T>
	class AutoReleaseClear
	{
	public:
		static void clear(T* ptr)
		{
			if (ptr)
				ptr->release();
		}
	};

	template <typename T>
	class AutoDisposeClear
	{
	public:
		static void clear(T* ptr)
		{
			if (ptr)
				ptr->dispose();
		}
	};

	template <typename T>
	class AutoDeleteClear
	{
	public:
		static void clear(T* ptr)
		{
			delete ptr;
		}
	};

	template <typename T>
	class AutoArrayDeleteClear
	{
	public:
		static void clear(T* ptr)
		{
			delete[] ptr;
		}
	};

	template <typename T, typename Clear>
	class AutoImpl
	{
	public:
		AutoImpl<T, Clear>(T* aPtr = NULL)
			: ptr(aPtr)
		{
		}

		~AutoImpl()
		{
			Clear::clear(ptr);
		}

		AutoImpl<T, Clear>& operator =(T* aPtr)
		{
			Clear::clear(ptr);
			ptr = aPtr;
			return *this;
		}

		operator T* ()
		{
			return ptr;
		}

		operator const T* () const
		{
			return ptr;
		}

		bool operator !() const
		{
			return !ptr;
		}

		bool hasData() const
		{
			return ptr != NULL;
		}

		T* operator ->()
		{
			return ptr;
		}

		T* release()
		{
			T* tmp = ptr;
			ptr = NULL;
			return tmp;
		}

		void reset(T* aPtr = NULL)
		{
			if (aPtr != ptr)
			{
				Clear::clear(ptr);
				ptr = aPtr;
			}
		}

	private:
		AutoImpl<T, Clear>(AutoImpl<T, Clear>&); // not implemented
		void operator =(AutoImpl<T, Clear>&);

	private:
		T* ptr;
	};

	template <typename T> class AutoDispose : public AutoImpl<T, AutoDisposeClear<T> >
	{
	public:
		AutoDispose(T* ptr = NULL)
			: AutoImpl<T, AutoDisposeClear<T> >(ptr)
		{
		}
	};

	template <typename T> class AutoRelease : public AutoImpl<T, AutoReleaseClear<T> >
	{
	public:
		AutoRelease(T* ptr = NULL)
			: AutoImpl<T, AutoReleaseClear<T> >(ptr)
		{
		}
	};

	template <typename T> class AutoDelete : public AutoImpl<T, AutoDeleteClear<T> >
	{
	public:
		AutoDelete(T* ptr = NULL)
			: AutoImpl<T, AutoDeleteClear<T> >(ptr)
		{
		}
	};

	template <typename T> class AutoArrayDelete : public AutoImpl<T, AutoArrayDeleteClear<T> >
	{
	public:
		AutoArrayDelete(T* ptr = NULL)
			: AutoImpl<T, AutoArrayDeleteClear<T> >(ptr)
		{
		}
	};
}

#endif	/* NANO_H */     