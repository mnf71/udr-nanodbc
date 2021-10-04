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

#define EXCEPTION_ARRAY_SIZE	100
#define ERROR_MESSAGE_LENGTH	1024

#define	RESOURCES_INDEFINED		"NANO$RESOURCES_INDEFINED" 

#define NANOUDR_ERR_MESSAGE		"NANO$NANOUDR_ERR_MESSAGE"
#define POINTER_CONN_INVALID	"NANO$POINTER_CONNECTION_INVALID"
#define POINTER_TNX_INVALID		"NANO$POINTER_TRANSACTION_INVALID"
#define POINTER_STMT_INVALID	"NANO$POINTER_STATEMENT_INVALID"
#define POINTER_RSLT_INVALID	"NANO$POINTER_RESULT_INVALID"
#define NANODBC_ERR_MESSAGE		"NANO$NANODBC_ERR_MESSAGE"
#define	BINDING_ERR_MESSAGE		"NANO$BINDING_ERR_MESSAGE"
#define	FETCHING_ERR_MESSAGE	"NANO$FETCHING_ERR_MESSAGE"

struct fb_context
{
	FB_UDR_STATUS_TYPE* status;
	FB_UDR_CONTEXT_TYPE* context;
};

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
	attachment_resources(ISC_UINT64 attachment_id) 
		: attachment_id(attachment_id), att_locale("cp1251") {
		connections.retain(nullptr); // service record for statement::statement();
	};
	~attachment_resources() noexcept;

	const fb_context* context() { return &attachment_context; };

	void expunge();

	const char* locale(const char* set_locale = NULL);
	const char* error_message(const char* last_error_message = NULL);

	void make_resources();

	struct attachment_connections
	{
		attachment_connections(attachment_resources* att_resources) 
			: outer(att_resources) {};
		void retain(const nanoudr::connection* conn);
		void expunge(const nanoudr::connection* conn);
		void release(const nanoudr::connection* conn);
		bool valid(const nanoudr::connection* conn);
		std::vector<nanoudr::connection*>& conn();
	private:
		std::vector<nanoudr::connection*> conn_v;
		std::vector<nanoudr::connection*>::iterator conn_it;
		attachment_resources* outer;
	};

	struct connection_transactions
	{
		connection_transactions(attachment_resources* att_resources) 
			: outer(att_resources) {};
		void retain(const nanoudr::transaction* tnx);
		void release(const nanoudr::transaction* tnx);
		bool valid(const nanoudr::transaction* tnx);
		std::vector<nanoudr::transaction*>& tnx();
	private:
		std::vector<nanoudr::transaction*> tnx_v;
		std::vector<nanoudr::transaction*>::iterator tnx_it;
		attachment_resources* outer;
	};

	struct connection_statements
	{
		connection_statements(attachment_resources* att_resources) 
			: outer(att_resources) {};
		void retain(const nanoudr::statement* stmt);
		void release(const nanoudr::statement* stmt);
		bool valid(const nanoudr::statement* stmt);
		std::vector<nanoudr::statement*>& stmt();
	private:
		std::vector<nanoudr::statement*> stmt_v;
		std::vector<nanoudr::statement*>::iterator stmt_it;
		attachment_resources* outer;
	};

	struct connection_results
	{
		connection_results(attachment_resources* att_resources) 
			: outer(att_resources) {};
		void retain(const nanoudr::result* rslt);
		void release(const nanoudr::result* rslt);
		bool valid(const nanoudr::result* rslt);
		std::vector<nanoudr::result*>& rslt();
	private:
		std::vector<nanoudr::result*> rslt_v;
		std::vector<nanoudr::result*>::iterator rslt_it;
		attachment_resources* outer;
	};

	const ISC_LONG exception_number(const char* name);
	const char* exception_message(const char* name);

	attachment_connections connections = attachment_connections(this);
	connection_transactions transactions = connection_transactions(this);
	connection_statements statements = connection_statements(this);
	connection_results results = connection_results(this);

private:
	ISC_UINT64 attachment_id;
	fb_context attachment_context;

	void context(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);

	// if number is zero then sended ANY_THROW 
	exception att_exceptions[EXCEPTION_ARRAY_SIZE] = {
		{RESOURCES_INDEFINED,	0, ""}, // used for class resources
		{NANOUDR_ERR_MESSAGE,	0, ""},
		{POINTER_CONN_INVALID,	0, "Pointer CONNECTION invalid."},
		{POINTER_TNX_INVALID,	0, "Pointer TRANSACTION invalid."},
		{POINTER_STMT_INVALID,	0, "Pointer STATEMENT invalid."},
		{POINTER_RSLT_INVALID,	0, "Pointer RESULT invalid."} ,
		{NANODBC_ERR_MESSAGE,	0, ""},
		{BINDING_ERR_MESSAGE,	0, ""},
		{FETCHING_ERR_MESSAGE,	0, ""}
	};

	void make_exceptions(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);
	void assign_exception(exception* att_exception, const short pos);

	std::string att_error_message;
	std::string att_locale;

	friend class resources;
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

	void initialize(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);
	void finalize(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);

	attachment_resources* attachment(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);

	exception resource_exception = { RESOURCES_INDEFINED, 0, "Attachment resources indefined." };

private:
	attachment_mapping att_m;

	ISC_UINT64 attachment_id(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);

	attachment_mapping::iterator att_it;
};

extern resources udr_resources;

} // namespace nanoudr

#endif	/* RSRS_H */