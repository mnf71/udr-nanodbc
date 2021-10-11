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
 //  Resources managment
 //

namespace nanoudr
{

//-----------------------------------------------------------------------------
//  Attachment resources
//

attachment_resources::~attachment_resources() noexcept
{
	attachment_resources::expunge();
}

void attachment_resources::expunge()
{
	for (auto c : connections.conn()) delete (nanoudr::connection*)(c);
}

void attachment_resources::context(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	attachment_context.status = status;
	attachment_context.context = context;
};

const char* attachment_resources::locale(const char* set_locale)
{
	if (set_locale) att_locale = set_locale;
	return att_locale.c_str();
}

const char* attachment_resources::error_message(const char* last_error_message)
{
	if (last_error_message) att_error_message = last_error_message;
	return att_error_message.c_str();
}

void attachment_resources::make_resources()
{
	FB_UDR_STATUS_TYPE* status = attachment_context.status;
	FB_UDR_CONTEXT_TYPE* context = attachment_context.context;
	
	make_exceptions(status, context);
	// ... other
}

void attachment_resources::make_exceptions(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	IAttachment* att;
	ITransaction* tra;
	AutoRelease<IStatement> stmt;
	AutoRelease<IMessageMetadata> meta;
	AutoRelease<IResultSet> curs;

	enum sql_rslt : short { name = 0, number, message };

	char* sql_stmt = "	\
SELECT CAST(TRIM(ex.rdb$exception_name) AS VARCHAR(63)) AS name,	\
       ex.rdb$exception_number as number, ex.rdb$message as message	\
  FROM rdb$exceptions ex	\
  WHERE TRIM(ex.rdb$exception_name) STARTING WITH 'NANO$'";

	try
	{
		att = context->getAttachment(status);
		tra = context->getTransaction(status);
		stmt.reset(att->prepare(status, tra, 0, sql_stmt, SQL_DIALECT_CURRENT, IStatement::PREPARE_PREFETCH_METADATA));
		meta.reset(stmt->getOutputMetadata(status));
		curs.reset(stmt->openCursor(status, tra, NULL, NULL, meta, 0));
		unsigned buf_length = meta->getMessageLength(status);
		unsigned char* buffer = new unsigned char[buf_length];

		nanoudr::exception udr_exception = { "\0", 0, "\0" };
		for (int i = 0; i < EXCEPTION_ARRAY_SIZE; ++i)
			assign_exception(&udr_exception, i);

		for (int i = 0; curs->fetchNext(status, buffer) == IStatus::RESULT_OK && i < EXCEPTION_ARRAY_SIZE; ++i)
		{
			memcpy( // this SQL_VARYING, see sql_stmt
				udr_exception.name,
				(buffer + 2 + meta->getOffset(status, sql_rslt::name)),
				meta->getLength(status, sql_rslt::name) - 2);
			udr_exception.number = *(ISC_LONG*)(buffer + meta->getOffset(status, sql_rslt::number));
			memcpy( // SQL_VARYING
				udr_exception.message,
				(buffer + 2 + meta->getOffset(status, sql_rslt::message)),
				meta->getLength(status, sql_rslt::message) - 2);
			assign_exception(&udr_exception, i);
		}
		curs->close(status);
		delete[] buffer;
	}
	catch (...)
	{
		throw("Make exceptions crashed.");
	}
}

//-----------------------------------------------------------------------------
//	

void attachment_resources::attachment_connections::retain(const nanoudr::connection* conn)
{
	conn_v.push_back((nanoudr::connection*)(conn));
}

void attachment_resources::attachment_connections::expunge(const nanoudr::connection* conn)
{
	conn_it = std::find(conn_v.begin(), conn_v.end(), conn);
	if (conn_it != conn_v.end())
	{
		for (auto t : outer->transactions.tnx()) if (t->connection() == conn) delete (nanoudr::transaction*)(t);
		for (auto s : outer->statements.stmt())	if (s->connection() == conn) delete (nanoudr::statement*)(s);
		for (auto r : outer->results.rslt()) if (r->connection() == conn) delete (nanoudr::result*)(r);
	}
}

void attachment_resources::attachment_connections::release(const nanoudr::connection* conn)
{
	conn_it = std::find(conn_v.begin(), conn_v.end(), conn);
	if (conn_it != conn_v.end())
	{
		for (auto t : outer->transactions.tnx()) if (t->connection() == conn) delete (nanoudr::transaction*)(t);
		for (auto s : outer->statements.stmt()) if (s->connection() == conn) delete (nanoudr::statement*)(s);
		for (auto r : outer->results.rslt()) if (r->connection() == conn) delete (nanoudr::result*)(r);
		conn_v.erase(conn_it);
	}
}

bool attachment_resources::attachment_connections::valid(const nanoudr::connection* conn)
{
	return std::find(conn_v.begin(), conn_v.end(), conn) != conn_v.end();
}

std::vector<nanoudr::connection*>& attachment_resources::attachment_connections::conn()
{
	return conn_v;
}

//-----------------------------------------------------------------------------
//

void attachment_resources::connection_transactions::retain(const nanoudr::transaction* tnx)
{
	tnx_v.push_back((nanoudr::transaction*)(tnx));
}

void attachment_resources::connection_transactions::release(const nanoudr::transaction* tnx)
{
	tnx_it = std::find(tnx_v.begin(), tnx_v.end(), tnx);
	if (tnx_it != tnx_v.end()) tnx_v.erase(tnx_it);
}

bool attachment_resources::connection_transactions::valid(const nanoudr::transaction* tnx)
{
	return std::find(tnx_v.begin(), tnx_v.end(), tnx) != tnx_v.end();
}

std::vector<nanoudr::transaction*>& attachment_resources::connection_transactions::tnx()
{
	return tnx_v;
}

//-----------------------------------------------------------------------------
//

void attachment_resources::connection_statements::retain(const nanoudr::statement* stmt)
{
	stmt_v.push_back((nanoudr::statement*)(stmt));
}

void attachment_resources::connection_statements::release(const nanoudr::statement* stmt)
{
	stmt_it = std::find(stmt_v.begin(), stmt_v.end(), stmt);
	if (stmt_it != stmt_v.end()) stmt_v.erase(stmt_it);
}

bool attachment_resources::connection_statements::valid(const nanoudr::statement* stmt)
{
	return std::find(stmt_v.begin(), stmt_v.end(), stmt) != stmt_v.end();
}

std::vector<nanoudr::statement*>& attachment_resources::connection_statements::stmt()
{
	return stmt_v;
}

//-----------------------------------------------------------------------------
//

void attachment_resources::connection_results::retain(const nanoudr::result* rslt)
{
	rslt_v.push_back((nanoudr::result*)(rslt));
}

void attachment_resources::connection_results::release(const nanoudr::result* rslt)
{
	rslt_it = std::find(rslt_v.begin(), rslt_v.end(), rslt);
	if (rslt_it != rslt_v.end()) rslt_v.erase(rslt_it);
}

bool attachment_resources::connection_results::valid(const nanoudr::result* rslt)
{
	return std::find(rslt_v.begin(), rslt_v.end(), rslt) != rslt_v.end();
}

std::vector<nanoudr::result*>& attachment_resources::connection_results::rslt()
{
	return rslt_v;
}

//-----------------------------------------------------------------------------
//

const long attachment_resources::exception_number(const char* name) // simple find num
{
	for (auto x : att_exceptions) 
		if (strcmp(x.name, name) == 0) return x.number;
	return 0;
}

const char* attachment_resources::exception_message(const char* name) // simple find msg
{
	for (auto &x : att_exceptions)
		if (strcmp(x.name, name) == 0) return x.message;
	return nullptr;
}

void attachment_resources::assign_exception(exception* att_exception, const short pos)
{
	memcpy_s(
		&att_exceptions[pos], sizeof(att_exceptions[pos]), att_exception, sizeof(exception)
	);
}

//-----------------------------------------------------------------------------
//  Shared UDR resources
//

resources::resources()
{
}

resources::~resources() noexcept
{
	for (att_it = att_m.begin(); att_it != att_m.end(); ++att_it)
	{
		delete (attachment_resources*)(att_it->second);
		att_m.erase(att_it);
	}
}

void resources::initialize(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	ISC_UINT64 àttachment_id = attachment_id(status, context);
	att_it = att_m.find(àttachment_id);
	if (att_it == att_m.end())
	{
		attachment_resources* att_resources = new attachment_resources(àttachment_id);
		att_m.insert(
			std::pair<ISC_UINT64, nanoudr::attachment_resources*>(àttachment_id, att_resources));
		att_resources->context(status, context);
		att_resources->make_resources();
	}
}

void resources::finalize(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	ISC_UINT64 àttachment_id = attachment_id(status, context);
	att_it = att_m.find(àttachment_id);
	if (att_it != att_m.end())
	{
		delete (attachment_resources*)(att_it->second);
		att_m.erase(att_it);
	}
}

attachment_resources* resources::attachment(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	attachment_resources* att_resources;
	att_it = att_m.find(attachment_id(status, context));
	att_resources = (att_it == att_m.end() ? nullptr : att_it->second);
	if (att_resources != nullptr) att_resources->context(status, context); // assign current pointer
	return att_resources;
}

ISC_UINT64 resources::attachment_id(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	ISC_UINT64 àttachment_id = 0;

	try
	{
		const ISC_UCHAR info[] = { isc_info_attachment_id, isc_info_end };
		ISC_UCHAR result[16];

		(context->getAttachment(status))->
			getInfo(status, sizeof(info), info, sizeof(result), result);

		ISC_UCHAR* p = result;
		if (*p++ == isc_info_attachment_id)
		{
			const ISC_USHORT l =
				(ISC_USHORT)(isc_vax_integer((const ISC_SCHAR*)(p), 2));
			p += 2;
			àttachment_id = isc_portable_integer(p, l);
		}
		else
			throw;
	}
	catch (...)
	{
		throw("Get info for attachment ID failed.");
	}

	return àttachment_id;
}

//-----------------------------------------------------------------------------
// package nano$udr
//

//-----------------------------------------------------------------------------
// create function initialize returns ty$nano_blank
//	external name 'nano!udr$initialize'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(udr$initialize)

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{ 
		attachment_resources* att_resources = nullptr;
		out->blankNull = FB_TRUE;
		try
		{
			udr_resources.initialize(status, context);
			att_resources = udr_resources.attachment(status, context);
			if (udr_resources.resource_exception.number == 0)
			{
				udr_resources.resource_exception.number = 
					att_resources->exception_number(udr_resources.resource_exception.name);
				memcpy(
					udr_resources.resource_exception.message, att_resources->exception_message(udr_resources.resource_exception.name),
					sizeof(udr_resources.resource_exception.message));
			}
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			ANY_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function finalize returns ty$nano_blank
//	external name 'nano!udr$finalize'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(udr$finalize)

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		attachment_resources* att_resources = nullptr;
		out->blankNull = FB_TRUE;
		try
		{
			udr_resources.finalize(status, context);
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			ANY_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function expunge returns ty$nano_blank
//	external name 'nano!udr$expunge'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(udr$expunge)

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		try
		{
			att_resources->expunge();
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			ANY_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function locale (
//   set_locale varchar(20) character set none default null
//	) returns varchar(20)
//	external name 'nano!udr$locale'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(udr$locale)

	FB_UDR_MESSAGE(
		InMessage,
		(FB_VARCHAR(20), set_locale)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(20), locale)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->localeNull = FB_TRUE;
		try
		{
			if (!in->set_localeNull) att_resources->locale(in->set_locale.str);
			FB_VARIYNG(out->locale, std::string(att_resources->locale()));
			out->localeNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			ANY_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function error_message
//	returns varchar(512) character set utf8
//	external name 'nano!udr$error_message'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(udr$error_message)

	unsigned out_count;

	enum out : short {
		e_msg = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> out_metadata(metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		out_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			out_char_sets[i] = out_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(512 * 4), e_msg)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->e_msgNull = FB_FALSE;
		std::string e_msg = att_resources->error_message();
		FB_VARIYNG(out->e_msg, e_msg); 
		U8_VARIYNG(out, e_msg);
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr
