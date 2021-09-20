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

#ifndef RSLT_H
#define RSLT_H

namespace nanoudr
{

//-----------------------------------------------------------------------------
// UDR Result class implementation
//

#ifndef CONN_H
	class connection;
#endif

class result 
{
public:
	nanodbc::result* native();

	explicit result(class nanoudr::connection& conn, class nanodbc::result& rslt);
	~result() noexcept;

	nanoudr::connection* connection();

private:
	class nanodbc::result rslt_;
	nanoudr::connection* conn_;
};

} // namespace nanoudr

#endif	/* RSLT_H */