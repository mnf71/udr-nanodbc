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
// package nano$tnx
//

namespace nanoudr
{

//-----------------------------------------------------------------------------
// UDR Transaction class implementation
//

transaction::transaction(attachment_resources* att_resources, class nanoudr::connection& conn)
	: nanodbc::transaction(conn)
{
	att_resources->transactions.retain(this);
	conn_ = &conn;
}

transaction::~transaction()
{
	nanodbc::transaction::~transaction();
}

nanoudr::connection* transaction::connection()
{
	return conn_;
}

//-----------------------------------------------------------------------------
// create function transaction_ (
//	 conn ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx_transaction'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_transaction)

	nanoudr::attachment_resources* att_resources;

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
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->tnxNull = FB_TRUE;
		nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
		if (!in->connNull && att_resources->connections.is_valid(conn))
		{
			try
			{
				nanodbc::transaction* tnx = new nanodbc::transaction(*conn);
				udr_helper.fb_ptr(out->tnx.str, (int64_t)tnx);
				out->tnxNull = FB_FALSE;
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
// create function release_ (
//	 tnx ty$pointer not null, 
// ) returns ty$pointer
// external name 'nano!tnx_release'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_release)

	nanoudr::attachment_resources* att_resources;

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
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->tnxNull = FB_TRUE;
		nanoudr::transaction* tnx = udr_helper.tnx_ptr(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.is_valid(tnx))
		{
			try
			{
				att_resources->transactions.release(tnx);
			}
			catch (std::runtime_error const& e)
			{
				udr_helper.fb_ptr(out->tnx.str, udr_helper.native_ptr(in->tnx.str));
				out->tnxNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_TNX_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_valid (
//	 tnx ty$pointer not null, 
//	) returns boolean
//	external name 'nano!tnx_is_valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx_is_valid)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, tnx)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, valid)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->valid =
			in->tnxNull ?
			udr_helper.fb_bool(false) :
			att_resources->transactions.is_valid(udr_helper.tnx_ptr(in->tnx.str));
		out->validNull = FB_FALSE;
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

	nanoudr::attachment_resources* att_resources;

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
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->connNull = FB_TRUE;
		nanoudr::transaction* tnx = udr_helper.tnx_ptr(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.is_valid(tnx))
		{
			try
			{
				nanoudr::connection* conn = tnx->connection();
				udr_helper.fb_ptr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_TNX_POINTER)
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

	nanoudr::attachment_resources* att_resources;

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
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		nanoudr::transaction* tnx = udr_helper.tnx_ptr(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.is_valid(tnx))
		{
			try
			{
				tnx->commit();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_TNX_POINTER)
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

	nanoudr::attachment_resources* att_resources;

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
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blank = BLANK;
		nanoudr::transaction* tnx = udr_helper.tnx_ptr(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.is_valid(tnx))
		{
			try
			{
				tnx->rollback();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_TNX_POINTER)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr
