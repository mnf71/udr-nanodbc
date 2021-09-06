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
// package nano$stmt
//

using namespace nanodbc;

#include <variant>

#include "stmt.h"

namespace nano
{

//-----------------------------------------------------------------------------
// create function statement_ (
//	 conn ty$pointer default null, 
//   query varchar(8191) character set utf8 default null,
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt_statement'
//	engine udr; 
//
// \brief
// statement (null, null, ...) returns new un-prepared statement
// statement (?, null, ...) returns statement object and associates it to the given connection
// statement (?, ?, ...) returns prepared a statement using the given connection and query
//

FB_UDR_BEGIN_FUNCTION(stmt_statement)

	unsigned in_count;

	enum in : short {
		conn = 0, query, timeout
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
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		try
		{
			nano::statement* stmt;
			if (!in->connNull)
			{
				nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
				if (!in->queryNull)
				{
					UTF8_IN(query);
					stmt = new nano::statement(*conn, NANODBC_TEXT(in->query.str), in->timeout);
				}
				else
					stmt = new nano::statement(*conn);
			}
			else
				stmt = new nano::statement();
			nano::fb_ptr(out->stmt.str, (int64_t)stmt);
			out->stmtNull = FB_FALSE;
		}	
		catch (std::runtime_error const& e)
		{
			out->stmtNull = FB_TRUE;
			NANO_THROW_ERROR(e.what());
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dispose (
//	 stmt ty$pointer not null 
// ) returns ty$pointer
// external name 'nano!stmt_dispose'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_dispose)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			try
			{
				delete nano::stmt_ptr(in->stmt.str);
				out->stmtNull = FB_TRUE;
			}
			catch (std::runtime_error const& e)
			{
				nano::fb_ptr(out->stmt.str, nano::native_ptr(in->stmt.str));
				out->stmtNull = FB_FALSE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->stmtNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}
		
FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function open_ (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null
// ) returns ty$nano_blank
// external name 'nano!stmt_open'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_open)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();

			if (in->connNull) { NANO_THROW_ERROR(INVALID_CONN_POINTER); }
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				impl->open(*conn);
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function opened (
//	 stmt ty$pointer not null 
//	) returns boolean
//	external name 'nano!stmt_opened'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_opened)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, opened)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				out->opened = nano::fb_bool(impl->open());
				out->openedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->openedNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->openedNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	 stmt ty$pointer not null 
//	) returns boolean
//	external name 'nano!stmt_connected'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_connected)

FB_UDR_MESSAGE(
	InMessage,
	(NANO_POINTER, stmt)
);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, connected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				out->connected = nano::fb_bool(impl->connected());
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
			NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 stmt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!stmt_connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_connection)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				nanodbc::connection conn = impl->connection();
				nano::fb_ptr(out->conn.str, (int64_t)&conn);
				out->connNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->connNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->connNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function close_ (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt_close'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_close)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nano::statement* stmt = nano::stmt_ptr(in->stmt.str);
			nanodbc::statement* impl = stmt->impl();
			try
			{
				if (stmt->batch_array != nullptr) 
					stmt->batch_array->clear();
				impl->close();
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function cancel_ (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt_cancel'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_cancel)

FB_UDR_MESSAGE(
	InMessage,
	(NANO_POINTER, stmt)
);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				impl->cancel();
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare_ (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer, 
//   query varchar(8191) character set utf8 not null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_prepare'
//	engine udr; 
//
// \brief
// prepare (?, ?, ?, ...) returns blank and prepares the given statement to execute its associated connection
// prepare (?, null, ?, ...) returns blank and opens and prepares the given statement to execute on the given connection
//

FB_UDR_BEGIN_FUNCTION(stmt_prepare)
	
	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, timeout
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
		(NANO_POINTER, conn)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nano::statement* stmt = nano::stmt_ptr(in->stmt.str);
			nanodbc::statement* impl = stmt->impl();
			try
			{
				if (stmt->batch_array != nullptr)
				{
					delete stmt->batch_array;
					stmt->batch_array = nullptr;
				}

				UTF8_IN(query);
				if (!in->connNull)
				{
					nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
					impl->prepare(*conn, NANODBC_TEXT(in->query.str), in->timeout);
				}
				else
					impl->prepare(NANODBC_TEXT(in->query.str), in->timeout);
	
				stmt->batch_array = new nano::params_array(impl->parameters());
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function timeout (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_timeout'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_timeout)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				impl->timeout(in->timeout);
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function execute_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1,
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt_execute_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_execute_direct)

	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, batch_operations, timeout
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
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			if (in->connNull) { NANO_THROW_ERROR(INVALID_CONN_POINTER); }
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				UTF8_IN(query);
				nanodbc::result rslt =
					impl->execute_direct(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
				nano::fb_ptr(out->rslt.str, (int64_t)&rslt);
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->rsltNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->rsltNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_just_execute_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_just_execute_direct)

	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, batch_operations, timeout
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
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			if (in->connNull) { NANO_THROW_ERROR(INVALID_CONN_POINTER); }
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				UTF8_IN(query);
				impl->just_execute_direct(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function execute_ (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1, 
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt_execute'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_execute)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				nanodbc::result rslt = impl->execute(in->batch_operations, in->timeout);
				nano::fb_ptr(out->rslt.str, (int64_t)&rslt);
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->rsltNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->rsltNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_ (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_just_execute'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_just_execute)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				impl->just_execute(in->batch_operations, in->timeout);
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function procedure_columns (
//	 stmt ty$pointer not null, 
//	 catalog_ varchar(128) character set utf8 not null, 
//	 schema_ varchar(128) character set utf8 not null, 
//	 procedure_ varchar(63) character set utf8 not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns ty$pointer
//	external name 'nano!stmt_procedure_columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_procedure_columns)

unsigned in_count;

	enum in : short {
		stmt = 0, catalog_, schema_, procedure_, column_
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
		(FB_VARCHAR(128 * 4), catalog_)
		(FB_VARCHAR(128 * 4), schema_)
		(FB_VARCHAR(63 * 4), procedure_)
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				UTF8_IN(catalog_);
				UTF8_IN(schema_);
				UTF8_IN(procedure_);
				UTF8_IN(column_);
				nanodbc::result rslt =
					impl->procedure_columns(NANODBC_TEXT(in->catalog_.str), NANODBC_TEXT(in->schema_.str), NANODBC_TEXT(in->procedure_.str),
						NANODBC_TEXT(in->column_.str));
				nano::fb_ptr(out->rslt.str, (int64_t)&rslt);
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->rsltNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->rsltNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	 stmt ty$pointer not null 
//	) returns integer
//	external name 'nano!stmt_affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_affected_rows)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, affected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				out->affected = impl->affected_rows();
				out->affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->affectedNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->affectedNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function columns (
//	 stmt ty$pointer not null 
//	) returns smallint
//	external name 'nano!stmt_columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_columns)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, columns)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				out->columns = impl->columns();
				out->columnsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->columnsNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->columnsNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function reset_parameters (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_reset_parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_reset_parameters)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nano::statement* stmt = nano::stmt_ptr(in->stmt.str);
			nanodbc::statement* impl = stmt->impl();
			try
			{
				if (stmt->batch_array != nullptr) 
					stmt->batch_array->clear();
				impl->reset_parameters();
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameters  ( 
//	 stmt ty$pointer not null 
//	) returns smallint
//	external name 'nano!stmt_parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_parameters)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, parameters)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				out->parameters = impl->parameters();
				out->parametersNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->parametersNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->parametersNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameter_size  ( 
//	 stmt ty$pointer not null, 
//	 param_index smallint not null
//	) returns integer
//	external name 'nano!stmt_parameter_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_parameter_size)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, param_index)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				out->size = impl->parameter_size(in->param_index);
				if (out->size < 0) out->size = -1; // BLOB
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->sizeNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{   
			 out->sizeNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// template <class T> void bind (...
//
// create function bind_[fb_type] (
//	 stmt ty$pointer not null,
// 	 param_index smallint not null,
//   value_	[fb_type]
//	) returns ty$nano_blank
//	external name 'nano!stmt_bind'
//	engine udr; 

FB_UDR_BEGIN_FUNCTION(stmt_bind)
	
	unsigned in_count;

	enum in : short {
		stmt = 0, param_index, value
	};

	AutoArrayDelete<unsigned> in_offsets;
	AutoArrayDelete<unsigned> in_null_offsets;
	AutoArrayDelete<unsigned> in_types;
	AutoArrayDelete<unsigned> in_char_sets;

	IUtil* utl = master->getUtilInterface();

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		
		in_offsets.reset(new unsigned[in_count]);
		in_null_offsets.reset(new unsigned[in_count]);
		in_types.reset(new unsigned[in_count]);
		in_char_sets.reset(new unsigned[in_count]);

		for (unsigned i = 0; i < in_count; ++i)
		{
			in_offsets[i] = in_metadata->getOffset(status, i);
			in_null_offsets[i] = in_metadata->getNullOffset(status, i);
			in_types[i] = in_metadata->getType(status, i);
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!*(ISC_SHORT*)(in + in_null_offsets[in::stmt]))
		{
			out->blank = BLANK;
			nano::statement* stmt = nano::stmt_ptr((const char*)(in + in_offsets[in::stmt]));
			nanodbc::statement* impl = stmt->impl();
			try
			{
				short param_index = *(ISC_SHORT*)(in + in_offsets[in::param_index]);
				long batch_index = 0;
				switch (in_types[in::value])
				{
					case SQL_TEXT: // char, varchar
					case SQL_VARYING: 
					{
						std::size_t length =
							(in_types[in::value] == SQL_TEXT ? strlen((char*)(in + in_offsets[in::value])) : *(ISC_USHORT*)(in + in_offsets[in::value])) *
							(in_char_sets[in::value] == fb_char_set::CS_UTF8 ? 4 : 1);
						char* param = new char[length + 1];
						memcpy(
							param, 
							in_types[in::value] == SQL_TEXT ? (char*)(in + in_offsets[in::value]) : (char*)(in + sizeof(ISC_USHORT) + in_offsets[in::value]), 
							length
						);
						param[length] = '\0';
						if (in_char_sets[in::value] == fb_char_set::CS_UTF8) utf8_to_loc(param, param);
						batch_index = stmt->batch_array->push(param_index, nanodbc::string(param));
						impl->bind(
							param_index, 
							(char*)(stmt->batch_array->value<nanodbc::string>(param_index, batch_index)->c_str())
						);
						delete[] param;
						break;
					}
					case SQL_SHORT: // smallint 
						batch_index = stmt->batch_array->push(param_index, *(ISC_SHORT*)(in + in_offsets[in::value]));
						impl->bind(param_index, (ISC_SHORT*)(stmt->batch_array->value<ISC_SHORT>(param_index, batch_index)));
						break;
					case SQL_LONG: // integer 
						batch_index = stmt->batch_array->push(param_index, *(ISC_LONG*)(in + in_offsets[in::value]));
						impl->bind(param_index, (ISC_LONG*)(stmt->batch_array->value<ISC_LONG>(param_index, batch_index)));
						break;
					case SQL_FLOAT: // float
						batch_index = stmt->batch_array->push(param_index, *(float*)(in + in_offsets[in::value]));
						impl->bind(param_index, (float*)(stmt->batch_array->value<float>(param_index, batch_index)));
						break;
					case SQL_DOUBLE: // double precision 
					case SQL_D_FLOAT: 
						batch_index = stmt->batch_array->push(param_index, *(double*)(in + in_offsets[in::value]));
						impl->bind(param_index, (double*)(stmt->batch_array->value<double>(param_index, batch_index)));
						break;
					case SQL_TIMESTAMP: // timestamp
					{
						Firebird::FbTimestamp fb; 
						// This class has memory layout identical to ISC_TIMESTAMP
						memcpy(&fb, (ISC_TIMESTAMP*)(in + in_offsets[in::value]), sizeof(FbTimestamp));
						struct nano::timestamp tm;
						fb.date.decode(utl, &tm.year, &tm.month, &tm.day);
						fb.time.decode(utl, &tm.hour, &tm.min, &tm.sec, &tm.fract);
						nanodbc::timestamp param = nano::set_timestamp(&tm);
						batch_index = stmt->batch_array->push(param_index, param);
						impl->bind(
							param_index, 
							(nanodbc::timestamp*)(stmt->batch_array->value<nanodbc::timestamp>(param_index, batch_index))
						);
						break;
					}
					case SQL_BLOB: // blob
						break;
					case SQL_ARRAY: // array
						break;
					case SQL_QUAD: // blob_id 
						break;
					case SQL_TYPE_TIME: // time
					{
						Firebird::FbTime fb;
						// This class has memory layout identical to ISC_TIME
						memcpy(&fb, (ISC_TIME*)(in + in_offsets[in::value]), sizeof(FbTime));
						struct nano::time t;
						fb.decode(utl, &t.hour, &t.min, &t.sec, &t.fract);
						nanodbc::time param = nano::set_time(&t);
						batch_index = stmt->batch_array->push(param_index, param);
						impl->bind(
							param_index,
							(nanodbc::time*)(stmt->batch_array->value<nanodbc::time>(param_index, batch_index))
						);
						break;
					}
					case SQL_TYPE_DATE: // date
					{
						Firebird::FbDate fb;
						// This class has memory layout identical to ISC_DATE
						memcpy(&fb, (ISC_DATE*)(in + in_offsets[in::value]), sizeof(FbDate));
						struct nano::date d;
						fb.decode(utl, &d.year, &d.month, &d.day);
						nanodbc::date param = nano::set_date(&d);
						batch_index = stmt->batch_array->push(param_index, param);
						impl->bind(
							param_index,
							(nanodbc::date*)(stmt->batch_array->value<nanodbc::date>(param_index, batch_index))
						);
						break;
					}
					case SQL_INT64: // bigint
						batch_index = stmt->batch_array->push(param_index, *(ISC_INT64*)(in + in_offsets[in::value]));
						impl->bind(param_index, (ISC_INT64*)(stmt->batch_array->value<ISC_INT64>(param_index, batch_index)));
						break;
					case SQL_BOOLEAN: // boolean
					{
						bool param = native_bool(*(FB_BOOLEAN*)(in + in_offsets[in::value]));
						batch_index = stmt->batch_array->push(param_index, nanodbc::string(param ? "true" : "false"));
						impl->bind(
							param_index,
							(char*)(stmt->batch_array->value<nanodbc::string>(param_index, batch_index)->c_str())
						);
						break; 
					}
					case SQL_NULL: // null
						impl->bind_null(param_index);
						break;
					default:
						break;
				}
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function bind_null (
//	 stmt ty$pointer not null,
// 	 param_index smallint not null,
// 	 batch_size integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!stmt_bind_null'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_bind_null)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, param_index)
		(FB_INTEGER, batch_size)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				impl->bind_null(in->param_index, in->batch_size);
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function describe_parameters (
//	 stmt ty$pointer not null,
// 	 idx smallint not null,
// 	 type_ smallint not null,
// 	 size_ integer not null,
// 	 scale_ smallint not null default 0
//	) returns ty$nano_blank
//	external name 'nano!stmt_describe_parameters
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_describe_parameters)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, idx)
		(FB_SMALLINT, type)
		(FB_INTEGER, size)
		(FB_SMALLINT, scale)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blank = BLANK;
			nanodbc::statement* impl = nano::stmt_ptr(in->stmt.str)->impl();
			try
			{
				impl->describe_parameters
					(std::vector<short>(in->idx), std::vector<short>(in->type), std::vector<unsigned long>(in->size), 
					 std::vector<short>(in->scale));
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
			 NANO_THROW_ERROR(INVALID_STMT_POINTER);
		}
	}

FB_UDR_END_FUNCTION

} // namespace nano

