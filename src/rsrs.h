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

#define EXCEPTION_ARRAY_SIZE	100
#define ERROR_MESSAGE_LENGTH	1024

#define RANDOM_ERROR_MESSAGE	"NANO$RANDOM_ERROR_MESSAGE"
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

class resours
{
public:
	resours();
	~resours();

	const char* locale(const char* set_locale = nullptr);
	const char* error_message(const char* last_error_message = nullptr);

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

	const long exception_number(const char* name);
	const char* exception_message(const char* name);
	void assign_exception(exception* udr_exception, short pos);

	bool initialized();

private:
	std::vector<nanoudr::connection*> connections;
	std::vector<nanodbc::transaction*> transactions;
	std::vector<nanoudr::statement*> statements;
	std::vector<nanodbc::result*> results;

	exception udr_exceptions[EXCEPTION_ARRAY_SIZE] = {
		{RANDOM_ERROR_MESSAGE,	0, ""},
		{INVALID_CONN_POINTER,	0, "Input parameter CONNECTION invalid."},
		{INVALID_TNX_POINTER,	0, "Input parameter TRANSACTION invalid."},
		{INVALID_STMT_POINTER,	0, "Input parameter STATEMENT invalid."},
		{INVALID_RSLT_POINTER,	0, "Input parameter RESULT invalid."}, 
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""},
		{"",	0, ""}
	};

	std::string udr_error_message;
	std::string udr_locale;

	bool initialized_;
};

extern resours udr_resours;

} // namespace nanoudr

#endif	/* CONN_H */