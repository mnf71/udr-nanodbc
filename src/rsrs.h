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

#ifndef RSRS_H
#define RSRS_H

namespace nanoudr
{

#ifndef CONN_H 
	class connection; 
#endif
#ifndef NANODBC_H
	class transaction;
#endif
#ifndef STMT_H
	class statement;
#endif
#ifndef NANODBC_H
	class result;
#endif

class resours
{
public:
	void retain_connection(nanoudr::connection* conn);
	bool is_valid_connection(nanoudr::connection* conn);
	void release_connection(nanoudr::connection* conn);

	void retain_transaction(nanodbc::transaction* tnx);
	bool is_valid_transaction(nanodbc::transaction* tnx);
	void release_transaction(nanodbc::transaction* tnx);

	void retain_statement(nanoudr::statement* stmt);
	bool is_valid_statement(nanoudr::statement* stmt);
	void release_statement(nanoudr::statement* stmt);

	void retain_result(nanodbc::result* rslt);
	bool is_valid_result(nanodbc::result* rslt);
	void release_result(nanodbc::result* rslt);

private:
	std::vector<nanoudr::connection*> connections;
	std::vector<nanodbc::transaction*> transactions;
	std::vector<nanoudr::statement*> statements;
	std::vector<nanodbc::result*> results;
};

extern resours udr_resours;

} // namespace nanoudr

#endif	/* CONN_H */