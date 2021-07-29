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

namespace nano
{

//-----------------------------------------------------------------------------
// create function statement (
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
			nanodbc::statement* stmt;
			if (in->connNull == FB_FALSE)
			{
				nanodbc::connection* conn = nano::connPtr(in->conn.str);
				if (in->queryNull == FB_FALSE)
					stmt = new nanodbc::statement(*conn, NANODBC_TEXT(in->query.str), in->timeout);
				else
					stmt = new nanodbc::statement(*conn);
			}
			else
				stmt = new nanodbc::statement();
			nano::fbPtr(out->stmt.str, (int64_t)stmt);
			out->stmtNull = FB_FALSE;
		}	
		catch (...)
		{
			out->stmtNull = FB_TRUE;
			throw;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dispose (
//	 stmt ty$pointer not null, 
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
		if (in->stmtNull == FB_FALSE)
		{
			try
			{
				delete nano::stmtPtr(in->stmt.str);
				out->stmtNull = FB_TRUE;
			}
			catch (...)
			{
				nano::fbPtr(out->stmt.str, nano::nativePtr(in->stmt.str));
				out->stmtNull = FB_FALSE;
				throw;
			}
		}
		else
		{
			 out->stmtNull = FB_TRUE;
			 throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function open (
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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			if (in->connNull == FB_TRUE) throw conn_POINTER_INVALID;
			nanodbc::connection* conn = nano::connPtr(in->conn.str);
			try
			{
				stmt->open(*conn);
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
			 throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function opened (
//	 stmt ty$pointer not null, 
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
		(FB_BOOLEAN, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (in->stmtNull == FB_FALSE)
		{
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				out->rslt = nano::fbBool(stmt->open());
				out->rsltNull = FB_FALSE;
			}
			catch (...)
			{
				out->rsltNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->rsltNull = FB_TRUE;
			throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	 stmt ty$pointer not null, 
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
		(FB_BOOLEAN, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (in->stmtNull == FB_FALSE)
		{
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				out->rslt = nano::fbBool(stmt->connected());
				out->rsltNull = FB_FALSE;
			}
			catch (...)
			{
				out->rsltNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->rsltNull = FB_TRUE;
			throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 tnx ty$pointer not null, 
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
		if (in->stmtNull == FB_FALSE)
		{
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				nanodbc::connection conn = stmt->connection();
				nano::fbPtr(out->conn.str, (int64_t)&conn);
				out->connNull = FB_FALSE;
			}
			catch (...)
			{
				out->connNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->connNull = FB_TRUE;
			 throw tnx_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function close (
//	 stmt ty$pointer not null, 
// ) returns ty$nano_blank
// external name 'nano!stmt_close'
// engine udr; 
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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				stmt->close();
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
			 throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function cancel (
//	 stmt ty$pointer not null, 
// ) returns ty$nano_blank
// external name 'nano!stmt_cancel'
// engine udr; 
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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				stmt->cancel();
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
			 throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer default null, 
//   query varchar(8191) character set utf8 not null,
// 	 timeout integer not null default 0 
// ) returns ty$nano_blank
// external name 'nano!stmt_prepare'
// engine udr; 
//
// \brief
// prepare (?, ?, ?, ...) returns blank and prepares the given statement to execute its associated connection
// prepare (?, null, ?, ...) returns blank and opens and prepares the given statement to execute on the given connection
//

FB_UDR_BEGIN_FUNCTION(stmt_prepare)

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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				if (in->connNull == FB_FALSE)
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					stmt->prepare(*conn, NANODBC_TEXT(in->query.str), in->timeout);
				}
				else
					stmt->prepare(NANODBC_TEXT(in->query.str), in->timeout);
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
			 throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function timeout (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
// ) returns ty$nano_blank
// external name 'nano!stmt_timeout'
// engine udr; 
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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmtPtr(in->stmt.str);
			try
			{
				stmt->timeout(in->timeout);
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
			 throw stmt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

} // namespace nano

