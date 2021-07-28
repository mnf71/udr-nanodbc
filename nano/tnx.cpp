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

using namespace nanodbc;

namespace nano 
{

	//-----------------------------------------------------------------------------
	// create function transaction (
	//	 conn ty$pointer, 
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
			if (in->connNull == FB_FALSE)
			{
				try
				{
					nanodbc::connection* conn = nano::connPtr(in->conn.str);
					nanodbc::transaction* tnx = new nanodbc::transaction(*conn);
					nano::fbPtr(out->tnx.str, (int64_t)tnx);
					out->tnxNull = FB_FALSE;
				}
				catch (...)
				{
					out->tnxNull = FB_TRUE;
					throw;
				}
			}
			else
				out->tnxNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create function dispose (
	//	 conn ty$pointer, 
	//	) returns ty$pointer
	//	external name 'nano!tnx_dispose'
	//	engine udr; 
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
			if (in->tnxNull == FB_FALSE)
			{
				try
				{
					delete nano::tnxPtr(in->tnx.str);
					out->tnxNull = FB_TRUE;
				}
				catch (...)
				{
					nano::fbPtr
						(out->tnx.str, nano::nativePtr(in->tnx.str));
					out->tnxNull = FB_FALSE;
					throw;
				}
			}
			else
				out->tnxNull = FB_TRUE;
		}

	FB_UDR_END_FUNCTION

	//-----------------------------------------------------------------------------
	// create function commit (
	//	 tnx ty$pointer, 
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
			(NANO_BLANK, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->tnxNull == FB_FALSE)
			{
				out->rslt = BLANK;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::transaction* tnx = nano::tnxPtr(in->tnx.str);
					tnx->commit();
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
	// create function rollback (
	//	 tnx ty$pointer, 
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
			(NANO_BLANK, rslt)
		);

		FB_UDR_EXECUTE_FUNCTION
		{
			if (in->tnxNull == FB_FALSE)
			{
				out->rslt = BLANK;
				out->rsltNull = FB_FALSE;
				try
				{
					nanodbc::transaction* tnx = nano::tnxPtr(in->tnx.str);
					tnx->rollback();
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

} // namespace nano
