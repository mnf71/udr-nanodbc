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

transaction::transaction(class attachment_resources& att_resources, class nanoudr::connection& conn)
	: nanodbc::transaction(conn)
{
	att_resources_ = &att_resources;
	att_resources_->transactions.retain(this);
	conn_ = &conn;
}

transaction::~transaction()
{
	att_resources_->transactions.release(this);
}

nanoudr::connection* transaction::connection()
{
	return conn_;
}

//-----------------------------------------------------------------------------
// create function transaction_ (
//	conn ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx$transaction'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx$transaction)
	
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
		(NANO_POINTER, tnx)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->tnxNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanoudr::transaction* tnx =	new nanoudr::transaction(*att_resources, *conn);
				helper.fb_ptr(out->tnx.str, tnx);
				out->tnxNull = FB_FALSE;
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
// create function valid (
//	tnx ty$pointer not null, 
//	) returns boolean
//	external name 'nano!tnx$valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx$valid)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->valid = helper.fb_bool(
			in->tnxNull ? false :
				att_resources->transactions.valid(helper.native_ptr<transaction>(in->tnx.str))
			);
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	tnx ty$pointer not null, 
//	) returns ty$pointer
//	external name 'nano!tnx$release'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx$release)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->tnxNull = FB_TRUE;
		if (!in->tnxNull)
		{
			nanoudr::transaction* tnx = helper.native_ptr<transaction>(in->tnx.str);
			try
			{
				if (att_resources->transactions.valid(tnx)) delete (nanoudr::transaction*)(tnx);
			}
			catch (std::runtime_error const& e)
			{
				helper.fb_ptr(out->tnx.str, tnx);
				out->tnxNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_TRANSACTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	tnx ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx$connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx$connection)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->connNull = FB_TRUE;
		nanoudr::transaction* tnx = helper.native_ptr<transaction>(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.valid(tnx))
		{
			try
			{
				const nanoudr::connection* conn = tnx->connection();
				helper.fb_ptr(out->conn.str, conn);
				out->connNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_TRANSACTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function commit_ (
//	tnx ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx$commit'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx$commit)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->tnxNull = FB_TRUE;
		nanoudr::transaction* tnx = helper.native_ptr<transaction>(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.valid(tnx))
		{
			try
			{
				tnx->commit();
				delete (nanoudr::transaction*)(tnx);
			}
			catch (std::runtime_error const& e)
			{
				helper.fb_ptr(out->tnx.str, tnx);
				out->tnxNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_TRANSACTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rollback (
//	tnx ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!tnx$rollback'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(tnx$rollback)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

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
		FUNCTION_RESOURCES
		out->tnxNull = FB_TRUE;
		nanoudr::transaction* tnx = helper.native_ptr<transaction>(in->tnx.str);
		if (!in->tnxNull && att_resources->transactions.valid(tnx))
		{
			try
			{
				tnx->rollback();
				delete (nanoudr::transaction*)(tnx);
			}
			catch (std::runtime_error const& e)
			{
				helper.fb_ptr(out->tnx.str, tnx);
				out->tnxNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_TRANSACTION)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr
