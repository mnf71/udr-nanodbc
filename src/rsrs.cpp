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
	attachment_context.current_transaction = nullptr;
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

ITransaction* attachment_resources::current_transaction(ITransaction* set_transaction)
{
	if (set_transaction) attachment_context.current_transaction = set_transaction;
	return 
		!attachment_context.current_transaction ?
			attachment_context.context->getTransaction(attachment_context.status) :
			attachment_context.current_transaction;  
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
	AutoRelease<IAttachment> att;
	AutoRelease<ITransaction> tra;
	AutoRelease<IStatement> stmt;
	AutoRelease<IMessageMetadata> meta;
	AutoRelease<IResultSet> curs;

	enum sql_rslt : short { name = 0, number, message };

	const char* sql_stmt = "\
SELECT CAST(TRIM(ex.rdb$exception_name) AS VARCHAR(63)) AS name,\
       ex.rdb$exception_number as number, ex.rdb$message as message\
  FROM rdb$exceptions ex\
  WHERE TRIM(ex.rdb$exception_name) STARTING WITH 'NANO$'";

	try
	{
		att.reset(context->getAttachment(status));
		tra.reset(context->getTransaction(status));
		stmt.reset(att->prepare(status, tra, 0, sql_stmt, SQL_DIALECT_CURRENT, IStatement::PREPARE_PREFETCH_METADATA));
		meta.reset(stmt->getOutputMetadata(status));
		curs.reset(stmt->openCursor(status, tra, NULL, NULL, meta, 0));

		AutoArrayDelete<unsigned char> buffer;
		buffer.reset(new unsigned char[meta->getMessageLength(status)]);
		nanoudr::exception udr_exception;
		for (short i = 0; i < EXCEPTION_ARRAY_SIZE; ++i)
		{
			memset(&udr_exception, '\0', sizeof(nanoudr::exception));
			if(!curs->isEof(status) && curs->fetchNext(status, buffer) == IStatus::RESULT_OK)
			{
				memcpy( // this SQL_VARYING, see sql_stmt
					udr_exception.name,
					(buffer + 2 + meta->getOffset(status, sql_rslt::name)),
					meta->getLength(status, sql_rslt::name) - 2);
				udr_exception.number = *(reinterpret_cast<ISC_LONG*>(buffer + meta->getOffset(status, sql_rslt::number)));
				memcpy( // SQL_VARYING
					udr_exception.message,
					(buffer + 2 + meta->getOffset(status, sql_rslt::message)),
					meta->getLength(status, sql_rslt::message) - 2);
			}
			assign_exception(&udr_exception, i);
		}
		curs->close(status); 
	}
	catch (...)
	{
		throw std::runtime_error("Make exceptions crashed.");
	}
}

//-----------------------------------------------------------------------------
//	

resources udr_resources;

void attachment_resources::attachment_connections::retain(const nanoudr::connection* conn)
{
	conn_v.push_back(const_cast<nanoudr::connection*>(conn));
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
	tnx_v.push_back(const_cast<nanoudr::transaction*>(tnx));
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
	stmt_v.push_back(const_cast<nanoudr::statement*>(stmt));
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
	rslt_v.push_back(const_cast<nanoudr::result*>(rslt));
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

const ISC_LONG attachment_resources::exception_number(const char* name) // simple find num
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
	memcpy(&att_exceptions[pos], att_exception, sizeof(exception));
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
	ISC_UINT64 att_id = attachment_id(status, context);
	att_it = att_m.find(att_id);
	if (att_it == att_m.end())
	{
		attachment_resources* att_resources = new attachment_resources(att_id);
		att_m.insert(
			std::pair<ISC_UINT64, nanoudr::attachment_resources*>(att_id, att_resources));
		att_resources->context(status, context);
		att_resources->make_resources();
	}
}

void resources::finalize(FB_UDR_STATUS_TYPE* status, FB_UDR_CONTEXT_TYPE* context)
{
	ISC_UINT64 att_id = attachment_id(status, context);
	att_it = att_m.find(att_id);
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
	ISC_UINT64 attachment_id = 0;

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
				static_cast<ISC_USHORT>(isc_vax_integer(reinterpret_cast<const ISC_SCHAR*>(p), 2));
			p += 2;
			attachment_id = isc_portable_integer(p, l);
		}
		else
			throw;
	}
	catch (...)
	{
		throw std::runtime_error("Get info for attachment ID failed.");
	}

	return attachment_id;
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
			NANOUDR_THROW(e.what())
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
			NANOUDR_THROW(e.what())
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		try
		{
			att_resources->expunge();
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			NANOUDR_THROW(e.what())
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
		ATTACHMENT_RESOURCES
		out->localeNull = FB_TRUE;
		try
		{
			if (!in->set_localeNull) att_resources->locale(in->set_locale.str);
			FB_VARIYNG(out->locale, std::string(att_resources->locale()))
			out->localeNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			NANOUDR_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function convert_[var]char (
//   value_ [var]char() character set none,
//   from_ varchar(20) character set none not null,
//   to_ varchar(20) character set none not null,
//   convert_size smallint not null default 0
//  ) returns [var]char() character set none
//  external name 'nano!udr$convert'
//  engine udr;
//

FB_UDR_BEGIN_FUNCTION(udr$convert)

	unsigned in_count;

	enum in : short {
		value = 0, from, to, convert_size
	};

	const unsigned out_count = 1;

	enum out : short {
		result = 0
	};

	AutoArrayDelete<unsigned> in_types;
	AutoArrayDelete<unsigned> in_lengths;
	AutoArrayDelete<unsigned> in_offsets;
	AutoArrayDelete<unsigned> in_null_offsets;

	AutoArrayDelete<unsigned> out_type;
	AutoArrayDelete<unsigned> out_length;
	AutoArrayDelete<unsigned> out_offset;
	AutoArrayDelete<unsigned> out_null_offset;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));
		AutoRelease<IMessageMetadata> out_metadata(metadata->getOutputMetadata(status));

		in_count = in_metadata->getCount(status);

		in_types.reset(new unsigned[in_count]);
		in_lengths.reset(new unsigned[in_count]);
		in_offsets.reset(new unsigned[in_count]);
		in_null_offsets.reset(new unsigned[in_count]);

		for (unsigned i = 0; i < in_count; ++i)
		{
			in_types[i] = in_metadata->getType(status, i);
			in_lengths[i] = in_metadata->getLength(status, i);
			in_offsets[i] = in_metadata->getOffset(status, i);
			in_null_offsets[i] = in_metadata->getNullOffset(status, i);
		}

		out_type.reset(new unsigned[out_count]);
		out_length.reset(new unsigned[out_count]);
		out_offset.reset(new unsigned[out_count]);
		out_null_offset.reset(new unsigned[out_count]);

		out_type[out::result] = out_metadata->getType(status, out::result);
		out_length[out::result] = out_metadata->getLength(status, out::result);
		out_offset[out::result] = out_metadata->getOffset(status, out::result);
		out_null_offset[out::result] = out_metadata->getNullOffset(status, out::result);
	}

	FB_UDR_EXECUTE_FUNCTION
	{
		ATTACHMENT_RESOURCES
		*(reinterpret_cast<ISC_SHORT*>(out + out_null_offset[out::result])) = FB_TRUE;
		if (!*(reinterpret_cast<ISC_SHORT*>(in + in_null_offsets[in::value])) ||
			!(in_types[in::value] == SQL_TEXT || in_types[in::value] == SQL_VARYING) ||
			!(out_type[out::result] == SQL_TEXT || out_type[out::result] == SQL_VARYING))
		{
			try {
				ISC_USHORT length = 
					static_cast<ISC_USHORT> (
						(in_types[in::value] == SQL_TEXT ?
							in_lengths[in::value] :	// полный размер переданного CHAR(N) с учетом пробелов 
							*(reinterpret_cast<ISC_USHORT*>(in + in_offsets[in::value]))
						)
					);

				ISC_USHORT convert_size = *(reinterpret_cast<ISC_SHORT*>(in + in_offsets[in::convert_size]));
				if (convert_size < 0) 
					throw std::runtime_error("CONVERT_SIZE, expected zero or positive value.");
				convert_size = (convert_size == 0 || convert_size > length) ? length : convert_size;

				convert_size =
					udr_helper.unicode_converter(
						reinterpret_cast<char*>(out + (out_type[out::result] == SQL_TEXT ? 0 : sizeof(ISC_USHORT)) + out_offset[out::result]),
						static_cast<ISC_USHORT>(out_length[out::result]), reinterpret_cast<const char*>(in + sizeof(ISC_USHORT) + in_offsets[in::to]),
						reinterpret_cast<const char*>(in + (in_types[out::result] == SQL_TEXT ? 0 : sizeof(ISC_USHORT)) + in_offsets[in::value]),
						convert_size, reinterpret_cast<const char*>(in + sizeof(ISC_USHORT) + in_offsets[in::from])
					);

				if (out_type[out::result] == SQL_TEXT)
				{
					if (out_length[out::result] > convert_size)
						memset(
							reinterpret_cast<char*>(out + out_offset[out::result]) + convert_size, ' ', out_length[out::result] - convert_size
						);
				}
				else
					*(reinterpret_cast<ISC_USHORT*>(out + out_offset[out::result])) = convert_size;
			
				*(reinterpret_cast<ISC_SHORT*>(out + out_null_offset[out::result])) = FB_FALSE;
			}	
			catch (std::runtime_error const& e) 
			{
				NANOUDR_THROW(e.what())
			}
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
		ATTACHMENT_RESOURCES
		out->e_msgNull = FB_FALSE;
		std::string e_msg = att_resources->error_message();
		FB_VARIYNG(out->e_msg, e_msg)
		U8_VARIYNG(out, e_msg)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr
