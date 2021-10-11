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
		: std::holds_alternative<std::vector<nanodbc::wide_string::value_type>>(vec) ? bind_type::NANODBC_WSTRING_VALUE_TYPE
		: std::holds_alternative<std::vector<nanodbc::wide_string>>(vec) ? bind_type::NANODBC_WIDE_STRING
		: std::holds_alternative<std::vector<nanodbc::string::value_type>>(vec) ? bind_type::NANODBC_STRING_VALUE_TYPE
		: std::holds_alternative<std::vector<nanodbc::string>>(vec) ? bind_type::NANODBC_STRING
		: std::holds_alternative<std::vector<std::vector<uint8_t>>>(vec) ? bind_type::NANODBC_BINARY
		: bind_type::UNKNOWN;
}

template <typename T>
T* params_batch::values(const short param_index)
{
	bind_types& vec = params[param_index].batch;
	return std::get<std::vector<T>>(vec).data();
}

template <class T>
std::vector<T>* params_batch::vector(const short param_index)
{
	bind_types& vec = params[param_index].batch;
	return &(std::get<std::vector<T>>(vec));
}

template <class T>
T* params_batch::value(const short param_index, const long batch_index)
{
	bind_types& vec = params[param_index].batch;
	T* v = std::get<std::vector<T>>(vec).data();
	return &(v[batch_index]);
}

bool* params_batch::nulls(const short param_index)
{
	return reinterpret_cast<bool*>(params[param_index].nulls.data());
}

bool params_batch::is_null(const short param_index, const long batch_index)
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

statement::statement(class attachment_resources& att_resources) 
	: nanodbc::statement()
	
{
	att_resources_ = &att_resources;
	att_resources_->statements.retain(this);
	conn_ = nullptr;

	scrollable_ = scroll_state::DEFAULT;
	params_ = nullptr;
}

statement::statement(class attachment_resources& att_resources, class nanoudr::connection& conn)
	: nanodbc::statement(conn)
{
	att_resources_ = &att_resources;
	att_resources_->statements.retain(this);
	conn_ = &conn;

	scrollable_ = scroll_state::DEFAULT;
	params_ = nullptr;
}

statement::statement(
	class attachment_resources& att_resources, class nanoudr::connection& conn, const nanodbc::string& query, 
	const scroll_state scrollable_usage, long timeout)
	: scrollable_(scroll_state::DEFAULT)
	, nanodbc::statement(conn)
{
	att_resources_ = &att_resources;
	att_resources_->statements.retain(this);
	conn_ = &conn;

	scrollable(scrollable_usage);
	nanodbc::statement::prepare(query, timeout);
	prepare_params();
}

statement::~statement() 
{
	release_params();
	att_resources_->statements.release(this);
	nanodbc::statement::~statement();
}

void statement::open(class nanoudr::connection& conn)
{
	nanoudr::statement::close();
	nanodbc::statement::open(conn);
	conn_ = &conn;
}

nanoudr::connection* statement::connection()
{
	return conn_;
}

void statement::close()
{
	release_params();
	scrollable_ = scroll_state::DEFAULT;
	nanodbc::statement::close();
}

void statement::prepare(
	class nanoudr::connection& conn, const nanodbc::string& query, const scroll_state scrollable_usage, long timeout)
{
	nanoudr::statement::open(conn);
	nanoudr::statement::prepare(query, scrollable_usage, timeout);
}

void statement::prepare(const nanodbc::string& query, const scroll_state scrollable_usage, long timeout)
{
	release_params();
	scrollable(scrollable_usage);
	nanodbc::statement::prepare(query, timeout);
	prepare_params();
}

nanoudr::result* statement::execute_direct(
	class nanoudr::connection& conn, const nanodbc::string& query, const scroll_state scrollable_usage, long batch_operations, long timeout)
{
	// ------ 
	// function emulation to avoid forked nanodbc 
	// 
	nanoudr::statement::open(conn);
	nanoudr::statement::prepare(query, scrollable_usage, timeout);
	return nanoudr::statement::execute(batch_operations);
}

void statement::just_execute_direct(
	class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations, long timeout
)
{
	nanoudr::statement::close();
	nanodbc::statement::just_execute_direct(conn, query, batch_operations, timeout);
	conn_ = &conn;
}

nanoudr::result* statement::execute(long batch_operations, long timeout)
{
	nanodbc::result rslt = nanodbc::statement::execute(batch_operations, timeout);
	return new nanoudr::result(*this->attachment(), *this->connection(), std::move(rslt));
}

void statement::just_execute(long batch_operations, long timeout)
{
	nanodbc::statement::just_execute(batch_operations, timeout);
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
		attachment_resources* att_resources = att_resources_;
		FB_UDR_STATUS_TYPE* status = att_resources_->context()->status;
		FB_UDR_CONTEXT_TYPE* context = att_resources_->context()->context;
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
						bind(param_index, params_->values<short>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<short>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_USHORT:
				{
					if (batch_operation)
						bind(param_index, params_->values<unsigned short>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned short>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_INT:
				{
					if (batch_operation)
						bind(param_index, params_->values<int>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_UINT:
				{
					if (batch_operation)
						bind(param_index, params_->values<unsigned int>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_LONG:
				{
					if (batch_operation)
						bind(param_index, params_->values<long int>(param_index), batch_operations,	nulls_flag);
					else
						bind(param_index, params_->value<long int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_ULONG:
				{
					if (batch_operation)
						bind(param_index, params_->values<unsigned long int>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned long int>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_INT64:
				{
					if (batch_operation)
						bind(param_index, params_->values<long long>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<long long>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_UINT64:
				{
					if (batch_operation)
						bind(param_index, params_->values<unsigned long long>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<unsigned long long>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_FLOAT:
				{
					if (batch_operation)
						bind(param_index, params_->values<float>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<float>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_DOUBLE:
				{
					if (batch_operation)
						bind(param_index, params_->values<double>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<double>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_DATE:
				{
					if (batch_operation)
						bind(param_index, params_->values<nanodbc::date>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::date>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_TIME:
				{
					if (batch_operation)
						bind(param_index, params_->values<nanodbc::time>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::time>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_TIMESTAMP:
				{
					if (batch_operation)
						bind(param_index, params_->values<nanodbc::timestamp>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::timestamp>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_WSTRING_VALUE_TYPE:
				{
					if (batch_operation)
						bind(param_index, params_->values<nanodbc::wide_string::value_type>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::wide_string::value_type>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_WIDE_STRING:
				{
					if (batch_operation)
						bind_strings(param_index, *(params_->vector<nanodbc::wide_string>(param_index)), nulls_flag);
					else
						bind(param_index, (nanodbc::wide_char_t*)(params_->value<nanodbc::string>(param_index, 0)->c_str()));
					break;
				}
				case bind_type::NANODBC_STRING_VALUE_TYPE:
				{
					if (batch_operation) 
						bind(param_index, params_->values<nanodbc::string::value_type>(param_index), batch_operations, nulls_flag);
					else
						bind(param_index, params_->value<nanodbc::string::value_type>(param_index, 0));
					break;
				}
				case bind_type::NANODBC_STRING:
				{
					if (batch_operation)
						bind_strings(param_index, *(params_->vector<nanodbc::string>(param_index)), nulls_flag);
					else
						bind(param_index, (char*)(params_->value<nanodbc::string>(param_index, 0)->c_str()));
					break;
				}
				case bind_type::NANODBC_BINARY:
				{
					if (batch_operation)
						bind(param_index, *(params_->vector<std::vector<uint8_t>>(param_index)), nulls_flag);
					else
						bind(param_index, *(params_->vector<std::vector<uint8_t>>(param_index))); // multiple... todo
					break;
				}
				case bind_type::UNKNOWN: default:
				{
					BINDING_THROW("Not supported NANODBC data type.")
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
//   scrollable boolean default null,
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt$statement'
//	engine udr; 
//
// statement (null, null, ...) returns new un-prepared statement
// statement (?, null, ...) returns statement object and associates it to the given connection
// statement (?, ?, ...) returns prepared a statement using the given connection and query
//

FB_UDR_BEGIN_FUNCTION(stmt$statement)

	unsigned in_count;

	enum in : short {
		conn = 0, query, scrollable, timeout
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
		(FB_BOOLEAN, scrollable)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->stmtNull = FB_TRUE;
		try
		{
			nanoudr::statement* stmt;
			if (!in->connNull)
			{
				nanoudr::connection* conn = udr_helper.conn_ptr(in->conn.str);
				if (att_resources->connections.valid(conn))
				{
					if (!in->queryNull)
					{
						U8_VARIYNG(in, query);
						stmt = new nanoudr::statement(
							*att_resources, *conn,
							NANODBC_TEXT(in->query.str),
							in->scrollableNull ? scroll_state::DEFAULT : 
								in->scrollable == FB_TRUE ? scroll_state::SCROLLABLE : scroll_state::NONSCROLLABLE,
							in->timeout);
					}
					else
						stmt = new nanoudr::statement(*att_resources, *conn);
				}
				else
					NANOUDR_THROW(POINTER_CONN_INVALID)
			}
			else
				stmt = new nanoudr::statement(*att_resources);
			udr_helper.fb_ptr(out->stmt.str, (int64_t)stmt);
			out->stmtNull = FB_FALSE;
		}	
		catch (std::runtime_error const& e)
		{
			NANODBC_THROW(e.what())
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function valid (
//	 stmt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!stmt$valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$valid)

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
		NANOUDR_RESOURCES
		out->valid = udr_helper.fb_bool(
			in->stmtNull ? false :
				att_resources->statements.valid(udr_helper.stmt_ptr(in->stmt.str))
			);
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	 stmt ty$pointer not null 
// ) returns ty$pointer
// external name 'nano!stmt$release'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$release)

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
		NANOUDR_RESOURCES
		out->stmtNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
			try
			{
				delete (nanoudr::statement*)(stmt);
			}
			catch (std::runtime_error const& e)
			{
				udr_helper.fb_ptr(out->stmt.str, (int64_t)stmt);
				out->stmtNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}
		
FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connected (
//	 stmt ty$pointer not null 
//	) returns boolean
//	external name 'nano!stmt$connected'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$connected)

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
		NANOUDR_RESOURCES
		out->connectedNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->connected = udr_helper.fb_bool(stmt->connected());
				out->connectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 stmt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!stmt$connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$connection)

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
		NANOUDR_RESOURCES
		out->connNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				nanoudr::connection* conn = stmt->connection();
				udr_helper.fb_ptr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function open_ (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null
// ) returns ty$nano_blank
// external name 'nano!stmt$open'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$open)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					stmt->open(*conn);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(POINTER_CONN_INVALID)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function close_ (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt$close'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$close)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->nanoudr::statement::close();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function cancel_ (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt$cancel'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$cancel)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->cancel();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function closed (
//	 stmt ty$pointer not null 
//	) returns boolean
//	external name 'nano!stmt$closed'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$closed)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, closed)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->closedNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->closed = udr_helper.fb_bool(!((nanodbc::statement*)stmt)->open());
				out->closedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null,  
//   query varchar(8191) character set utf8 not null,
//   scrollable boolean default null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt$prepare_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$prepare_direct)

	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, scrollable, timeout
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
		(FB_BOOLEAN, scrollable)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					U8_VARIYNG(in, query);
					stmt->prepare(
						*conn, 
						NANODBC_TEXT(in->query.str),
						in->scrollableNull ? scroll_state::DEFAULT : 
							in->scrollable == FB_TRUE ? scroll_state::SCROLLABLE : scroll_state::NONSCROLLABLE,
						in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(POINTER_CONN_INVALID)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prepare_ (
//	 stmt ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
//   scrollable boolean default null,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt$prepare'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$prepare)

	unsigned in_count;

	enum in : short {
		stmt = 0, query, scrollable, timeout
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
		(FB_BOOLEAN, scrollable)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				U8_VARIYNG(in, query);
				stmt->prepare(
					NANODBC_TEXT(in->query.str), 
					in->scrollableNull ? scroll_state::DEFAULT :
						in->scrollable == FB_TRUE ? scroll_state::SCROLLABLE : scroll_state::NONSCROLLABLE,
					in->timeout);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;

			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function scrollable (
//	 stmt ty$pointer not null, 
// 	 usage_ boolean default null 
//	) returns boolean
//	external name 'nano!stmt$scrollable'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$scrollable)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_BOOLEAN, usage)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, scrollable)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->scrollableNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				if (!in->usageNull)
					stmt->scrollable(
						in->usage == FB_TRUE ? scroll_state::SCROLLABLE : scroll_state::NONSCROLLABLE
					);
				scroll_state scrollable = (scroll_state)(stmt->scrollable());
				out->scrollableNull = FB_FALSE;
				switch (stmt->scrollable())
				{
					case scroll_state::SCROLLABLE: out->scrollable = FB_TRUE;
					case scroll_state::NONSCROLLABLE: out->scrollable = FB_FALSE;
					default:
						out->scrollableNull = FB_TRUE;
				}
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function timeout (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt$timeout'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$timeout)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->timeout(in->timeout);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;

			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function execute_direct (
//	 stmt ty$pointer not null, 
//	 conn ty$pointer not null, 
//   query varchar(8191) character set utf8 not null,
//   scrollable boolean default null,
// 	 batch_operations integer not null default 1,
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt$execute_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$execute_direct)

	unsigned in_count;

	enum in : short {
		stmt = 0, conn, query, scrollable, batch_operations, timeout
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
		(FB_BOOLEAN, scrollable)
		(FB_INTEGER, batch_operations)
		(FB_INTEGER, timeout)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					U8_VARIYNG(in, query);
					nanoudr::result* rslt =
						stmt->execute_direct(
							*conn, 
							NANODBC_TEXT(in->query.str),
							in->scrollableNull ? scroll_state::DEFAULT :
								in->scrollable == FB_TRUE ? scroll_state::SCROLLABLE : scroll_state::NONSCROLLABLE,
							in->batch_operations, in->timeout);
					udr_helper.fb_ptr(out->rslt.str, (int64_t)rslt);
					out->rsltNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(POINTER_CONN_INVALID)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
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
//	external name 'nano!stmt$just_execute_direct'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$just_execute_direct)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.conn_ptr(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					U8_VARIYNG(in, query);
					stmt->just_execute_direct(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(POINTER_CONN_INVALID)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function execute_ (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1, 
// 	 timeout integer not null default 0 
//	) returns ty$pointer
//	external name 'nano!stmt$execute'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$execute)

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
		NANOUDR_RESOURCES
		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->bind_params(in->batch_operations);
				nanoudr::result* rslt = stmt->execute(in->batch_operations, in->timeout);
				udr_helper.fb_ptr(out->rslt.str, (int64_t)rslt);
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function just_execute_ (
//	 stmt ty$pointer not null, 
// 	 batch_operations integer not null default 1,
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt$just_execute'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$just_execute)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->bind_params(in->batch_operations);
				stmt->just_execute(in->batch_operations, in->timeout);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what());
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
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
//	external name 'nano!stmt$procedure_columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$procedure_columns)

	unsigned in_count;

	enum in : short {
		stmt = 0, catalog, schema, procedure, column
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
		(FB_VARCHAR(128 * 4), catalog)
		(FB_VARCHAR(128 * 4), schema)
		(FB_VARCHAR(63 * 4), procedure)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				U8_VARIYNG(in, catalog);
				U8_VARIYNG(in, schema);
				U8_VARIYNG(in, procedure);
				U8_VARIYNG(in, column);
				nanodbc::result rslt =
					stmt->procedure_columns(NANODBC_TEXT(in->catalog.str), NANODBC_TEXT(in->schema.str), 
						NANODBC_TEXT(in->procedure.str), NANODBC_TEXT(in->column.str));
				udr_helper.fb_ptr(
					out->rslt.str, (int64_t)(new nanoudr::result(*att_resources, *stmt->connection(), std::move(rslt))));
				out->rsltNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	 stmt ty$pointer not null 
//	) returns integer
//	external name 'nano!stmt$affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$affected_rows)

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
		NANOUDR_RESOURCES
		out->affectedNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->affected = stmt->affected_rows();
				out->affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function columns (
//	 stmt ty$pointer not null 
//	) returns smallint
//	external name 'nano!stmt$columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$columns)

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
		NANOUDR_RESOURCES
		out->columnsNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->columns = stmt->columns();
				out->columnsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function reset_parameters (
//	 stmt ty$pointer not null, 
// 	 timeout integer not null default 0 
//	) returns ty$nano_blank
//	external name 'nano!stmt$reset_parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$reset_parameters)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->reset_parameters();
				if (stmt->params() != nullptr) stmt->params()->clear();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameters  ( 
//	 stmt ty$pointer not null 
//	) returns smallint
//	external name 'nano!stmt$parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$parameters)

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
		NANOUDR_RESOURCES
		out->parametersNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->parameters = stmt->parameters();
				out->parametersNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameter_size  ( 
//	 stmt ty$pointer not null, 
//	 param_index smallint not null
//	) returns integer
//	external name 'nano!stmt$parameter_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$parameter_size)

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
		NANOUDR_RESOURCES
		out->sizeNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->size = stmt->parameter_size(in->param_index);
				if (out->size < 0) out->size = -1; // BLOB
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// template <class T> void bind (...
//
// create function bind_[fb_type] (
//	 stmt ty$pointer not null,
// 	 param_index smallint not null,
//   value_	native Firebird datatype,
//   param_size smallint default null
//	) returns ty$nano_blank
//	external name 'nano!stmt$bind'
//	engine udr; 

FB_UDR_BEGIN_FUNCTION(stmt$bind)

	unsigned in_count;

	enum in : short {
		stmt = 0, param_index, value, param_size
	};

	AutoArrayDelete<unsigned> in_types;
	AutoArrayDelete<unsigned> in_lengths;
	AutoArrayDelete<unsigned> in_char_sets;
	AutoArrayDelete<unsigned> in_offsets;
	AutoArrayDelete<unsigned> in_null_offsets;

	IUtil* utl = master->getUtilInterface();

	FB_UDR_CONSTRUCTOR
	{
		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		
		in_types.reset(new unsigned[in_count]);
		in_lengths.reset(new unsigned[in_count]);
		in_char_sets.reset(new unsigned[in_count]);
		in_offsets.reset(new unsigned[in_count]);
		in_null_offsets.reset(new unsigned[in_count]);

		for (unsigned i = 0; i < in_count; ++i)
		{
			in_types[i] = in_metadata->getType(status, i);
			in_lengths[i] = in_metadata->getLength(status, i);
			in_char_sets[i] = in_metadata->getCharSet(status, i);
			in_offsets[i] = in_metadata->getOffset(status, i);
			in_null_offsets[i] = in_metadata->getNullOffset(status, i);
		}
	}

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr((char*)(in + in_offsets[in::stmt]));
		if (!*(ISC_SHORT*)(in + in_null_offsets[in::stmt]) && att_resources->statements.valid(stmt))
		{
			try
			{
				params_batch* params = stmt->params();
				if (params)
				{
					short param_index = *(ISC_SHORT*)(in + in_offsets[in::param_index]);
					bool null_flag = *(ISC_SHORT*)(in + in_null_offsets[in::value]) ? true : false;
					switch (in_types[in::value])
					{
						case SQL_TEXT: // char
						case SQL_VARYING: // varchar
						{
							if (null_flag)
								params->push(param_index, (nanodbc::string)(NANODBC_TEXT("\0")), null_flag);
							else
							{
								bool u8_string = (in_char_sets[in::value] == fb_char_set::CS_UTF8);
								ISC_USHORT length =
									(in_types[in::value] == SQL_TEXT ?
										in_lengths[in::value] :	// полный размер переданного CHAR(N) с учетом пробелов 
										*(ISC_USHORT*)(in + in_offsets[in::value])); 
								ISC_USHORT param_size = 0;
								if (in_count > in::param_size && !*(ISC_SHORT*)(in + in_null_offsets[in::param_size]))
								{
									param_size = *(ISC_SHORT*)(in + in_offsets[in::param_size]);
									if (param_size < 0)
										BINDING_THROW("PARAM_SIZE, expected zero or positive value.")
								}
								param_size = 
									param_size == 0 || param_size > length ? length : param_size;
								char* param = new char[param_size + 1]; // null-term string
								if (u8_string)
									param_size =
										udr_helper.utf8_in(
											att_resources, param, param_size,
											(const char*)(in + (in_types[in::value] == SQL_TEXT ? 0 : sizeof(ISC_USHORT)) + in_offsets[in::value]),
											length);
								else
									memcpy(
										param,
										(char*)(in + (in_types[in::value] == SQL_TEXT ? 0 : sizeof(ISC_USHORT)) + in_offsets[in::value]),
										param_size
									);
								param[param_size] = '\0';
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
						case SQL_D_FLOAT: // VAX ?
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
								param = udr_helper.set_timestamp(&tm);
							}
							params->push(param_index, param, null_flag);
							break;
						}
						case SQL_BLOB: // blob
						{
							if (null_flag)
								params->push(param_index, (nanodbc::string)(NANODBC_TEXT("\0")), null_flag);
							else
								BINDING_THROW("Binding BLOB to be developed.")
							break;
						}
						case SQL_ARRAY: // array
						{
							BINDING_THROW("Binding SQL_ARRAY not supported.")
							break;
						}
						case SQL_QUAD: // blob_id 
						{
							BINDING_THROW("Binding SQL_QUAD not supported.")
							break;
						}
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
								nanodbc::time param = udr_helper.set_time(&t);
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
								nanodbc::date param = udr_helper.set_date(&d);
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
								param = udr_helper.native_bool(*(FB_BOOLEAN*)(in + in_offsets[in::value]));
							params->push(param_index, (nanodbc::string)(NANODBC_TEXT(param ? "True" : "False")), null_flag);
							break;
						}
						case SQL_NULL: // null, nothing
						{
							break;
						}
						default:
						{
							BINDING_THROW("Binding unknow Firebird SQL datatype.")
							break;
						}
					}
				}
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function bind_null (
//	 stmt ty$pointer not null,
// 	 param_index smallint not null,
// 	 batch_size integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!stmt$bind_null'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$bind_null)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->bind_null(in->param_index, in->batch_size);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
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
//	external name 'nano!stmt$describe_parameters
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$describe_parameters)

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
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.stmt_ptr(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->describe_parameters
				(std::vector<short>(in->idx), std::vector<short>(in->type), std::vector<unsigned long>(in->size),
					std::vector<short>(in->scale));
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_STMT_INVALID)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr

