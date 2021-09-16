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
// package nano$stmt
//

namespace nanoudr
{

//-----------------------------------------------------------------------------
// Params batch (values pointers for binding)
//

params_batch::params_batch(short count)
{
	params = new param[count];
	count_ = count;
}

params_batch::~params_batch() noexcept
{
	clear();
	delete[] params;
}

template <class T>
long params_batch::push(short param_index, T const value, const bool null)
{
	param* p = &params[param_index];
	
	bind_types& vec = p->batch;
	if (!std::holds_alternative<std::vector<T>>(vec)) 
		vec = std::vector<T>{ value }; // init allocator
	else		
		std::get<std::vector<T>>(vec).push_back(value);
	p->nulls.push_back(null);
	
	return static_cast<long>(p->nulls.size() - 1); 
}

void params_batch::clear()
{
	for (std::size_t param_index = 0; param_index < count_; ++param_index)
	{
		param* p = &params[param_index];
		std::visit([](auto&& arg) { arg.clear(); }, p->batch);
		p->nulls.clear();
	}
}

bind_type params_batch::touch(short param_index)
{
	bind_types& vec = params[param_index].batch;
	return
		std::holds_alternative<std::vector<short>>(vec) ? bind_type::NANODBC_SHORT
		: std::holds_alternative<std::vector<unsigned short>>(vec) ? bind_type::NANODBC_USHORT
		: std::holds_alternative<std::vector<int>>(vec) ? bind_type::NANODBC_INT
		: std::holds_alternative<std::vector<unsigned int>>(vec) ? bind_type::NANODBC_UINT
		: std::holds_alternative<std::vector<long int>>(vec) ? bind_type::NANODBC_LONG
		: std::holds_alternative<std::vector<unsigned long int>>(vec) ? bind_type::NANODBC_ULONG
		: std::holds_alternative<std::vector<long long>>(vec) ? bind_type::NANODBC_INT64 
		: std::holds_alternative<std::vector<unsigned long long>>(vec) ? bind_type::NANODBC_UINT64
		: std::holds_alternative<std::vector<float>>(vec) ? bind_type::NANODBC_FLOAT
		: std::holds_alternative<std::vector<double>>(vec) ? bind_type::NANODBC_DOUBLE
		: std::holds_alternative<std::vector<nanodbc::date>>(vec) ? bind_type::NANODBC_DATE
		: std::holds_alternative<std::vector<nanodbc::time>>(vec) ? bind_type::NANODBC_TIME
		: std::holds_alternative<std::vector<nanodbc::timestamp>>(vec) ? bind_type::NANODBC_TIMESTAMP
		: std::holds_alternative<std::vector<nanodbc::string>>(vec) ? bind_type::NANODBC_STRING
		: std::holds_alternative<std::vector<nanodbc::wide_string>>(vec) ? bind_type::NANODBC_WIDE_STRING
		: bind_type::NANODBC_UNKNOWN;
}

template <typename T>
T* params_batch::data(short param_index)
{
	bind_types& vec = params[param_index].batch;
	return std::get<std::vector<T>>(vec).data();
}

template <class T>
T* params_batch::value(short param_index, long batch_index)
{
	bind_types& vec = params[param_index].batch;
	T* v = std::get<std::vector<T>>(vec).data();
	return &(v[batch_index]);
}

template <class T>
std::vector<T>* params_batch::batch(short param_index)
{
	bind_types& vec = params[param_index].batch;
	return &(std::get<std::vector<T>>(vec));
}

bool* params_batch::nulls(short param_index)
{
	return reinterpret_cast<bool*>(params[param_index].nulls.data());
}

bool params_batch::is_null(short param_index, long batch_index)
{
	return params[param_index].nulls.at(batch_index);
}

short params_batch::count()
{
	return count_;
}

//-----------------------------------------------------------------------------
// UDR Statement class implementation
//

statement::statement() : nanodbc::statement()
{
	udr_resours.retain_statement(this);
	params_ = nullptr;
	conn_ = nullptr;
}

statement::statement(class nanoudr::connection& conn)
	: nanodbc::statement(conn)
{
	udr_resours.retain_statement(this);
	params_ = nullptr;
	conn_ = &conn;
}

statement::statement(class nanoudr::connection& conn, const nanodbc::string& query, long timeout)
	: nanodbc::statement(conn, query, timeout)
{
	udr_resours.retain_statement(this);
	prepare_params();
	conn_ = &conn;
}

statement::~statement() 
{
	release_params();
	nanodbc::statement::~statement();
}

void statement::open(class nanoudr::connection& conn)
{
	nanodbc::statement::open(conn);
	conn_ = &conn;
}

nanoudr::connection* statement::connection()
{
	return conn_;
}

void statement::prepare(class nanoudr::connection& conn, const nanodbc::string& query, long timeout)
{
	release_params();
	nanodbc::statement::prepare(conn, query, timeout);
	prepare_params();
	conn_ = &conn;
}

void statement::prepare(const nanodbc::string& query, long timeout)
{
	release_params();
	nanodbc::statement::prepare(query, timeout);
	prepare_params();
}

class nanodbc::result statement::execute_direct(
	class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations, long timeout
)
{
	nanodbc::result rslt =
		nanodbc::statement::execute_direct(conn, query, batch_operations, timeout);
	conn_ = &conn;
	return rslt;
}

void statement::just_execute_direct(
	class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations, long timeout
)
{
	nanodbc::statement::just_execute_direct(conn, query, batch_operations, timeout);
	conn_ = &conn;
}

void statement::prepare_params()
{
	params_ = nullptr;
	short parameters_count = parameters();
	if (parameters_count != 0)
		params_ = new nanoudr::params_batch(parameters_count);
}

void statement::release_params()
{
	if (params_ != nullptr) delete params_;
	params_ = nullptr;
}

void statement::bind_params(long batch_operations)
{
	if (params_)
	{
		bool batch_operation = !(batch_operations == 1);
		for (short param_index = 0; param_index < params_->count(); ++param_index)
		{
			bool* nulls_flag;
			if (!batch_operation && params_->is_null(param_index, 0))
			{
				bind_null(param_index);
				continue;
			}
			else
				nulls_flag = params_->nulls(param_index);

			switch (params_->touch(param_index))
			{
				case bind_type::NANODBC_SHORT:
				{
					if (batch_operation)
						bind(param_index, params_->data<short>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<short>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_USHORT:
				{
					if (batch_operation)
						bind(param_index, params_->data<unsigned short>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned short>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_INT:
				{
					if (batch_operation)
						bind(param_index, params_->data<int>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_UINT:
				{
					if (batch_operation)
						bind(param_index, params_->data<unsigned int>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_LONG:
				{
					if (batch_operation)
						bind(param_index, params_->data<long int>(param_index), batch_operations,	nulls_flag);
					else
						bind(param_index, params_->value<long int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_ULONG:
				{
					if (batch_operation)
						bind(param_index, params_->data<unsigned long int>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned long int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_INT64:
				{
					if (batch_operation)
						bind(param_index, params_->data<long long>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<long long>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_UINT64:
				{
					if (batch_operation)
						bind(param_index, params_->data<unsigned long long>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned long long>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_FLOAT:
				{
					if (batch_operation)
						bind(param_index, params_->data<float>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<float>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_DOUBLE:
				{
					if (batch_operation)
						bind(param_index, params_->data<double>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<double>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_DATE:
				{
					if (batch_operation)
						bind(param_index, params_->data<nanodbc::date>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::date>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_TIME:
				{
					if (batch_operation)
						bind(param_index, params_->data<nanodbc::time>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::time>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_TIMESTAMP:
				{
					if (batch_operation)
						bind(param_index, params_->data<nanodbc::timestamp>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::timestamp>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_STRING:
				{
					if (batch_operation)
						bind_strings(param_index, *(params_->batch<nanodbc::string>(param_index)), nulls_flag);
					else
						bind(param_index, (char*)(params_->value<nanodbc::string>(param_index, 0)->c_str()));
					break;
				}
				case bind_type::NANODBC_WIDE_STRING:
				{
					if (batch_operation)
						bind_strings(param_index, *(params_->batch<nanodbc::wide_string>(param_index)), nulls_flag);
					else
						bind(param_index, (nanodbc::wide_char_t*)(params_->value<nanodbc::string>(param_index, 0)->c_str()));
					break;
				}
				case bind_type::NANODBC_UNKNOWN:
				default:
				{
					throw("Binding unknow NANODBC datatype .");
					break;
				}
			}
		}
	}
}

params_batch* statement::params()
{
	return params_;
}

//-----------------------------------------------------------------------------
// create function statement_ (
//	 conn ty$pointer default null, 
//   query varchar(8191) character set utf8 default null,
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt_statement'
//	engine udr; 
//
// statement (null, null, ...) returns new un-prepared statement
// statement (?, null, ...) returns statement object and associates it to the given connection
// statement (?, ?, ...) returns prepared a statement using the given connection and query
//

FB_UDR_BEGIN_FUNCTION(stmt_statement)

	unsigned in_count;

	enum in : short {
		conn = 0, query, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->stmtNull = FB_TRUE;
		try
		{
			nanoudr::statement* stmt;
			if (!in->connNull)
			{
				nanoudr::connection* conn = nanoudr::conn_ptr(in->conn.str);
				if (udr_resours.is_valid_connection(conn))
				{
					if (!in->queryNull)
					{
						UTF8_IN(query);
						stmt = new nanoudr::statement(*conn, NANODBC_TEXT(in->query.str), in->timeout);
					}
					else
						stmt = new nanoudr::statement(*conn);
				}
				else
					NANOUDR_THROW(INVALID_CONN_POINTER)
			}
			else
				stmt = new nanoudr::statement();
			nanoudr::fb_ptr(out->stmt.str, (int64_t)stmt);
			out->stmtNull = FB_FALSE;
		}	
		catch (std::runtime_error const& e)
		{
			RANDOM_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	 stmt ty$pointer not null 
// ) returns ty$pointer
// external name 'nano!stmt_release'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_release)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->stmtNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				udr_resours.release_statement(stmt);
			}
			catch (std::runtime_error const& e)
			{
				nanoudr::fb_ptr(out->stmt.str, (int64_t)stmt);
				out->stmtNull = FB_FALSE;
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}
		
FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_valid (
//	 stmt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!stmt_is_valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_is_valid)

FB_UDR_MESSAGE(
	InMessage,
	(NANO_POINTER, stmt)
);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, valid)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->valid =
			in->stmtNull ?
			nanoudr::fb_bool(false) :
			udr_resours.is_valid_statement(nanoudr::stmt_ptr(in->stmt.str));
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function open_ (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null
// ) returns ty$nano_blank
// external name 'nano!stmt_open'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_open)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			nanoudr::connection* conn = in->connNull ? nullptr : nanoudr::conn_ptr(in->conn.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					if (conn != nullptr && udr_resours.is_valid_connection(conn))
					{
						stmt->open(*conn);
						out->blank = BLANK;
						out->blankNull = FB_FALSE;
					}
					else
						NANOUDR_THROW(INVALID_CONN_POINTER)
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function opened (
//	 stmt ty$pointer not null 
//	) returns boolean
//	external name 'nano!stmt_opened'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_opened)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, opened)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->openedNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					out->opened = nanoudr::fb_bool(((nanodbc::statement*)stmt)->open());
					out->openedNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	 stmt ty$pointer not null 
//	) returns boolean
//	external name 'nano!stmt_connected'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_connected)

FB_UDR_MESSAGE(
	InMessage,
	(NANO_POINTER, stmt)
);


	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, connected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->connectedNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					out->connected = nanoudr::fb_bool(stmt->connected());
					out->connectedNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 stmt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!stmt_connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_connection)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->connNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					nanoudr::connection* conn = stmt->connection();
					nanoudr::fb_ptr(out->conn.str, (int64_t)conn);
					out->connNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function close_ (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt_close'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_close)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->close();
					stmt->release_params();
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function cancel_ (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt_cancel'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_cancel)

FB_UDR_MESSAGE(
	InMessage,
	(NANO_POINTER, stmt)
);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->cancel();
					stmt->release_params();
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null,  
//   query varchar(8191) character set utf8 not null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_prepare_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_prepare_direct)

unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(NANO_POINTER, conn)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			nanoudr::connection* conn = in->connNull ? nullptr : nanoudr::conn_ptr(in->conn.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					if (conn != nullptr && udr_resours.is_valid_connection(conn))
					{
						UTF8_IN(query);
						stmt->prepare(*conn, NANODBC_TEXT(in->query.str), in->timeout);
						out->blank = BLANK;
						out->blankNull = FB_FALSE;
					}
					else
						NANOUDR_THROW(INVALID_CONN_POINTER)
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare_ (
//	 stmt ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_prepare'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_prepare)
	
	unsigned in_count;

	enum in : short {
		stmt = 0, query, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					UTF8_IN(query);
					stmt->prepare(NANODBC_TEXT(in->query.str), in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)

			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function timeout (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_timeout'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_timeout)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->timeout(in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)

			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function execute_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1,
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt_execute_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_execute_direct)

	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, batch_operations, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(NANO_POINTER, conn)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->rsltNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			nanoudr::connection* conn = in->connNull ? nullptr : nanoudr::conn_ptr(in->conn.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					if (conn != nullptr && udr_resours.is_valid_connection(conn))
					{
						UTF8_IN(query);
						nanodbc::result rslt =
							stmt->execute_direct(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
						nanoudr::fb_ptr(out->rslt.str, (int64_t)&rslt);
						out->rsltNull = FB_FALSE;
					}
					else
						NANOUDR_THROW(INVALID_CONN_POINTER)
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
// 	 batch_operations integer not null default 1 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_just_execute_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_just_execute_direct)

	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, batch_operations, timeout
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(NANO_POINTER, conn)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			nanoudr::connection* conn = in->connNull ? nullptr : nanoudr::conn_ptr(in->conn.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					if (conn != nullptr && udr_resours.is_valid_connection(conn))
					{
						UTF8_IN(query);
						stmt->just_execute_direct(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
						out->blank = BLANK;
						out->blankNull = FB_FALSE;
					}
					else
						NANOUDR_THROW(INVALID_CONN_POINTER)
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function execute_ (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1, 
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt_execute'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_execute)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->rsltNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->bind_params(in->batch_operations);
					nanodbc::result rslt = stmt->execute(in->batch_operations, in->timeout);
					nanoudr::fb_ptr(out->rslt.str, (int64_t)&rslt);
					out->rsltNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_ (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_just_execute'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_just_execute)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->bind_params(in->batch_operations);
					stmt->just_execute(in->batch_operations, in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what());
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function procedure_columns (
//	 stmt ty$pointer not null, 
//	 catalog_ varchar(128) character set utf8 not null, 
//	 schema_ varchar(128) character set utf8 not null, 
//	 procedure_ varchar(63) character set utf8 not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns ty$pointer
//	external name 'nano!stmt_procedure_columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_procedure_columns)

unsigned in_count;

	enum in : short {
		stmt = 0, catalog_, schema_, procedure_, column_
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_VARCHAR(128 * 4), catalog_)
		(FB_VARCHAR(128 * 4), schema_)
		(FB_VARCHAR(63 * 4), procedure_)
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->rsltNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					UTF8_IN(catalog_);
					UTF8_IN(schema_);
					UTF8_IN(procedure_);
					UTF8_IN(column_);
					nanodbc::result rslt =
						stmt->procedure_columns(NANODBC_TEXT(in->catalog_.str), NANODBC_TEXT(in->schema_.str), NANODBC_TEXT(in->procedure_.str),
							NANODBC_TEXT(in->column_.str));
					nanoudr::fb_ptr(out->rslt.str, (int64_t)&rslt);
					out->rsltNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	 stmt ty$pointer not null 
//	) returns integer
//	external name 'nano!stmt_affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_affected_rows)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, affected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->affectedNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					out->affected = stmt->affected_rows();
					out->affectedNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function columns (
//	 stmt ty$pointer not null 
//	) returns smallint
//	external name 'nano!stmt_columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_columns)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, columns)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->columnsNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					out->columns = stmt->columns();
					out->columnsNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function reset_parameters (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt_reset_parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_reset_parameters)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->stmtNull)
		{
			out->blankNull = FB_TRUE;
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->reset_parameters();
					if (stmt->params() != nullptr) stmt->params()->clear();
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameters  ( 
//	 stmt ty$pointer not null 
//	) returns smallint
//	external name 'nano!stmt_parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_parameters)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, parameters)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->parametersNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					out->parameters = stmt->parameters();
					out->parametersNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameter_size  ( 
//	 stmt ty$pointer not null, 
//	 param_index smallint not null
//	) returns integer
//	external name 'nano!stmt_parameter_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_parameter_size)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, param_index)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->sizeNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					out->size = stmt->parameter_size(in->param_index);
					if (out->size < 0) out->size = -1; // BLOB
					out->sizeNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// template <class T> void bind (...
//
// create function bind_[fb_type] (
//	 stmt ty$pointer not null,
// 	 param_index smallint not null,
//   value_	[fb_type]
//	) returns ty$nano_blank
//	external name 'nano!stmt_bind'
//	engine udr; 

FB_UDR_BEGIN_FUNCTION(stmt_bind)
	
	unsigned in_count;

	enum in : short {
		stmt = 0, param_index, value
	};

	AutoArrayDelete<unsigned> in_offsets;
	AutoArrayDelete<unsigned> in_null_offsets;
	AutoArrayDelete<unsigned> in_types;
	AutoArrayDelete<unsigned> in_char_sets;

	IUtil* utl = master->getUtilInterface();

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		
		in_offsets.reset(new unsigned[in_count]);
		in_null_offsets.reset(new unsigned[in_count]);
		in_types.reset(new unsigned[in_count]);
		in_char_sets.reset(new unsigned[in_count]);

		for (unsigned i = 0; i < in_count; ++i)
		{
			in_offsets[i] = in_metadata->getOffset(status, i);
			in_null_offsets[i] = in_metadata->getNullOffset(status, i);
			in_types[i] = in_metadata->getType(status, i);
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!*(ISC_SHORT*)(in + in_null_offsets[in::stmt]))
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr((char*)(in + in_offsets[in::stmt]));
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					params_batch* params = stmt->params();
					if (params)
					{
						short param_index = *(ISC_SHORT*)(in + in_offsets[in::param_index]);
						bool null_flag = *(ISC_SHORT*)(in + in_null_offsets[in::value]) ? true : false;
						switch (in_types[in::value])
						{
						case SQL_TEXT: // char, varchar
						case SQL_VARYING:
						{
							if (null_flag)
								params->push(param_index, (nanodbc::string)(NANODBC_TEXT("\0")), null_flag);
							else
							{
								std::size_t length =
									(in_types[in::value] == SQL_TEXT ? strlen((char*)(in + in_offsets[in::value])) : *(ISC_USHORT*)(in + in_offsets[in::value])) *
									(in_char_sets[in::value] == fb_char_set::CS_UTF8 ? 4 : 1);
								char* param = new char[length + 1];
								memcpy(
									param,
									in_types[in::value] == SQL_TEXT ? (char*)(in + in_offsets[in::value]) : (char*)(in + sizeof(ISC_USHORT) + in_offsets[in::value]),
									length
								);
								param[length] = '\0';
								if (in_char_sets[in::value] == fb_char_set::CS_UTF8) utf8_to_loc(param, param);
								params->push(param_index, (nanodbc::string)(NANODBC_TEXT(param)), null_flag);
								delete[] param;
							}
							break;
						}
						case SQL_SHORT: // smallint
							params->push(param_index, *(ISC_SHORT*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_LONG: // integer 
							params->push(param_index, *(ISC_LONG*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_FLOAT: // float
							params->push(param_index, *(float*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_DOUBLE: // double precision 
						case SQL_D_FLOAT:
							params->push(param_index, *(double*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_TIMESTAMP: // timestamp
						{
							nanodbc::timestamp param;
							if (null_flag)
							{
								param.year = 1900; param.month = 1;	param.day = 1;
								param.hour = 23; param.min = 59; param.sec = 59; param.fract = 0;
							}
							else
							{
								Firebird::FbTimestamp fb;
								// This class has memory layout identical to ISC_TIMESTAMP
								memcpy(&fb, (ISC_TIMESTAMP*)(in + in_offsets[in::value]), sizeof(FbTimestamp));
								struct nanoudr::timestamp tm;
								fb.date.decode(utl, &tm.year, &tm.month, &tm.day);
								fb.time.decode(utl, &tm.hour, &tm.min, &tm.sec, &tm.fract);
								param = nanoudr::set_timestamp(&tm);
							}
							params->push(param_index, param, null_flag);
							break;
						}
						case SQL_BLOB: // blob
						{
							if (null_flag)
								params->push(param_index, (nanodbc::string)(NANODBC_TEXT("\0")), null_flag);
							else
							{
							}
							break;
						}
						case SQL_ARRAY: // array
							throw("Binding SQL_ARRAY datatype not support.");
							break;
						case SQL_QUAD: // blob_id 
							break;
						case SQL_TYPE_TIME: // time
						{
							nanodbc::time param;
							if (null_flag)
							{
								param.hour = 23; param.min = 59; param.sec = 59;
							}
							else
							{
								Firebird::FbTime fb;
								// This class has memory layout identical to ISC_TIME
								memcpy(&fb, (ISC_TIME*)(in + in_offsets[in::value]), sizeof(FbTime));
								struct nanoudr::time t;
								fb.decode(utl, &t.hour, &t.min, &t.sec, &t.fract);
								nanodbc::time param = nanoudr::set_time(&t);
							}
							params->push(param_index, param, null_flag);
							break;
						}
						case SQL_TYPE_DATE: // date
						{
							nanodbc::date param;
							if (null_flag)
							{
								param.year = 1900; param.month = 1;	param.day = 1;
							}
							else
							{
								Firebird::FbDate fb;
								// This class has memory layout identical to ISC_DATE
								memcpy(&fb, (ISC_DATE*)(in + in_offsets[in::value]), sizeof(FbDate));
								struct nanoudr::date d;
								fb.decode(utl, &d.year, &d.month, &d.day);
								nanodbc::date param = nanoudr::set_date(&d);
							}
							params->push(param_index, param, null_flag);
							break;
						}
						case SQL_INT64: // bigint
							params->push(param_index, *(ISC_INT64*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_BOOLEAN: // boolean
						{
							bool param;
							if (null_flag) param = false;
							else
								param = nanoudr::native_bool(*(FB_BOOLEAN*)(in + in_offsets[in::value]));
							params->push(param_index, (nanodbc::string)(NANODBC_TEXT(param ? "True" : "False")), null_flag);
							break;
						}
						case SQL_NULL: // null
						{
							params->push(param_index, (nanodbc::string)(NANODBC_TEXT("\0")), true);
							break;
						}
						default:
						{
							throw("Binding unknow Firebird SQL datatype.");
							break;
						}
						}
					}
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function bind_null (
//	 stmt ty$pointer not null,
// 	 param_index smallint not null,
// 	 batch_size integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!stmt_bind_null'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_bind_null)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, param_index)
		(FB_INTEGER, batch_size)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{
					stmt->bind_null(in->param_index, in->batch_size);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function describe_parameters (
//	 stmt ty$pointer not null,
// 	 idx smallint not null,
// 	 type_ smallint not null,
// 	 size_ integer not null,
// 	 scale_ smallint not null default 0
//	) returns ty$nano_blank
//	external name 'nano!stmt_describe_parameters
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt_describe_parameters)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, idx)
		(FB_SMALLINT, type)
		(FB_INTEGER, size)
		(FB_SMALLINT, scale)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = nanoudr::stmt_ptr(in->stmt.str);
			try
			{
				if (udr_resours.is_valid_statement(stmt))
				{

					stmt->describe_parameters
					(std::vector<short>(in->idx), std::vector<short>(in->type), std::vector<unsigned long>(in->size),
						std::vector<short>(in->scale));
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_STMT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				RANDOM_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STMT_POINTER)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr

