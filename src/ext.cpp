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

//-----------------------------------------------------------------------------
// nanodbc.cpp (.h) See implementation for details. 
//

#ifdef _WIN32
// needs to be included above sql.h for windows
#if !defined(__MINGW32__) && !defined(NOMINMAX)
#define NOMINMAX
#endif
#include <windows.h>
#endif 

#include <sql.h>
#include <sqlext.h>

#define NANODBC_STRINGIZE_I(text) #text
#define NANODBC_STRINGIZE(text) NANODBC_STRINGIZE_I(text)

#define NANODBC_THROW_DATABASE_ERROR(handle, handle_type)	\
    throw nanodbc::database_error(							\
        handle, handle_type, __FILE__ ":" NANODBC_STRINGIZE(__LINE__) ": ") 

#define NANODBC_CALL_RC(FUNC, RC, ...) RC = FUNC(__VA_ARGS__)
#define NANODBC_CALL(FUNC, ...) FUNC(__VA_ARGS__)

#include <nanodbc.h> 

namespace
{

inline bool success(RETCODE rc)
{
	return rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO;
}

}

//-----------------------------------------------------------------------------
// NANODBC extensions for UDR
//

#include "stmt.h"

namespace nanoudr
{

//-----------------------------------------------------------------------------
// UDR Statement class implementation
//

void statement::scrollable(const scroll_state scrollable_usage)
{
	if (scrollable_usage == scroll_state::DEFAULT)
		return;
#if (ODBCVER >= 0x0300)
	RETCODE rc;
	if (scrollable_ != scrollable_usage)
	{
		NANODBC_CALL_RC(
			SQLSetStmtAttr,
			rc,
			native_statement_handle(),
			SQL_ATTR_CURSOR_SCROLLABLE,
			scrollable_usage == scroll_state::SCROLLABLE ? (SQLPOINTER)SQL_SCROLLABLE : (SQLPOINTER)SQL_NONSCROLLABLE,
			SQL_IS_INTEGER);
	}
	if (!success(rc))
		NANODBC_THROW_DATABASE_ERROR(native_statement_handle(), SQL_HANDLE_STMT);
	scrollable_ = scrollable_usage;
#endif
	return;
}

} // namespace nanoudr

