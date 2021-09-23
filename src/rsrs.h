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
#define	BINDING_ERR_MESSAGE		"NANO$BINDING_ERR_MESSAGE"

struct exception
{
	char name[64];
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

//-----------------------------------------------------------------------------
//  Attachment resources
//

class attachment_resources
{
public:
	attachment_resources();
	attachment_resources(IAttachment* attachment);
	~attachment_resources();

	const char* locale(const char* set_locale = NULL);
	const char* error_message(const char* last_error_message = NULL);
	
	void initialize(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context);
	
	bool ready();

	struct attachment_connections
	{
		attachment_connections(nanoudr::attachment_resources* rsrs);
		std::vector<nanoudr::connection*>& conn();
		void retain(nanoudr::connection* conn);
		bool is_valid(nanoudr::connection* conn);
		void expunge(nanoudr::connection* conn);
		void release(nanoudr::connection* conn);
	private:
		std::vector<nanoudr::connection*> conn_;
		attachment_resources* rsrs_;
	};

	struct connection_transactions
	{
		connection_transactions(nanoudr::attachment_resources* rsrs);
		std::vector<nanoudr::transaction*>& tnx();
		void retain(nanoudr::transaction* tnx);
		bool is_valid(nanoudr::transaction* tnx);
		void release(nanoudr::transaction* tnx);
	private:
		std::vector<nanoudr::transaction*> tnx_;
		attachment_resources* rsrs_;
	};

	struct connection_statements
	{
		connection_statements(nanoudr::attachment_resources* rsrs);
		std::vector<nanoudr::statement*>& stmt();
		void retain(nanoudr::statement* stmt);
		bool is_valid(nanoudr::statement* stmt);
		void release(nanoudr::statement* stmt);
	private:
		std::vector<nanoudr::statement*> stmt_;
		attachment_resources* rsrs_;
	};

	struct connection_results
	{
		connection_results(nanoudr::attachment_resources* rsrs);
		std::vector<nanoudr::result*>& rslt();
		void retain(nanoudr::result* rslt);
		bool is_valid(nanoudr::result* rslt);
		void release(nanoudr::result* rslt);
	private:
		std::vector<nanoudr::result*> rslt_;
		attachment_resources* rsrs_;
	};

	void expunge();

	const ISC_LONG exception_number(const char* name);
	const char* exception_message(const char* name);

	attachment_connections connections = attachment_connections(this);
	connection_transactions transactions = connection_transactions(this);
	connection_statements statements = connection_statements(this);
	connection_results results = connection_results(this);

private:
	IAttachment* attachment_;

	// if number is zero then sended ANY_THROW, see initialize(...)
	exception udr_exceptions[EXCEPTION_ARRAY_SIZE] = {
		{NANODBC_ERR_MESSAGE,	0, ""},
		{INVALID_CONN_POINTER,	0, "Input parameter CONNECTION invalid."},
		{INVALID_TNX_POINTER,	0, "Input parameter TRANSACTION invalid."},
		{INVALID_STMT_POINTER,	0, "Input parameter STATEMENT invalid."},
		{INVALID_RSLT_POINTER,	0, "Input parameter RESULT invalid."} ,
		{BINDING_ERR_MESSAGE,	0, ""}
	};

	void assign_exception(exception* udr_exception, short pos);

	std::string att_error_message;
	std::string att_locale;

	bool ready_;
};

//-----------------------------------------------------------------------------
//  Shared UDR resources
//

class resources
{
public:
	resources();
	~resources();

	void registred(IAttachment* att);
	void unregistred(IAttachment* att);
	attachment_resources* instance(IAttachment* att);

private:
	std::vector<nanoudr::attachment_resources*> att_;
};

extern attachment_resources udr_resources; // redo
extern resources udr_resources_;

} // namespace nanoudr

#endif	/* CONN_H */