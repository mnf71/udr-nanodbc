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
// Convenience functions
// package nano$func
//

namespace nanoudr
{

//-----------------------------------------------------------------------------
// todo: 
//	create procedure list_datasources - ?
//

//-----------------------------------------------------------------------------
// todo: 
//	create procedure list_drivers - ?
//

//-----------------------------------------------------------------------------
// create function execute_conn (
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1 
// 	 timeout integer not null default 0, 
//	) returns ty$pointer
//	external name 'nano!func_execute_conn'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_execute_conn)

	nanoudr::attachment_resources* att_resources;

	unsigned in_count;

	enum in : short {
		conn = 0, query, batch_operations, timeout
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
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->rsltNull = FB_TRUE;
		nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
		if (!in->connNull && att_resources->connections.is_valid(conn))
		{
			try
			{
				U8_VARIYNG(in, query);
				nanodbc::result rslt = 
					nanodbc::execute(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
				udr_helper.fb_ptr(out->rslt.str, (int64_t)(new nanoudr::result(*conn, std::move(rslt))));
				out->rsltNull = FB_FALSE;
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
// create function just_execute_conn (
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!func_just_execute_conn'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_just_execute_conn)

	nanoudr::attachment_resources* att_resources;
	
	unsigned in_count;

	enum in : short {
		conn = 0, query, batch_operations, timeout
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
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
		if (!in->connNull && att_resources->connections.is_valid(conn))
		{
			try
			{
				U8_VARIYNG(in, query);
				nanodbc::just_execute
					(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
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
// create function execute_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$pointer
//	external name 'nano!func_execute_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_execute_stmt)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.is_valid(stmt))
		{
			try
			{
				nanodbc::result rslt = nanodbc::execute(*stmt, in->batch_operations);
				udr_helper.fb_ptr(
					out->rslt.str, (int64_t)(new nanoudr::result(*stmt->connection(), std::move(rslt))));
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!func_just_execute_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_just_execute_stmt)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.is_valid(stmt))
		{
			nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
			try
			{
				nanodbc::just_execute(*stmt, in->batch_operations);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function transact_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$pointer
//	external name 'nano!func_transact_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_transact_stmt)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.is_valid(stmt))
		{
			try
			{
				nanodbc::result rslt = nanodbc::transact(*stmt, in->batch_operations);
				udr_helper.fb_ptr(out->rslt.str, (int64_t)&rslt);
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_transact_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!func_just_transact_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_just_transact_stmt)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.is_valid(stmt))
		{
			try
			{
				nanodbc::just_transact(*stmt, in->batch_operations);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare_stmt (
//	 stmt ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!func_prepare_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_prepare_stmt)

	nanoudr::attachment_resources* att_resources;

	unsigned in_count;

	enum in : short {
		stmt = 0, query, timeout
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
		(NANO_POINTER, stmt)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.is_valid(stmt))
		{
			try
			{
				U8_VARIYNG(in, query);
				nanodbc::prepare(*stmt, (NANODBC_TEXT(in->query.str)), in->timeout);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr

