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

attachment_resources::attachment_resources(ISC_UINT64 attachment_id)
{
	attachment_id_ = attachment_id;
	att_locale = "cp1251";
};

attachment_resources::~attachment_resources() noexcept
{
	attachment_resources::expunge();
}

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

void attachment_resources::initialize(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context)
{
	make_exceptions(status, context);
}

void attachment_resources::make_exceptions(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context)
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
	}
	catch (...)
	{
		throw("Make exceptions crashed.");
	}
}

//-----------------------------------------------------------------------------
//	

attachment_resources::attachment_connections::attachment_connections(nanoudr::attachment_resources* owner)
{
	owner_ = owner;
}

std::vector<nanoudr::connection*>& attachment_resources::attachment_connections::conn()
{
	return conn_;
}

void attachment_resources::attachment_connections::retain(nanoudr::connection* conn)
{
	conn_.push_back(conn);
}

void attachment_resources::attachment_connections::expunge(nanoudr::connection* conn)
{
	std::vector<nanoudr::connection*>::iterator it = std::find(conn_.begin(), conn_.end(), conn);
	if (it != conn_.end())
	{
		for (auto t : owner_->transactions.tnx()) 
			if (t->connection() == conn) owner_->transactions.release(t);
		for (auto s : owner_->statements.stmt())
			if (s->connection() == conn) owner_->statements.release(s);
		for (auto r : owner_->results.rslt())
			if (r->connection() == conn) owner_->results.release(r);
	}
}

void attachment_resources::attachment_connections::release(nanoudr::connection* conn)
{
	std::vector<nanoudr::connection*>::iterator it = std::find(conn_.begin(), conn_.end(), conn);
	if (it != conn_.end())
	{
		for (auto t : owner_->transactions.tnx())
			if (t->connection() == conn) owner_->transactions.release(t);
		for (auto s : owner_->statements.stmt())
			if (s->connection() == conn) owner_->statements.release(s);
		for (auto r : owner_->results.rslt())
			if (r->connection() == conn) owner_->results.release(r);
		delete (nanoudr::connection*)(conn);
		conn_.erase(it);
	}
}

bool attachment_resources::attachment_connections::is_valid(nanoudr::connection* conn)
{
	return find(conn_.begin(), conn_.end(), conn) != conn_.end();
}

//-----------------------------------------------------------------------------
//

std::vector<nanoudr::transaction*>& attachment_resources::connection_transactions::tnx()
{
	return tnx_;
}

void attachment_resources::connection_transactions::retain(nanoudr::transaction* tnx)
{
	tnx_.push_back(tnx);
}

void attachment_resources::connection_transactions::release(nanoudr::transaction* tnx)
{
	std::vector<nanoudr::transaction*>::iterator it = std::find(tnx_.begin(), tnx_.end(), tnx);
	if (it != tnx_.end())
	{
		delete (nanoudr::transaction*)(tnx);
		tnx_.erase(it);
	}
}

bool attachment_resources::connection_transactions::is_valid(nanoudr::transaction* tnx)
{
	return find(tnx_.begin(), tnx_.end(), tnx) != tnx_.end();
}

//-----------------------------------------------------------------------------
//

std::vector<nanoudr::statement*>& attachment_resources::connection_statements::stmt()
{
	return stmt_;
}

void attachment_resources::connection_statements::retain(nanoudr::statement* stmt)
{
	stmt_.push_back(stmt);
}

void attachment_resources::connection_statements::release(nanoudr::statement* stmt)
{
	std::vector<nanoudr::statement*>::iterator it = std::find(stmt_.begin(), stmt_.end(), stmt);
	if (it != stmt_.end())
	{
		delete (nanoudr::statement*)(stmt);
		stmt_.erase(it);
	}
}

bool attachment_resources::connection_statements::is_valid(nanoudr::statement* stmt)
{
	return find(stmt_.begin(), stmt_.end(), stmt) != stmt_.end();
}

//-----------------------------------------------------------------------------
//

std::vector<nanoudr::result*>& attachment_resources::connection_results::rslt()
{
	return rslt_;
}

void attachment_resources::connection_results::retain(nanoudr::result* rslt)
{
	rslt_.push_back(rslt);
}

void attachment_resources::connection_results::release(nanoudr::result* rslt)
{
	std::vector<nanoudr::result*>::iterator it = std::find(rslt_.begin(), rslt_.end(), rslt);
	if (it != rslt_.end())
	{
		delete (nanoudr::result*)(rslt);
		rslt_.erase(it);
	}
}

bool attachment_resources::connection_results::is_valid(nanoudr::result* rslt)
{
	return find(rslt_.begin(), rslt_.end(), rslt) != rslt_.end();
}

//-----------------------------------------------------------------------------
//

void attachment_resources::expunge()
{
	for (auto c : connections.conn()) connections.release(c); 
	for (auto s : statements.stmt()) statements.release(s); // nullptr conn_
}

const long attachment_resources::exception_number(const char* name) // simple find num
{
	for (auto x : udr_exceptions) 
		if (strcmp(x.name, name) == 0) return x.number;
	return 0;
}

const char* attachment_resources::exception_message(const char* name) // simple find msg
{
	for (auto &x : udr_exceptions)
		if (strcmp(x.name, name) == 0) return x.message;
	return nullptr;
}

void attachment_resources::assign_exception(exception* udr_exception, short pos)
{
	memcpy(udr_exceptions[pos].name, udr_exception->name, sizeof(udr_exceptions[pos].name));
	udr_exceptions[pos].number = udr_exception->number;
	memcpy(udr_exceptions[pos].message, udr_exception->message, sizeof(udr_exceptions[pos].message));
}

//-----------------------------------------------------------------------------
//  Shared UDR resources
//

resources::resources()
{
	attachments.insert( 
		std::pair<ISC_UINT64, nanoudr::attachment_resources*>(0, new attachment_resources(0))
	);
}

resources::~resources() noexcept
{
	for (it_att_ = attachments.begin(); it_att_ != attachments.end(); ++it_att_)
	{
		delete (attachment_resources*)(it_att_->second);
		attachments.erase(it_att_);
	}
}

attachment_resources* resources::attachment(
	FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context, const bool read_only)
{
	nanoudr::attachment_resources* att_rsrs_ = nullptr;

	try
	{
		ISC_UINT64 ąttachment_id = attachment_id(status, context);
		it_att_ = attachments.find(ąttachment_id);
		if (it_att_ == attachments.end())
		{
			if (!read_only) 
			{
				att_rsrs_ = new attachment_resources(ąttachment_id);
				attachments.insert(
					std::pair<ISC_UINT64, nanoudr::attachment_resources*>(ąttachment_id, att_rsrs_));
				att_rsrs_->initialize(status, context);
			}	
		}
		else
			att_rsrs_ = it_att_->second;
	}
	catch (...)
	{
	}

	return att_rsrs_;
}

void resources::expunge(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context)
{
	try
	{
		ISC_UINT64 ąttachment_id = attachment_id(status, context);
		it_att_ = attachments.find(ąttachment_id);
		if (it_att_ != attachments.end())
		{
			delete (attachment_resources*)(it_att_->second);
			attachments.erase(it_att_);
		}
	}
	catch (...)
	{
	}
}

ISC_UINT64 resources::attachment_id(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context)
{
	ISC_UINT64 ąttachment_id = 0;

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
			ąttachment_id = isc_portable_integer(p, l);
		}
		else
			throw;
	}
	catch (...)
	{
		throw("Attachment ID missing.");
	}

	return ąttachment_id;
}

//-----------------------------------------------------------------------------
// package nano$udr
//

//-----------------------------------------------------------------------------
// create function change_locale (
//   set_locale varchar(20) character set none not null default 'cp1251',
//	) returns ty$nano_blank
//	external name 'nano!change_locale'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(locale)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		InMessage,
		(FB_VARCHAR(20), set_locale)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		if (!in->set_localeNull)
		{
			try
			{
				att_resources->locale(in->set_locale.str);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function error_message
//	returns varchar(512) character set utf8
//	external name 'nano!error_message'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(error_message)

	nanoudr::attachment_resources* att_resources;

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
		out->e_msgNull = FB_FALSE;

		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)
		
		std::string e_msg = att_resources->error_message();
		FB_VARIYNG(out->e_msg, e_msg); 
		U8_VARIYNG(out, e_msg);
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function expunge_connections returns ty$nano_blank
//	external name 'nano!expunge_connections'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(expunge_connections)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)
		
		out->blankNull = FB_TRUE;
		try
		{
			att_resources->expunge();
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			NANODBC_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function expunge_resources returns ty$nano_blank
//	external name 'nano!expunge_resources'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(expunge_resources)

	nanoudr::attachment_resources* att_resources;

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		att_resources = udr_resources.attachment(status, context);
		if (!att_resources)
			NANOUDR_THROW(RESOURCES_INDEFINED)

		out->blankNull = FB_TRUE;
		try
		{
			udr_resources.expunge(status, context);
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		catch (std::runtime_error const& e)
		{
			NANODBC_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr
