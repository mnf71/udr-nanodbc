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

//-----------------------------------------------------------------------------
// create function connection (
//	 attr varchar(512) character set utf8 default null, 
//	 user varchar(63) character set utf8 default null, 
//	 pass varchar(63) character set utf8 default null, 
//	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!conn_connection'
//	engine udr; 
//
// \brief
// connection (null, null, null, ...) returns new connection object, initially not connected	
// connection (?, null, null, ...) returns new connection object and immediately connect by SQLDriverConnect
// connection (?, ?, ?, ...) returns new connection object and immediately connect by SQLConnect
//

FB_UDR_BEGIN_FUNCTION(conn_connection)

	FB_UDR_MESSAGE(
		InMessage,
		(FB_VARCHAR(512 * 4), attr)
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
		catch (...)
		{
			out->connNull = FB_TRUE;
			throw;
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
			catch (...)
			{
				nano::fb_ptr(out->conn.str, nano::native_ptr(in->conn.str));
				out->connNull = FB_FALSE;
				throw;
			}
		}
		else
		{
			out->connNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->blankNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->blankNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			throw conn_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connect (
//	 conn ty$pointer not null, 
//	 attr varchar(512) character set utf8 not null, 
//	 user varchar(64) character set utf8 default null, 
//	 pass varchar(64) character set utf8 default null, 
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
			catch (...)
			{
				out->blankNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->connectedNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->connectedNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->blankNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->blankNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->transactionsNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->transactionsNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->infoNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->infoNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->nameNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->versionNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->versionNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->nameNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->nameNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			throw conn_POINTER_INVALID;
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
			catch (...)
			{
				out->nameNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->nameNull = FB_TRUE;
			throw conn_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

} // namespace nano