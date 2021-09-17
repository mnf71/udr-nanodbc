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
 *  The Original Code was created by Adriano dos Santos Fernandes
 *  for the Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2015 Adriano dos Santos Fernandes <adrianosf@gmail.com>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________. 
 *
 *  2021 Maxim Filatov <2chemist@mail.ru>
 */

#ifndef NANO_H
#define NANO_H

#define FB_UDR_STATUS_TYPE ::Firebird::ThrowStatusWrapper

#include <ibase.h>
#include <UdrCppEngine.h>

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
			delete [] ptr;
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

		operator T*()
		{
			return ptr;
		}

		operator const T*() const
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

/*
 *  The Original Code was created by Maxim Filatov for the
 *  Firebird Open Source RDBMS project.
 *
 *  Copyright (c) 2021 Maxim Filatov <2chemist@mail.ru>
 *  and all contributors signed below.
 *
 *  All Rights Reserved.
 *  Contributor(s): ______________________________________.
 */

#include <nanodbc.h> 

using namespace Firebird;

#include "conn.h" 
#include "tnx.h" 
#include "stmt.h" 
#include "rslt.h" 
#include "rsrs.h" 

namespace nanoudr
{

//-----------------------------------------------------------------------------
//

#define	ANY_THROW(exception_message)	\
{	\
	udr_resours.error_message((exception_message));	\
	ISC_STATUS_ARRAY vector = {	\
		isc_arg_gds,	\
		isc_random, isc_arg_string, (ISC_STATUS)((exception_message)),	\
		isc_arg_end};	\
	status->setErrors(vector);	\
	return; \
}	/* ANY_THROW */

#define	NANODBC_THROW(exception_message)	\
{	\
	ISC_LONG exception_number = udr_resours.exception_number(NANODBC_ERR_MESSAGE);	\
	if (exception_number == 0)	\
	{	\
		ISC_STATUS_ARRAY vector = {	\
			isc_arg_gds,	\
			isc_exception_name, isc_arg_string, (ISC_STATUS)(NANODBC_ERR_MESSAGE),	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, (ISC_STATUS)(udr_resours.error_message()),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	else	\
	{	\
		udr_resours.error_message((exception_message));	\
		ISC_STATUS_ARRAY vector = {	\
			isc_arg_gds,	\
			isc_except, isc_arg_number, exception_number,	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, (ISC_STATUS)((exception_message)),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	return; \
}	/* NANODBC_THROW */

#define	NANOUDR_THROW(exception_name)	\
{	\
	ISC_LONG exception_number = udr_resours.exception_number((exception_name));		\
	udr_resours.error_message(udr_resours.exception_message((exception_name)));	\
	if (exception_number == 0)	\
	{	\
		ISC_STATUS vector[] = {	\
			isc_arg_gds,	\
			isc_exception_name, isc_arg_string, (ISC_STATUS)(exception_name),	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, (ISC_STATUS)(udr_resours.error_message()),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	else	\
	{	\
		ISC_STATUS vector[] = {	\
			isc_arg_gds,	\
			isc_except, isc_arg_number, exception_number,	\
			isc_arg_gds,	\
			isc_random, isc_arg_string, (ISC_STATUS)(udr_resours.error_message()),	\
			isc_arg_end};	\
		status->setErrors(vector);	\
	}	\
	return; \
}	/* NANOUDR_THROW */

#define	NANO_POINTER			FB_CHAR(8)	// domain types
#define	NANO_BLANK				FB_INTEGER	//

#define	BLANK					-1			// void function emulation

//-----------------------------------------------------------------------------
//

void fb_ptr(char* cptr, int64_t iptr);
int64_t native_ptr(const char* cptr);

nanoudr::connection* conn_ptr(const char* cptr);
nanoudr::transaction* tnx_ptr(const char* cptr);
nanoudr::statement* stmt_ptr(const char* cptr);
nanodbc::result* rslt_ptr(const char* cptr);
        
//-----------------------------------------------------------------------------
//

FB_BOOLEAN fb_bool(bool value);
bool native_bool(const ISC_UCHAR value);

//-----------------------------------------------------------------------------
//

#define FB_STRING(fb, s)	\
	(fb).length = (ISC_USHORT)sizeof((fb).str);	\
	ISC_USHORT s_length = (ISC_USHORT)(s).length();	\
	(fb).length = (fb).length <= s_length ? (fb).length : s_length;	\
	memcpy((fb).str, (s).c_str(), (fb).length);	\
	(fb).str[(fb).length] = '\0';	/* FB_STRING */

#define UTF8_IN(param)	\
	if (in_char_sets[in::##param] == fb_char_set::CS_UTF8)	\
	{	\
		utf8_to_loc(in->##param.str, in->##param.str);	\
	}	/* UTF8_IN */  

#define UTF8_OUT(param)	\
	if (out_char_sets[out::##param] == fb_char_set::CS_UTF8)	\
	{	\
		loc_to_utf8(out->##param.str, out->##param.str);	\
	}	/* UTF8_OUT */

//-----------------------------------------------------------------------------
//

extern char udr_locale[20];

void utf8_to_loc(char* dest, const char* src);
void loc_to_utf8(char* dest, const char* src);

enum fb_char_set
{
	CS_NONE = 0,		// No Character Set

	CS_BINARY = 1,		// BINARY BYTES
	CS_ASCII = 2,		// ASCII
	CS_UNICODE_FSS = 3, // UNICODE in FSS format	- 3b
	CS_UTF8 = 4,		// UTF-8	- 4b
	CS_SJIS = 5,		// SJIS		- 2b
	CS_EUCJ = 6,		// EUC-J	- 2b

	CS_JIS_0208 = 7,		// JIS 0208; 1990
	CS_UNICODE_UCS2 = 8,	// UNICODE v 1.10

	CS_DOS_737 = 9,
	CS_DOS_437 = 10,	// DOS CP 437
	CS_DOS_850 = 11,	// DOS CP 850
	CS_DOS_865 = 12,	// DOS CP 865
	CS_DOS_860 = 13,	// DOS CP 860
	CS_DOS_863 = 14,	// DOS CP 863

	CS_DOS_775 = 15,
	CS_DOS_858 = 16,
	CS_DOS_862 = 17,
	CS_DOS_864 = 18,

	CS_NEXT = 19,		// NeXTSTEP OS native charset

	CS_ISO8859_1 = 21,	// ISO-8859.1
	CS_ISO8859_2 = 22,	// ISO-8859.2
	CS_ISO8859_3 = 23,	// ISO-8859.3
	CS_ISO8859_4 = 34,	// ISO-8859.4
	CS_ISO8859_5 = 35,	// ISO-8859.5
	CS_ISO8859_6 = 36,	// ISO-8859.6
	CS_ISO8859_7 = 37,	// ISO-8859.7
	CS_ISO8859_8 = 38,	// ISO-8859.8
	CS_ISO8859_9 = 39,	// ISO-8859.9
	CS_ISO8859_13 = 40,	// ISO-8859.13

	CS_KSC5601 = 44,	// KOREAN STANDARD 5601	- 2b

	CS_DOS_852 = 45,	// DOS CP 852
	CS_DOS_857 = 46,	// DOS CP 857
	CS_DOS_861 = 47,	// DOS CP 861

	CS_DOS_866 = 48,
	CS_DOS_869 = 49,

	CS_CYRL = 50,
	CS_WIN1250 = 51,	// Windows cp 1250
	CS_WIN1251 = 52,	// Windows cp 1251
	CS_WIN1252 = 53,	// Windows cp 1252
	CS_WIN1253 = 54,	// Windows cp 1253
	CS_WIN1254 = 55,	// Windows cp 1254

	CS_BIG5 = 56,		// Big Five unicode cs
	CS_GB2312 = 57,		// GB 2312-80 cs	- 2b

	CS_WIN1255 = 58,	// Windows cp 1255
	CS_WIN1256 = 59,	// Windows cp 1256
	CS_WIN1257 = 60,	// Windows cp 1257

	CS_UTF16 = 61,		// UTF-16
	CS_UTF32 = 62,		// UTF-32

	CS_KOI8R = 63,		// Russian KOI8R
	CS_KOI8U = 64,		// Ukrainian KOI8U

	CS_WIN1258 = 65,	// Windows cp 1258

	CS_TIS620 = 66,		// TIS620
	CS_GBK = 67,		// GBK	- 2b	
	CS_CP943C = 68,		// CP943C	- 2b

	CS_GB18030 = 69		// GB18030	- 4b
};

//-----------------------------------------------------------------------------
//

struct date
{
	unsigned year;  
	unsigned month;
	unsigned day;
};

struct time
{
	unsigned hour;
	unsigned min;
	unsigned sec;
	unsigned fract;
};

struct timestamp
{
	unsigned year;
	unsigned month;
	unsigned day;
	unsigned hour;
	unsigned min;
	unsigned sec;
	unsigned fract;
};

nanodbc::timestamp set_timestamp(nanoudr::timestamp* tm);
nanodbc::date set_date(nanoudr::date* d);
nanodbc::time set_time(nanoudr::time* t);

nanoudr::timestamp get_timestamp(nanodbc::timestamp* tm);
nanoudr::date get_date(nanodbc::date* d);
nanoudr::time get_time(nanodbc::time* t);

} // namespace nanoudr

#endif	/* NANO_H */