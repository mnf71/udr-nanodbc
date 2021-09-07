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
// package nano$tnx
//

#include "conn.h"

namespace nanoudr
{

//-----------------------------------------------------------------------------
// create function transaction_ (
//	 conn ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx_transaction'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_transaction)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->connNull)
		{
			nanoudr::connection* conn = nanoudr::conn_ptr(in->conn.str);
			try
			{
				nanodbc::transaction* tnx = new nanodbc::transaction(*conn);
				nanoudr::fb_ptr(out->tnx.str, (int64_t)tnx);
				out->tnxNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->tnxNull = FB_TRUE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			out->tnxNull = FB_TRUE;
			NANO_THROW_ERROR(INVALID_CONN_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function dispose (
//	 tnx ty$pointer not null, 
// ) returns ty$pointer
// external name 'nano!tnx_dispose'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_dispose)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->tnxNull)
		{
			try
			{
				delete nanoudr::tnx_ptr(in->tnx.str);
				out->tnxNull = FB_TRUE;
			}
			catch (std::runtime_error const& e)
			{
				nanoudr::fb_ptr(out->tnx.str, nanoudr::native_ptr(in->tnx.str));
				out->tnxNull = FB_FALSE;
				NANO_THROW_ERROR(e.what());
			}
		}
		else
		{
			 out->tnxNull = FB_TRUE;
			 NANO_THROW_ERROR(INVALID_TNX_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function commit_ (
//	 tnx ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!tnx_commit'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_commit)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->tnxNull)
		{
			out->blank = BLANK;
			nanodbc::transaction* tnx = nanoudr::tnx_ptr(in->tnx.str);
			try
			{
				tnx->commit();
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
			 NANO_THROW_ERROR(INVALID_TNX_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rollback (
//	 tnx ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!tnx_rollback'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_rollback)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->tnxNull)
		{
			out->blank = BLANK;
			nanodbc::transaction* tnx = nanoudr::tnx_ptr(in->tnx.str);
			try
			{
				tnx->rollback();
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
			 NANO_THROW_ERROR(INVALID_TNX_POINTER);
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 tnx ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx_connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_connection)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->tnxNull)
		{
			nanodbc::transaction* tnx = nanoudr::tnx_ptr(in->tnx.str);
			try
			{
				nanodbc::connection conn = tnx->connection();
				nanoudr::fb_ptr(out->conn.str, (int64_t)&conn);
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
			 NANO_THROW_ERROR(INVALID_TNX_POINTER);
		}
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr
