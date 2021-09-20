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

#include <UdrCppEngine.h>

#define EXCEPTION_ARRAY_SIZE	100
#define ERROR_MESSAGE_LENGTH	1024

#define NANODBC_ERR_MESSAGE		"NANO$NANODBC_ERR_MESSAGE"
#define INVALID_CONN_POINTER	"NANO$INVALID_CONN_POINTER"
#define INVALID_TNX_POINTER		"NANO$INVALID_TNX_POINTER"
#define INVALID_STMT_POINTER	"NANO$INVALID_STMT_POINTER"
#define INVALID_RSLT_POINTER	"NANO$INVALID_RSLT_POINTER"

struct exception
{
	char name[32];
	ISC_LONG number;
	char message[ERROR_MESSAGE_LENGTH];
};

#ifndef CONN_H
	class connection;
#endif

#ifndef TNX_H
	class transaction;
#endif

#ifndef STMT_H
	class statement;
#endif

#ifndef RSLT_H
	class result;
#endif

class resours
{
public:
	resours();
	~resours();

	const char* locale(const char* set_locale = NULL);
	const char* error_message(const char* last_error_message = NULL);

	void retain_connection(nanoudr::connection* conn);
	bool is_valid_connection(nanoudr::connection* conn);
	void expunge_connection(nanoudr::connection* conn);
	void release_connection(nanoudr::connection* conn);

	void retain_transaction(nanoudr::transaction* tnx);
	bool is_valid_transaction(nanoudr::transaction* tnx);
	void release_transaction(nanoudr::transaction* tnx);

	void retain_statement(nanoudr::statement* stmt);
	bool is_valid_statement(nanoudr::statement* stmt);
	void expunge_statement(nanoudr::connection* conn);
	void release_statement(nanoudr::statement* stmt);

	void retain_result(nanoudr::result* rslt);
	bool is_valid_result(nanoudr::result* rslt);
	void release_result(nanoudr::result* rslt);

	void expunge();

	const ISC_LONG exception_number(const char* name);
	const char* exception_message(const char* name);

	void initialize(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context);
	bool ready();

private:
	void assign_exception(exception* udr_exception, short pos);

	std::vector<nanoudr::connection*> connections;
	std::vector<nanoudr::transaction*> transactions;
	std::vector<nanoudr::statement*> statements;
	std::vector<nanoudr::result*> results;

	// if number is zero then sended ANY_THROW, see make_ready()
	exception udr_exceptions[EXCEPTION_ARRAY_SIZE] = {
		{NANODBC_ERR_MESSAGE,	0, ""},
		{INVALID_CONN_POINTER,	0, "Input parameter CONNECTION invalid."},
		{INVALID_TNX_POINTER,	0, "Input parameter TRANSACTION invalid."},
		{INVALID_STMT_POINTER,	0, "Input parameter STATEMENT invalid."},
		{INVALID_RSLT_POINTER,	0, "Input parameter RESULT invalid."}, 
	};

	std::string udr_error_message;
	std::string udr_locale;

	bool ready_;
};

extern resours udr_resours;

} // namespace nanoudr

#endif	/* CONN_H */