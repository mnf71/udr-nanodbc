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
// package nano$func
//

using namespace nanodbc;

namespace nano
{

//-----------------------------------------------------------------------------
// todo: create procedure list_datasources - ?
//

//-----------------------------------------------------------------------------
// todo: create procedure list_drivers - ?
//

//-----------------------------------------------------------------------------
// create function execute_conn (
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1 
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!func_execute_conn'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_execute_conn)

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
		if (in->connNull == FB_FALSE)
		{
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				nanodbc::result rslt = 
					nanodbc::execute(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
				nano::fb_ptr(out->rslt.str, (int64_t)&rslt);
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
			throw conn_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_conn (
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!func_just_execute_conn'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_just_execute_conn)

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
		if (in->connNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::connection* conn = nano::conn_ptr(in->conn.str);
			try
			{
				nanodbc::just_execute
					(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
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
// create function execute_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$pointer
//	external name 'nano!func_execute_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_execute_stmt)

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
		if (in->stmtNull == FB_FALSE)
		{
			nanodbc::statement* stmt = nano::stmt_ptr(in->stmt.str);
			try
			{
				nanodbc::result rslt = nanodbc::execute(*stmt, in->batch_operations);
				nano::fb_ptr(out->rslt.str, (int64_t)&rslt);
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
// create function just_execute_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!func_just_execute_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_just_execute_stmt)

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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmt_ptr(in->stmt.str);
			try
			{
				nanodbc::just_execute(*stmt, in->batch_operations);
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
// create function transact_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$pointer
//	external name 'nano!func_transact_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_transact_stmt)

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
		if (in->stmtNull == FB_FALSE)
		{
			nanodbc::statement* stmt = nano::stmt_ptr(in->stmt.str);
			try
			{
				nanodbc::result rslt = nanodbc::transact(*stmt, in->batch_operations);
				nano::fb_ptr(out->rslt.str, (int64_t)&rslt);
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
// create function just_transact_stmt (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!func_just_transact_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_just_transact_stmt)

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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmt_ptr(in->stmt.str);
			try
			{
				nanodbc::just_transact(*stmt, in->batch_operations);
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
// create function prepare_stmt (
//	 stmt ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!func_prepare_stmt'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(func_prepare_stmt)

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
		if (in->stmtNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::statement* stmt = nano::stmt_ptr(in->stmt.str);
			try
			{
				nanodbc::prepare(*stmt, (NANODBC_TEXT(in->query.str)), in->timeout);
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

