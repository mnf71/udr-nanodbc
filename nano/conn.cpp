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
 
using namespace nanodbc;
using namespace Firebird;

namespace nano
{
	//-----------------------------------------------------------------------------
	// create nano$conn.connection
	//	returns ty$pointer
	//	external name 'nano!conn_connection'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_connection)

		FB_UDR_MESSAGE(
			OutMessage,
			(NANO_POINTER, conn)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			try
			{
				nanodbc::connection* conn = new nanodbc::connection();
				nano::fbPtr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
			}
			catch (...)
			{
				nano::fbPtr(out->conn.str, 0L);
				out->connNull = FB_TRUE;
				throw;
			}
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.sql_connection (
	//	 dsn varchar(512) character set utf8 not null , 
	//	 user varchar(63) character set utf8 not null , 
	//	 pass varchar(63) character set utf8 not null, 
	//	 timeout integer not null default = 0 
	//	) returns ty$pointer
	//	external name 'nano!conn_sql_connection'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_sql_connection)

		FB_UDR_MESSAGE(
			InMessage,
			(FB_VARCHAR(512 * 4), dsn)
			(FB_VARCHAR(63 * 4), user) // FB 4.0 future  
			(FB_VARCHAR(63 * 4), pass) // 
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
				nanodbc::connection* conn =
					new nanodbc::connection
						(NANODBC_TEXT(in->dsn.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
				nano::fbPtr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
			}
			catch (...)
			{
				nano::fbPtr(out->conn.str, 0L);
				out->connNull = FB_TRUE;
				throw;
			}
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.driver_connection (
	//	 connection_string varchar(512) character set utf8 not null,  
	//	 timeout integer not null default = 0
	//	) returns ty$pointer
	//	external name 'nano!conn_driver_connect'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_driver_connection)

		FB_UDR_MESSAGE(
			InMessage,
			(FB_VARCHAR(512 * 4), connection_string)
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
				nanodbc::connection* conn =
					new nanodbc::connection(NANODBC_TEXT(in->connection_string.str), in->timeout);
				nano::fbPtr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
			}
			catch (...)
			{
				nano::fbPtr(out->conn.str, 0L);
				out->connNull = FB_TRUE;
				throw;
			}
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.dispose (
	//	 conn ty$pointer, 
	//	) returns ty$pointer
	//	external name 'nano!conn_dispose'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_Dispose)

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
			if (in->connNull == FB_FALSE)
			{
				try
				{
					delete nano::connPtr(in->conn.str);
					nano::fbPtr(out->conn.str, 0L);
					out->connNull = FB_TRUE;
				}
				catch (...)
				{
					nano::fbPtr(out->conn.str, nano::nativePtr(in->conn.str));
					out->connNull = FB_FALSE;
					throw;
				}
			}
			else 
				out->connNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.allocate (
	//	 conn ty$pointer, 
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
			(FB_SMALLINT, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				out->rslt = BLANK_RESULT;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					conn->allocate();
				}
				catch (...)
				{
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.deallocate (
	//	 conn ty$pointer, 
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
			(NANO_BLANK, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				out->rslt = BLANK_RESULT;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					conn->deallocate();
				}
				catch (...)
				{
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.sql_connect (
	//	 conn ty$pointer, 
	//	 dsn varchar(512) character set utf8 not null, 
	//	 user varchar(64) character set utf8 not null, 
	//	 pass varchar(64) character set utf8 not null, 
	//	 timeout integer not null default = 0
	//	) returns ty$nano_blank
	//	external name 'nano!conn_sql_connect'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_sql_connect)

		FB_UDR_MESSAGE(
			InMessage,
			(NANO_POINTER, conn)
			(FB_VARCHAR(512 * 4), dsn)
			(FB_VARCHAR(63 * 4), user)
			(FB_VARCHAR(63 * 4), pass)
			(FB_INTEGER, timeout)
		);

		FB_UDR_MESSAGE(
			OutMessage,
			(NANO_BLANK, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				out->rslt = BLANK_RESULT;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					conn->connect
						(NANODBC_TEXT(in->dsn.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
				}
				catch (...)
				{
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.driver_connect (
	//	 conn ty$pointer, 
	//	 connection_string varchar(512) character set utf8 not null, 
	//	 timeout integer not null default = 0
	//	) returns ty$nano_blank
	//	external name 'nano!conn_driver_connect'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_driver_connect)

		FB_UDR_MESSAGE(
			InMessage,
			(NANO_POINTER, conn)
			(FB_VARCHAR(512 * 4), connection_string)
			(FB_INTEGER, timeout)
		);

		FB_UDR_MESSAGE(
			OutMessage,
			(NANO_BLANK, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				out->rslt = BLANK_RESULT;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					conn->connect(NANODBC_TEXT(in->connection_string.str), in->timeout);
				}
				catch (...)
				{
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.connected (
	//	 conn ty$pointer, 
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
			(FB_BOOLEAN, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					out->rslt = nano::fbBool(conn->connected());
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.disconnect (
	//	 conn ty$pointer, 
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
			(NANO_BLANK, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				out->rslt = BLANK_RESULT;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					conn->disconnect();
				}
				catch (...)
				{
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.transactions (
	//	 conn ty$pointer, 
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
			(FB_INTEGER, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				out->rslt = -1;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					out->rslt = (ISC_LONG)conn->transactions();
				}
				catch (...)
				{
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.get_info (
	//	 conn ty$pointer, 
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
			(FB_VARCHAR(256 * 4), rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				memset(out->rslt.str, 0, sizeof(out->rslt.str));
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					string info = conn->get_info<string>(in->info_type);
					out->rslt.length = 
						(ISC_USHORT) info.length() < (ISC_USHORT) sizeof(out->rslt.str) ? 
							(ISC_USHORT) info.length() : (ISC_USHORT) sizeof(out->rslt.str);
					memcpy(out->rslt.str, info.c_str(), out->rslt.length);
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.dbms_name (
	//	 conn ty$pointer, 
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
			(FB_VARCHAR(128 * 4), rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				memset(out->rslt.str, 0, sizeof(out->rslt.str));
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					string name = conn->dbms_name();
					out->rslt.length =
						(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->rslt.str) ?
							(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->rslt.str);
					memcpy(out->rslt.str, name.c_str(), out->rslt.length);
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.dbms_version (
	//	 conn ty$pointer, 
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
			(FB_VARCHAR(128 * 4), rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				memset(out->rslt.str, 0, sizeof(out->rslt.str));
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					string version = conn->dbms_version();
					out->rslt.length =
						(ISC_USHORT)version.length() < (ISC_USHORT)sizeof(out->rslt.str) ?
							(ISC_USHORT)version.length() : (ISC_USHORT)sizeof(out->rslt.str);
					memcpy(out->rslt.str, version.c_str(), out->rslt.length);
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.driver_name (
	//	 conn ty$pointer, 
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
			(FB_VARCHAR(128 * 4), rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				memset(out->rslt.str, 0, sizeof(out->rslt.str));
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					string name = conn->driver_name();
					out->rslt.length =
						(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->rslt.str) ?
							(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->rslt.str);
					memcpy(out->rslt.str, name.c_str(), out->rslt.length);
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.database_name (
	//	 conn ty$pointer, 
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
			(FB_VARCHAR(128 * 4), rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				memset(out->rslt.str, 0, sizeof(out->rslt.str));
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					string name = conn->database_name();
					out->rslt.length =
						(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->rslt.str) ?
							(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->rslt.str);
					memcpy(out->rslt.str, name.c_str(), out->rslt.length);
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create nano$conn.catalog_name (
	//	 conn ty$pointer, 
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
			(FB_VARCHAR(128 * 4), rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->connNull == FB_FALSE)
			{
				memset(out->rslt.str, 0, sizeof(out->rslt.str));
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					string name = conn->catalog_name();
					out->rslt.length =
						(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->rslt.str) ?
							(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->rslt.str);
					memcpy(out->rslt.str, name.c_str(), out->rslt.length);
					out->rsltNull = FB_FALSE;
				}
				catch (...)
				{
					out->rsltNull = FB_TRUE;
					throw;
				}
			}
			else
				out->rsltNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

} // namespace nano