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
 // Used resourses of heap
 // 

namespace nanoudr
{

//-----------------------------------------------------------------------------
//

resours::resours()
{
	ready_ = false;
	udr_locale = "cp1251";
};

resours::~resours() 
{

}

const char* resours::locale(const char* set_locale)
{
	if (set_locale) 
		udr_locale = set_locale;
	return udr_locale.c_str();
}

const char* resours::error_message(const char* last_error_message)
{
	if (last_error_message) 
		udr_error_message = last_error_message;
	return udr_error_message.c_str();
}

void resours::retain_connection(nanoudr::connection* conn)
{
	connections.push_back(conn);
}

bool resours::is_valid_connection(nanoudr::connection* conn)
{
	return find(connections.begin(), connections.end(), conn) != connections.end();
}

void resours::release_connection(nanoudr::connection* conn)
{
	std::vector<nanoudr::connection*>::iterator
		it = std::find(connections.begin(), connections.end(), conn);
	if (it != connections.end()) 
	{
		for (auto s : statements) 
			if (s->connection() == conn) release_statement(s);
		delete (nanoudr::connection*)(conn);
		connections.erase(it);
	}
}

void resours::retain_statement(nanoudr::statement* stmt)
{
	statements.push_back(stmt);
}

bool resours::is_valid_statement(nanoudr::statement* stmt)
{
	return find(statements.begin(), statements.end(), stmt) != statements.end();
}

void resours::release_statement(nanoudr::statement* stmt)
{
	std::vector<nanoudr::statement*>::iterator
		it = std::find(statements.begin(), statements.end(), stmt);
	if (it != statements.end())
	{
		delete (nanoudr::statement*)(stmt);
		statements.erase(it);
	}
}

const long resours::exception_number(const char* name)
{
	for (short pos = 0; pos < sizeof(udr_exceptions); ++pos)
		if (strcmp(udr_exceptions[pos].name, name) == 0) 
			return udr_exceptions[pos].number;
	return 0;
}

const char* resours::exception_message(const char* name)
{
	for (short pos = 0; pos < sizeof(udr_exceptions); ++pos)
		if (strcmp(udr_exceptions[pos].name, name) == 0)
			return udr_exceptions[pos].message;
	return nullptr;
}

void resours::assign_exception(exception* udr_exception, short pos)
{
	memcpy(udr_exceptions[pos].name, udr_exception->name, sizeof(udr_exceptions[pos].name));
	udr_exceptions[pos].number = udr_exception->number;
	memcpy(udr_exceptions[pos].message, udr_exception->message, sizeof(udr_exceptions[pos].message));
}

void resours::make_ready(FB_UDR_STATUS_TYPE* status, ::Firebird::IExternalContext* context)
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

	ready_ = false;
	try
	{ 
		att = context->getAttachment(status);
		tra = context->getTransaction(status);
		stmt.reset(att->prepare(status, tra, 0, sql_stmt, SQL_DIALECT_CURRENT, IStatement::PREPARE_PREFETCH_METADATA));
		meta.reset(stmt->getOutputMetadata(status));
		curs.reset(stmt->openCursor(status, tra, NULL, NULL, meta, 0));
		unsigned buf_length = meta->getMessageLength(status);
		unsigned char* buffer = new unsigned char[buf_length];
		nanoudr::exception udr_exception = {"", 0, ""};
		for (int i = 0; i < EXCEPTION_ARRAY_SIZE; ++i)
		{
			udr_resours.assign_exception(&udr_exception, i);
		}
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
			udr_resours.assign_exception(&udr_exception, i);
		}
		curs->close(status);
		ready_ = true;
	}
	catch (...)
	{
		throw("Make exceptions crashed.");
	}
}

bool resours::ready()
{
	return ready_;
}

//-----------------------------------------------------------------------------
// package nano$udr
//

//-----------------------------------------------------------------------------
// create function make_ready (
//	  udr_locale varchar(20) character set none not null default 'cp1251',
//	) returns ty$nano_blank
//	external name 'nano!make_ready'
//	engine udr; 
//
FB_UDR_BEGIN_FUNCTION(make_ready)
	
	FB_UDR_MESSAGE(
		InMessage,
		(FB_VARCHAR(20), udr_locale)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		try
		{
			udr_resours.make_ready(status, context);
			if (!in->udr_localeNull) udr_resours.locale(in->udr_locale.str);
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
// create function set_locale (
//   udr_locale varchar(20) character set none not null default 'cp1251',
//	) returns ty$nano_blank
//	external name 'nano!set_locale'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(set_locale)

FB_UDR_MESSAGE(
	InMessage,
	(FB_VARCHAR(20), udr_locale)
);

FB_UDR_MESSAGE(
	OutMessage,
	(NANO_BLANK, blank)
);

FB_UDR_EXECUTE_FUNCTION
{
	if (!in->udr_localeNull)
	{
		out->blankNull = FB_TRUE;
		try
		{
			udr_resours.locale(in->udr_locale.str);
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
		FB_STRING(out->e_msg, (std::string)(udr_resours.error_message()));
		out->e_msgNull = FB_FALSE;
		UTF8_OUT(e_msg);
	}

FB_UDR_END_FUNCTION


} // namespace nanoudr
