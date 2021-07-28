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
using namespace nanodbc;

namespace nano
{

	//-----------------------------------------------------------------------------
	// create function catalog_name (
	//	 conn ty$pointer, 
	//	) returns varchar(128) character set utf8
	//	external name 'nano!conn_catalog_name'
	//	engine udr; 
	//

	FB_UDR_BEGIN_FUNCTION(conn_catalog_name)

		FB_UDR_MESSAGE(
			InMessage,
			(NANO_POINTER, conn)
		);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (in->connNull == FB_FALSE)
		{
			memset(out->rslt.str, 0, sizeof(out->rslt.str));
			try
			{
				nanodbc::connection* conn = nano::connPtr(in->conn.str);
				string name = conn->catalog_name();
				out->rslt.length =
					(ISC_USHORT)name.length() < (ISC_USHORT)sizeof(out->rslt.str) ?
						(ISC_USHORT)name.length() : (ISC_USHORT)sizeof(out->rslt.str);
				memcpy(out->rslt.str, name.c_str(), out->rslt.length);
				out->rsltNull = FB_FALSE;
			}
			catch (...)
			{
				out->rsltNull = FB_TRUE;
				throw;
			}
		}
		else
			out->rsltNull = FB_TRUE;
	}

		FB_UDR_END_FUNCTION



} // namespace nano
