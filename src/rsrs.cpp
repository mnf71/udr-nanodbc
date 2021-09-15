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
 // Used resourses of heap
 // 

namespace nanoudr
{

nanoudr::resours udr_resours;

void resours::retain_connection(nanoudr::connection* conn)
{
	connections.push_back(conn);
}

bool resours::is_valid_connection(nanoudr::connection* conn)
{
	return find(connections.begin(), connections.end(), conn) != connections.end();
}

void resours::release_connection(nanoudr::connection* conn)
{
	std::vector<nanoudr::connection*>::iterator
		it = std::find(connections.begin(), connections.end(), conn);
	if (it != connections.end()) 
	{
		for (auto s : statements) 
			if (s->connection() == conn) release_statement(s);
		delete (nanoudr::connection*)(conn);
		connections.erase(it);
	}
}

void resours::retain_statement(nanoudr::statement* stmt)
{
	statements.push_back(stmt);
}

bool resours::is_valid_statement(nanoudr::statement* stmt)
{
	return find(statements.begin(), statements.end(), stmt) != statements.end();
}

void resours::release_statement(nanoudr::statement* stmt)
{
	std::vector<nanoudr::statement*>::iterator
		it = std::find(statements.begin(), statements.end(), stmt);
	if (it != statements.end())
	{
		delete (nanoudr::statement*)(stmt);
		statements.erase(it);
	}
}




} // namespace nanoudr
