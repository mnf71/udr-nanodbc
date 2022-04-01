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

connection::connection(class attachment_resources& att_resources) : nanodbc::connection()
{
	att_resources_ = &att_resources;
	att_resources_->connections.retain(this);
}

connection::connection(
	class attachment_resources& att_resources, const nanodbc::string& dsn, const nanodbc::string& user, const nanodbc::string& pass, long timeout)
	: nanodbc::connection(dsn, user, pass, timeout)
{
	att_resources_ = &att_resources;
	att_resources_->connections.retain(this);
}

connection::connection(class attachment_resources& att_resources, const nanodbc::string& connection_string, long timeout) 
	: nanodbc::connection(connection_string, timeout)
{
	att_resources_ = &att_resources;
	att_resources_->connections.retain(this);
}

connection::~connection()
{
	att_resources_->connections.release(this);
}

//-----------------------------------------------------------------------------
// create function connection (
//	attr varchar(512) character set none [utf8] default null, 
//	user_ varchar(128) character set none [utf8] default null, 
//	pass varchar(128) character set none [utf8] default null, 
//	timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!conn$connection'
//	engine udr; 
//
// connection (null, null, null, ...) returns new connection object, initially not connected	
// connection (?, null, null, ...) returns new connection object and immediately connect by SQLDriverConnect
// connection (?, ?, ?, ...) returns new connection object and immediately connect by SQLConnect
//

FB_UDR_BEGIN_FUNCTION(conn$connection)
	
	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		attr = 0, user, pass, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(FB_VARCHAR(128 * 4), user)
		(FB_VARCHAR(128 * 4), pass)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->connNull = FB_TRUE;
		try
		{
			U8_VARIYNG(in, attr)
			U8_VARIYNG(in, user)
			U8_VARIYNG(in, pass)
			nanoudr::connection* conn;
			if (in->userNull && in->passNull)
			{
				if (!in->attrNull)
					conn = new nanoudr::connection(*att_resources, NANODBC_TEXT(in->attr.str), in->timeout);
				else
					conn = new nanoudr::connection(*att_resources);
			}
			else
				conn =
					new nanoudr::connection(*att_resources, NANODBC_TEXT(in->attr.str), NANODBC_TEXT(in->user.str),
						NANODBC_TEXT(in->pass.str), in->timeout);
			helper.fb_ptr(out->conn.str, conn);
			out->connNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			NANODBC_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function valid (
//	conn ty$pointer not null, 
//	) returns boolean
//	external name 'nano!conn$valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$valid)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->valid = helper.fb_bool(
			in->connNull ? false :
				att_resources->connections.valid(helper.native_ptr<connection>(in->conn.str))
			);
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	conn ty$pointer not null, 
//	) returns ty$pointer
//	external name 'nano!conn$release'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$release)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->connNull = FB_TRUE;
		if (!in->connNull)
		{
			nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
			try
			{
				if (att_resources->connections.valid(conn)) delete (nanoudr::connection*)(conn);
			}
			catch (std::runtime_error const& e)
			{
				helper.fb_ptr(out->conn.str, conn);
				out->connNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function expunge (
//	conn ty$pointer not null, 
//	) returns ty$nano_blank
//	external name 'nano!conn$expunge'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$expunge)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				att_resources->connections.expunge(conn);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function allocate (
//	conn ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!conn$allocate'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$allocate)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				conn->allocate();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function deallocate (
//	conn ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!conn$deallocate'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$deallocate)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				conn->deallocate();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function isolation_level (
//	tnx ty$pointer not null, 
//	level_ smallint default null 
//	) returns smallint
//	external name 'nano!conn$isolation_level'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$isolation_level)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
		(FB_SMALLINT, level)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, isolation_level)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->isolation_levelNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				if (!in->levelNull)	conn->isolation_level(static_cast<isolation_state>(in->level));
				out->isolation_level = conn->isolation_level();
				out->isolation_levelNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connect_ (
//	conn ty$pointer not null, 
//	attr varchar(512) character set none [utf8] not null, 
//	user varchar(128) character set none [utf8] default null, 
//	pass varchar(128) character set none [utf8] default null, 
//	timeout integer not null default = 0
// 	) returns ty$nano_blank
// 	external name 'nano!conn$connect'
// 	engine udr; 
//
// connect (?, ?, null, null, ...) returns blank and connect to the given data source by SQLDriverConnect
// connect (?, ?, ?, ?, ...) returns blank and connect to the given data source by by SQLConnect
//

FB_UDR_BEGIN_FUNCTION(conn$connect)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		conn = 0, attr, user, pass, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(FB_VARCHAR(128 * 4), user)
		(FB_VARCHAR(128 * 4), pass)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			U8_VARIYNG(in, attr)
			U8_VARIYNG(in, user)
			U8_VARIYNG(in, pass)
			try
			{
				if (in->userNull && in->passNull)
					conn->connect(NANODBC_TEXT(in->attr.str), in->timeout);
				else
					conn->connect
					(NANODBC_TEXT(in->attr.str), NANODBC_TEXT(in->user.str), NANODBC_TEXT(in->pass.str), in->timeout);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	conn ty$pointer not null 
//	) returns boolean
//	external name 'nano!conn$connected'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$connected)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->connectedNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				out->connected = helper.fb_bool(conn->connected());
				out->connectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function disconnect_ (
//	conn ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!conn$disconnect'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$disconnect)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				conn->disconnect();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function transactions (
//	conn ty$pointer not null 
//	) returns integer
//	external name 'nano!conn$transactions'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$transactions)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->transactionsNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				out->transactions = static_cast<ISC_LONG>(conn->transactions());
				out->transactionsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function get_info (
//	conn ty$pointer not null, 
//	info_type smallint not null
//	) returns varchar(256) character set none [utf8]
//	external name 'nano!conn$get_info'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$get_info)

	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		info = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		FUNCTION_RESOURCES
		out->infoNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanodbc::string info = conn->get_info<nanodbc::string>(in->info_type);
				FB_VARIYNG(out->info, info)
				U8_VARIYNG(out, info)
				out->infoNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dbms_name (
//	conn ty$pointer not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!conn$dbms_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$dbms_name)

	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		dbms_name = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		FUNCTION_RESOURCES
		out->dbms_nameNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanodbc::string dbms_name = conn->dbms_name();
				FB_VARIYNG(out->dbms_name, dbms_name)
				U8_VARIYNG(out, dbms_name)
				out->dbms_nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dbms_version (
//	conn ty$pointer not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!conn$dbms_version'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$dbms_version)
	
	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		version = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		FUNCTION_RESOURCES
		out->versionNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanodbc::string dbms_version = conn->dbms_version();
				FB_VARIYNG(out->version, dbms_version)
				U8_VARIYNG(out, version)
				out->versionNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function driver_name (
//	conn ty$pointer not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!conn$driver_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$driver_name)

	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		drv_name = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		FUNCTION_RESOURCES
		out->drv_nameNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanodbc::string driver_name = conn->driver_name();
				FB_VARIYNG(out->drv_name, driver_name)
				U8_VARIYNG(out, drv_name)
				out->drv_nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function database_name (
//	conn ty$pointer not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!conn$database_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$database_name)

	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		db_name = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		FUNCTION_RESOURCES
		out->db_nameNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanodbc::string db_name = conn->database_name();
				FB_VARIYNG(out->db_name, db_name)
				U8_VARIYNG(out, db_name)
				out->db_nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function catalog_name (
//	conn ty$pointer not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!conn$catalog_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(conn$catalog_name)

	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		ctlg_name = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		FUNCTION_RESOURCES
		out->ctlg_nameNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanodbc::string ctlg_name = conn->catalog_name();
				FB_VARIYNG(out->ctlg_name, ctlg_name)
				U8_VARIYNG(out, ctlg_name)
				out->ctlg_nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr