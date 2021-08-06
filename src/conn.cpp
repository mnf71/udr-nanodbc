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
 
using namespace Firebird;

//-----------------------------------------------------------------------------
// package nano$conn
//

using namespace nanodbc;

namespace nano
{

char last_error_message[ERROR_MESSAGE_LENGTH] = { 0 };

//-----------------------------------------------------------------------------
// create function connection (
//	 udr_locale varchar(20) character set none not null default '.1251',
//	 attr varchar(512) character set utf8 default null, 
//	 user varchar(63) character set utf8 default null, 
//	 pass varchar(63) character set utf8 default null, 
//	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!conn_connection'
//	engine udr; 
//
// \brief
// connection (?, null, null, null, ...) returns new connection object, initially not connected	
// connection (?, ?, null, null, ...) returns new connection object and immediately connect by SQLDriverConnect
// connection (?, ?, ?, ?, ...) returns new connection object and immediately connect by SQLConnect
//

FB_UDR_BEGIN_FUNCTION(conn_connection)

	unsigned in_count;

	enum in : short {
		udr_locale = 0, attr, user, pass, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(FB_VARCHAR(512 * 4), attr)
		(FB_VARCHAR(20), udr_locale)
		(FB_VARCHAR(63 * 4), user)
		(FB_VARCHAR(63 * 4), pass)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		try
		{
			strncpy_s(
				nano::udr_locale, in->udr_locale.length + 1, in->udr_locale.str, _TRUNCATE
			); // initialize locale

			if (in_char_sets[in::attr] == fb_char_set::CS_UTF8) utf8_to_loc(in->attr.str, in->attr.str);
			if (in_char_sets[in::user] == fb_char_set::CS_UTF8) utf8_to_loc(in->user.str, in->user.str);
			if (in_char_sets[in::pass] == fb_char_set::CS_UTF8) utf8_to_loc(in->pass.str, in->pass.str);

			nanodbc::connection* conn;
			if (in->userNull && in->passNull)
			{
				if (!in->attrNull) 
					conn = new nanodbc::connection(NANODBC_TEXT(in->attr.str), in->timeout);
				else
					conn = new nanodbc::connection();
			}
			else
				conn =
					new nanodbc::connection
						(NANODBC_TEXT(in->attr.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
			nano::fb_ptr(out->conn.str, (int64_t)conn);
			out->connNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			out->connNull = FB_TRUE;
			NANO_THROW_ERROR(e.what());
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dispose (
//	 conn ty$pointer not null, 
//	) returns ty$pointer
//	external name 'nano!conn_dispose'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_dispose)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull) 
		{
			try
			{
				delete nano::conn_ptr(in->conn.str);
				out->connNull = FB_TRUE;
			}
			catch (std::runtime_error const& e)
			{
				nano::fb_ptr(out->conn.str, nano::native_ptr(in->conn.str));
				out->connNull = FB_FALSE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->connNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function allocate (
//	 conn ty$pointer not null, 
//	) returns ty$nano_blank
//	external name 'nano!conn_allocate'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_allocate)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull) 
		{
			out->blank = BLANK;
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				conn->allocate();
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function deallocate (
//	 conn ty$pointer not null, 
//	) returns ty$nano_blank
//	external name 'nano!conn_deallocate'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_deallocate)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			out->blank = BLANK;
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				conn->deallocate();
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connect (
//	 conn ty$pointer not null, 
//	 attr varchar(512) character set utf8 not null, 
//	 user varchar(64) character set utf8 default null, 
// 	 pass varchar(64) character set utf8 default null, 
//	 timeout integer not null default = 0
//	) returns ty$nano_blank
//	external name 'nano!conn_connect'
//	engine udr; 
//
// \brief
// connect (?, ?, null, null, ...) returns blank and connect to the given data source by SQLDriverConnect
// connect (?, ?, ?, ?, ...) returns blank and connect to the given data source by by SQLConnect
//

FB_UDR_BEGIN_FUNCTION(conn_connect)

	unsigned in_count;

	enum in : short {
		conn = 0, attr, user, pass, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
		(FB_VARCHAR(512 * 4), attr)
		(FB_VARCHAR(63 * 4), user)
		(FB_VARCHAR(63 * 4), pass)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			out->blank = BLANK;

			if (in_char_sets[in::attr] == fb_char_set::CS_UTF8) utf8_to_loc(in->attr.str, in->attr.str);
			if (in_char_sets[in::user] == fb_char_set::CS_UTF8) utf8_to_loc(in->user.str, in->user.str);
			if (in_char_sets[in::pass] == fb_char_set::CS_UTF8) utf8_to_loc(in->pass.str, in->pass.str);

			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				if (in->userNull && in->passNull)
					conn->connect(NANODBC_TEXT(in->attr.str), in->timeout);
				else
					conn->connect
						(NANODBC_TEXT(in->attr.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	 conn ty$pointer not null, 
//	) returns boolean
//	external name 'nano!conn_connected'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_connected)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, connected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				out->connected = nano::fb_bool(conn->connected());
				out->connectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->connectedNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->connectedNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function disconnect (
//	 conn ty$pointer not null, 
//	) returns ty$nano_blank
//	external name 'nano!conn_disconnect'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_disconnect)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			out->blank = BLANK;
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				conn->disconnect();
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function transactions (
//	 conn ty$pointer not null, 
//	) returns integer
//	external name 'nano!conn_transactions'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_transactions)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, transactions)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				out->transactions = (ISC_LONG)conn->transactions();
				out->transactionsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->transactionsNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->transactionsNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function get_info (
//	 conn ty$pointer not null, 
//	 info_type smallint not null
//	) returns varchar(256) character set utf8
//	external name 'nano!conn_get_info'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_get_info)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
		(FB_SMALLINT, info_type)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(256 * 4), info)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				string info = conn->get_info<string>(in->info_type);
				out->info.length =
					(ISC_USHORT) info.length() < (ISC_USHORT) sizeof(out->info.str) ?
						(ISC_USHORT) info.length() : (ISC_USHORT) sizeof(out->info.str);
				memcpy(out->info.str, info.c_str(), out->info.length);
				out->infoNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->infoNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->infoNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dbms_name (
//	 conn ty$pointer not null, 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_dbms_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_dbms_name)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				string name = conn->dbms_name();
				out->name.length =
					(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->name.str) ?
						(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->name.str);
				memcpy(out->name.str, name.c_str(), out->name.length);
				out->nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->nameNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dbms_version (
//	 conn ty$pointer not null, 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_dbms_version'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_dbms_version)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), version)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				string version = conn->dbms_version();
				out->version.length =
					(ISC_USHORT)version.length() < (ISC_USHORT)sizeof(out->version.str) ?
						(ISC_USHORT)version.length() : (ISC_USHORT)sizeof(out->version.str);
				memcpy(out->version.str, version.c_str(), out->version.length);
				out->versionNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->versionNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->versionNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function driver_name (
//	 conn ty$pointer not null, 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_driver_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_driver_name)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				string name = conn->driver_name();
				out->name.length =
					(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->name.str) ?
						(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->name.str);
				memcpy(out->name.str, name.c_str(), out->name.length);
				out->nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->nameNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function database_name (
//	 conn ty$pointer not null, 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_database_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_database_name)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				string name = conn->database_name();
				out->name.length =
					(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->name.str) ?
						(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->name.str);
				memcpy(out->name.str, name.c_str(), out->name.length);
				out->nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->nameNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function catalog_name (
//	 conn ty$pointer not null, 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_catalog_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_catalog_name)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				string name = conn->catalog_name();
				out->name.length =
					(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->name.str) ?
						(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->name.str);
				memcpy(out->name.str, name.c_str(), out->name.length);
				out->nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->nameNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function e_message 
//	returns varchar(512) character set utf8
//	external name 'nano!conn_e_message'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_e_message)
	
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
		out->e_msgNull = FB_FALSE;
		out->e_msg.length = (ISC_SHORT)strlen(nano::last_error_message);
		memcpy(out->e_msg.str, nano::last_error_message, out->e_msg.length);
		out->e_msg.str[out->e_msg.length] = '\0';
		if (out_char_sets[out::e_msg] == fb_char_set::CS_UTF8)
			loc_to_utf8(out->e_msg.str, out->e_msg.str);
	}

FB_UDR_END_FUNCTION

} // namespace nano