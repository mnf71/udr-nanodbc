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
 // package nano$conn
 //

#include <nanodbc.h> 

namespace nanoudr
{

//-----------------------------------------------------------------------------
//

class connection : public nanodbc::connection
{
public:
	connection() : nanodbc::connection()
	{
	};

	connection(const nanodbc::string& dsn, const nanodbc::string& user, const nanodbc::string& pass, long timeout = 0) 
		: nanodbc::connection(dsn, user, pass, timeout)
	{
	};

	connection(const nanodbc::string& connection_string, long timeout = 0) 
		: nanodbc::connection(connection_string, timeout)
	{
	};

	~connection() noexcept
	{
		nanodbc::connection::~connection();
	};

	std::vector<nanoudr::statement*> statements;
};

} // namespace nanoudr