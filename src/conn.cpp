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

//-----------------------------------------------------------------------------
// package nano$conn
//

namespace nanoudr
{

//-----------------------------------------------------------------------------
// UDR Connection class implementation
//

connection::connection() : nanodbc::connection() 
{
	udr_resources.connections.retain(this);
}

connection::connection(
	const nanodbc::string& dsn, const nanodbc::string& user, const nanodbc::string& pass, long timeout) 
	: nanodbc::connection(dsn, user, pass, timeout)
{
	udr_resources.connections.retain(this);
}

connection::connection(const nanodbc::string& connection_string, long timeout)
	: nanodbc::connection(connection_string, timeout)
{
	udr_resources.connections.retain(this);
}

connection::~connection()
{
	nanodbc::connection::~connection();
}

//-----------------------------------------------------------------------------
// create function connection (
//	 attr varchar(512) character set utf8 default null, 
//	 user_ varchar(63) character set utf8 default null, 
//	 pass varchar(63) character set utf8 default null, 
//	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!conn_connection'
//	engine udr; 
//
// connection (null, null, null, ...) returns new connection object, initially not connected	
// connection (?, null, null, ...) returns new connection object and immediately connect by SQLDriverConnect
// connection (?, ?, ?, ...) returns new connection object and immediately connect by SQLConnect
//

FB_UDR_BEGIN_FUNCTION(conn_connection)

	unsigned in_count;

	enum in : short {
		attr = 0, user, pass, timeout
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
		out->connNull = FB_TRUE;

		if (!udr_resources.ready())
		try
		{
			udr_resources.initialize(status, context);
		}
		catch (std::runtime_error const& e)
		{
			ANY_THROW(e.what())
		}
		
		try
		{
			U8_VARIYNG(in, attr);
			U8_VARIYNG(in, user);
			U8_VARIYNG(in, pass);
			nanoudr::connection* conn;
			if (in->userNull && in->passNull)
			{
				if (!in->attrNull) 
					conn = new nanoudr::connection(NANODBC_TEXT(in->attr.str), in->timeout);
				else
					conn = new nanoudr::connection();
			}
			else
				conn =
					new nanoudr::connection
						(NANODBC_TEXT(in->attr.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
			udr_helper.fb_ptr(out->conn.str, (int64_t)conn);
			out->connNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			NANODBC_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function expunge (
//	 conn ty$pointer not null, 
//	) returns ty$nano_blank
//	external name 'nano!conn_expunge'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_expunge)

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
		out->blankNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					udr_resources.connections.expunge(conn);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	 conn ty$pointer not null, 
//	) returns ty$pointer
//	external name 'nano!conn_release'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_release)

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
		out->connNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				udr_resources.connections.release(conn);
			}
			catch (std::runtime_error const& e)
			{
				udr_helper.fb_ptr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_valid (
//	 conn ty$pointer not null, 
//	) returns boolean
//	external name 'nano!conn_is_valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_is_valid)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, valid)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->valid =
			in->connNull ?
			udr_helper.fb_bool(false) :
			udr_resources.connections.is_valid(udr_helper.conn_ptr(in->conn.str));
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function allocate (
//	 conn ty$pointer not null 
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
		out->blankNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					conn->allocate();
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function deallocate (
//	 conn ty$pointer not null 
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
		out->blankNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					conn->deallocate();
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connect_ (
//	 conn ty$pointer not null, 
//	 attr varchar(512) character set utf8 not null, 
//	 user varchar(64) character set utf8 default null, 
// 	 pass varchar(64) character set utf8 default null, 
//	 timeout integer not null default = 0
//	) returns ty$nano_blank
//	external name 'nano!conn_connect'
//	engine udr; 
//
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
		out->blankNull = FB_TRUE;
		if (!in->connNull)
		{
			U8_VARIYNG(in, attr);
			U8_VARIYNG(in, user);
			U8_VARIYNG(in, pass);
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					if (in->userNull && in->passNull)
						conn->connect(NANODBC_TEXT(in->attr.str), in->timeout);
					else
						conn->connect
						(NANODBC_TEXT(in->attr.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	 conn ty$pointer not null 
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
		out->connectedNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					out->connected = udr_helper.fb_bool(conn->connected());
					out->connectedNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function disconnect_ (
//	 conn ty$pointer not null 
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
		out->blankNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					conn->disconnect();
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function transactions (
//	 conn ty$pointer not null 
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
		out->transactionsNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					out->transactions = (ISC_LONG)conn->transactions();
					out->transactionsNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
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

unsigned out_count;

	enum out : short {
		info = 0
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
		out->infoNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					nanodbc::string info = conn->get_info<nanodbc::string>(in->info_type);
					FB_VARIYNG(out->info, info);
					U8_VARIYNG(out, info);
					out->infoNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dbms_name (
//	 conn ty$pointer not null 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_dbms_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_dbms_name)

	unsigned out_count;

	enum out : short {
		dbms_name = 0
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
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), dbms_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->dbms_nameNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					nanodbc::string dbms_name = conn->dbms_name();
					FB_VARIYNG(out->dbms_name, dbms_name);
					U8_VARIYNG(out, dbms_name);
					out->dbms_nameNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dbms_version (
//	 conn ty$pointer not null 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_dbms_version'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_dbms_version)
	
	unsigned out_count;

	enum out : short {
		version = 0
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
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), version)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->versionNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					nanodbc::string dbms_version = conn->dbms_version();
					FB_VARIYNG(out->version, dbms_version);
					U8_VARIYNG(out, version);
					out->versionNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function driver_name (
//	 conn ty$pointer not null 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_driver_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_driver_name)

	unsigned out_count;

	enum out : short {
		drv_name = 0
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
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), drv_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->drv_nameNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					nanodbc::string driver_name = conn->driver_name();
					FB_VARIYNG(out->drv_name, driver_name);
					U8_VARIYNG(out, drv_name);
					out->drv_nameNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function database_name (
//	 conn ty$pointer not null 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_database_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_database_name)

	unsigned out_count;

	enum out : short {
		db_name = 0
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
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), db_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->db_nameNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					nanodbc::string db_name = conn->database_name();
					FB_VARIYNG(out->db_name, db_name);
					U8_VARIYNG(out, db_name);
					out->db_nameNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function catalog_name (
//	 conn ty$pointer not null 
//	) returns varchar(128) character set utf8
//	external name 'nano!conn_catalog_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn_catalog_name)

	unsigned out_count;

	enum out : short {
		ctlg_name = 0
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
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), ctlg_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->ctlg_nameNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (udr_resources.connections.is_valid(conn))
				{
					nanodbc::string ctlg_name = conn->catalog_name();
					FB_VARIYNG(out->ctlg_name, ctlg_name);
					U8_VARIYNG(out, ctlg_name);
					out->ctlg_nameNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONN_POINTER)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr