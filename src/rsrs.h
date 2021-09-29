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

#include <map>

namespace nanoudr
{

#include <UdrCppEngine.h>

#define EXCEPTION_ARRAY_SIZE	100
#define ERROR_MESSAGE_LENGTH	1024

#define	RESOURCES_INDEFINED		"NANO$RESOURCES_INDEFINED"
#define INVALID_CONN_POINTER	"NANO$INVALID_CONN_POINTER"
#define INVALID_TNX_POINTER		"NANO$INVALID_TNX_POINTER"
#define INVALID_STMT_POINTER	"NANO$INVALID_STMT_POINTER"
#define INVALID_RSLT_POINTER	"NANO$INVALID_RSLT_POINTER"
#define NANODBC_ERR_MESSAGE		"NANO$NANODBC_ERR_MESSAGE"
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
	attachment_resources(ISC_UINT64 attachment_id);
	void initialize(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context);
	~attachment_resources() noexcept;

	const char* locale(const char* set_locale = NULL);
	const char* error_message(const char* last_error_message = NULL);

	struct attachment_connections
	{
		attachment_connections(nanoudr::attachment_resources* owner_);
		std::vector<nanoudr::connection*>& conn();
		void retain(nanoudr::connection* conn);
		bool is_valid(nanoudr::connection* conn);
		void expunge(nanoudr::connection* conn);
		void release(nanoudr::connection* conn);
	private:
		std::vector<nanoudr::connection*> conn_;
		attachment_resources* owner_;
	};

	struct connection_transactions
	{
		connection_transactions();
		std::vector<nanoudr::transaction*>& tnx();
		void retain(nanoudr::transaction* tnx);
		bool is_valid(nanoudr::transaction* tnx);
		void release(nanoudr::transaction* tnx);
	private:
		std::vector<nanoudr::transaction*> tnx_;
	};

	struct connection_statements
	{
		connection_statements();
		std::vector<nanoudr::statement*>& stmt();
		void retain(nanoudr::statement* stmt);
		bool is_valid(nanoudr::statement* stmt);
		void release(nanoudr::statement* stmt);
	private:
		std::vector<nanoudr::statement*> stmt_;
	};

	struct connection_results
	{
		connection_results();
		std::vector<nanoudr::result*>& rslt();
		void retain(nanoudr::result* rslt);
		bool is_valid(nanoudr::result* rslt);
		void release(nanoudr::result* rslt);
	private:
		std::vector<nanoudr::result*> rslt_;
	};

	void expunge();

	const ISC_LONG exception_number(const char* name);
	const char* exception_message(const char* name);

	attachment_connections connections = attachment_connections(this);
	connection_transactions transactions = connection_transactions();
	connection_statements statements = connection_statements();
	connection_results results = connection_results();

private:
	ISC_UINT64 attachment_id_;

	// if number is zero then sended ANY_THROW, see initialize(...)
	exception udr_exceptions[EXCEPTION_ARRAY_SIZE] = {
		{RESOURCES_INDEFINED,	0, "Attachment resources indefined."},
		{INVALID_CONN_POINTER,	0, "Input parameter CONNECTION invalid."},
		{INVALID_TNX_POINTER,	0, "Input parameter TRANSACTION invalid."},
		{INVALID_STMT_POINTER,	0, "Input parameter STATEMENT invalid."},
		{INVALID_RSLT_POINTER,	0, "Input parameter RESULT invalid."} ,
		{NANODBC_ERR_MESSAGE,	0, ""},
		{BINDING_ERR_MESSAGE,	0, ""}
	};

	void make_exceptions(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context);

	void assign_exception(exception* udr_exception, short pos);

	std::string att_error_message;
	std::string att_locale;
};

//-----------------------------------------------------------------------------
//  Shared UDR resources
//

using attachment_mapping = std::map<ISC_UINT64, nanoudr::attachment_resources*>;

class resources
{
public:
	resources();
	~resources() noexcept;

	attachment_resources* attachment(
		FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context, const bool read_only = true);
	void expunge(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context);

private:
	attachment_mapping attachments;

	ISC_UINT64 attachment_id(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context);

	attachment_mapping::iterator it_att_;
};

extern resources udr_resources;

} // namespace nanoudr

#endif	/* CONN_H */