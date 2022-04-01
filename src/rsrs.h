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

#define EXCEPTION_ARRAY_SIZE	20
#define ERROR_MESSAGE_LENGTH	1024

#define	RESOURCES_UNDEFINED		"NANO$RESOURCES_UNDEFINED" 
#define INVALID_RESOURCE		"NANO$INVALID_RESOURCE"

#define NANOUDR_ERROR			"NANO$NANOUDR_ERROR"
#define NANODBC_ERROR			"NANO$NANODBC_ERROR"

#define INVALID_CONNECTION		INVALID_RESOURCE, "Invalid connection pointer."
#define INVALID_TRANSACTION		INVALID_RESOURCE, "Invalid transaction pointer."
#define INVALID_STATEMENT		INVALID_RESOURCE, "Invalid statement pointer."
#define INVALID_RESULT			INVALID_RESOURCE, "Invalid result pointer."
#define INVALID_CATALOG			INVALID_RESOURCE, "Invalid catalog pointer."

#define	BINDING_ERROR			"NANO$BINDING_ERROR"
#define	FETCHING_ERROR			"NANO$FETCHING_ERROR"
#define	PUMPING_ERROR			"NANO$PUMPING_ERROR"

struct resources_context
{
	FB_UDR_STATUS_TYPE* status;
	FB_UDR_CONTEXT_TYPE* context;
	ITransaction* current_transaction;
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

#ifndef CTLG_H
	class catalog;
#endif

//-----------------------------------------------------------------------------
//  Attachment resources
//

struct attachment_snapshot
{
	FB_UDR_STATUS_TYPE* status;
	FB_UDR_CONTEXT_TYPE* context;
	ITransaction* transaction;
};

class attachment_resources
{
public:

	attachment_resources(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context, const ISC_UINT64 attachment_id);
	~attachment_resources() noexcept;

	void expunge();

	const ISC_UINT64 current_attachment_id() { return attachment_id; };
	const attachment_snapshot* current_snapshot(FB_UDR_STATUS_TYPE* status = nullptr, FB_UDR_CONTEXT_TYPE* context = nullptr);
	const ITransaction* current_transaction(ITransaction* set_transaction = nullptr);
	const char* current_error_message(const char* set_error_message = NULL);
	const char* current_locale(const char* set_locale = NULL);

	const ISC_LONG exception_number(const char* name);
	const char* exception_message(const char* name);

	void pull_up_resources();

	class attachment_connections
	{
		public:
			attachment_connections(attachment_resources* att_resources)
				: att_resources_(att_resources) 
			{
			};
			~attachment_connections() noexcept {};
			void retain(const nanoudr::connection* conn);
			void expunge(const nanoudr::connection* conn);
			void release(const nanoudr::connection* conn);
			bool valid(const nanoudr::connection* conn);
			std::vector<nanoudr::connection*>& conn();
		private:
			std::vector<nanoudr::connection*> conn_v;
			std::vector<nanoudr::connection*>::iterator conn_it;
			attachment_resources* att_resources_;
	};

	class connection_transactions
	{
		public:
			connection_transactions() {};
			~connection_transactions() noexcept {};
			void retain(const nanoudr::transaction* tnx);
			void release(const nanoudr::transaction* tnx);
			bool valid(const nanoudr::transaction* tnx);
			std::vector<nanoudr::transaction*>& tnx();
		private:
			std::vector<nanoudr::transaction*> tnx_v;
			std::vector<nanoudr::transaction*>::iterator tnx_it;
	};

	class connection_statements
	{
		public:
			connection_statements() {};
			~connection_statements() noexcept {};
			void retain(const nanoudr::statement* stmt);
			void release(const nanoudr::statement* stmt);
			bool valid(const nanoudr::statement* stmt);
			std::vector<nanoudr::statement*>& stmt();
		private:
			std::vector<nanoudr::statement*> stmt_v;
			std::vector<nanoudr::statement*>::iterator stmt_it;
	};

	class connection_results
	{
		public:
			connection_results() {};
			~connection_results() noexcept {};
			void retain(const nanoudr::result* rslt);
			void release(const nanoudr::result* rslt);
			bool valid(const nanoudr::result* rslt);
			std::vector<nanoudr::result*>& rslt();
		private:
			std::vector<nanoudr::result*> rslt_v;
			std::vector<nanoudr::result*>::iterator rslt_it;
	};

	class connection_catalogs
	{
		public:
			connection_catalogs() {};
			~connection_catalogs() noexcept {};
			void retain(const nanoudr::catalog* ctlg);
			void release(const nanoudr::catalog* ctlg);
			bool valid(const nanoudr::catalog* ctlg);
			std::vector<nanoudr::catalog*>& ctlg();
		private:
			std::vector<nanoudr::catalog*> ctlg_v;
			std::vector<nanoudr::catalog*>::iterator ctlg_it;
	};

	attachment_connections connections = attachment_connections(this);
	connection_transactions transactions = connection_transactions();
	connection_statements statements = connection_statements();
	connection_results results = connection_results();
	connection_catalogs catalogs = connection_catalogs();

private:
	ISC_UINT64 attachment_id;

	// if number is zero then sended ANY_THROW 
	exception exceptions[EXCEPTION_ARRAY_SIZE] = {
		{RESOURCES_UNDEFINED,		0, ""}, 
		{INVALID_RESOURCE,			0, ""},
		{NANOUDR_ERROR,				0, ""},
		{NANODBC_ERROR,				0, ""},
		{BINDING_ERROR,				0, ""},
		{FETCHING_ERROR,			0, ""},
		{PUMPING_ERROR,				0, ""}
	};

	attachment_snapshot snapshot;
	std::string error_message;
	std::string locale;

	void pull_up_exceptions();
};

//-----------------------------------------------------------------------------
//  UDR resources 
//

using resources_mapping = std::map<ISC_UINT64, attachment_resources*>;

class resources_pool
{
public:
	resources_pool();
	~resources_pool() noexcept;

	ISC_UINT64 initialize_attachment(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);
	attachment_resources* current_resources(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);
	attachment_resources* current_resources(const ISC_UINT64 attachment_id);
	void finalize_attachment(const ISC_UINT64 attachment_id);

	exception exceptions = {
		RESOURCES_UNDEFINED, 0, "Attachment resources undefined."
	};

private:
	resources_mapping resources_map;
	resources_mapping::iterator resources_it;

	ISC_UINT64 attachment_id(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context);
};

extern resources_pool pool;

} // namespace nanoudr

#endif	/* RSRS_H */