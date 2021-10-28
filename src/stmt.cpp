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

params_batchs::params_batchs(const short count)
{
	batchs = new batch[count];
	count_ = count;
}

params_batchs::~params_batchs() noexcept
{
	clear();
	delete[] batchs;
}

short params_batchs::count()
{
	return count_;
}

template <class T>
long params_batchs::push(const short parameter_index, T const value, const bool null)
{
	bind_types& values = (&batchs[parameter_index])->values;
	std::vector<uint8_t>& nulls = (&batchs[parameter_index])->nulls;

	if (!std::holds_alternative<std::vector<T>>(values))
		values = std::vector<T>{};
	std::get<std::vector<T>>(values).push_back(value);
	nulls.push_back(null);

	return static_cast<long>(nulls.size() - 1);
}

template <class T>
long params_batchs::push(const short parameter_index, T && value, const bool * null)
{
	bind_types& values = (&batchs[parameter_index])->values;
	std::vector<uint8_t>& nulls = (&batchs[parameter_index])->nulls;

	if (!std::holds_alternative<std::vector<T>>(values))
		values = std::vector<T>{};
	std::get<std::vector<T>>(values).push_back(std::move(value));
	nulls.push_back(*null);

	return static_cast<long>(nulls.size() - 1);
}

bind_type params_batchs::touch(const short parameter_index)
{
	bind_types& vec = batchs[parameter_index].values;
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

template <typename T> T* params_batchs::values(const short parameter_index)
{
	bind_types& vec = batchs[parameter_index].values;
	return std::get<std::vector<T>>(vec).data();
}

template <class T> std::vector<T>* params_batchs::vvalues(const short parameter_index)
{
	bind_types& vec = batchs[parameter_index].values;
	return &(std::get<std::vector<T>>(vec));
}

template <class T> T* params_batchs::value(const short parameter_index, const long batch_index)
{
	bind_types& vec = batchs[parameter_index].values;
	T* v = std::get<std::vector<T>>(vec).data();
	return &(v[batch_index]);
}

bool params_batchs::is_null(const short parameter_index, const long batch_index)
{
	return batchs[parameter_index].nulls.at(batch_index);
}

bool* params_batchs::nulls(const short parameter_index)
{
	return reinterpret_cast<bool*>(batchs[parameter_index].nulls.data());
}

void params_batchs::clear()
{
	for (short parameter_index = 0; parameter_index < count_; ++parameter_index)
	{
		batch* b = &batchs[parameter_index];
		std::visit([](auto&& arg) { arg.clear(); }, b->values);
		b->nulls.clear();
	}
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

	scrollable_ = scroll_state::STMT_DEFAULT;
	batchs_ = nullptr;
}

statement::statement(class attachment_resources& att_resources, class nanoudr::connection& conn)
	: nanodbc::statement(conn)
{
	att_resources_ = &att_resources;
	att_resources_->statements.retain(this);
	conn_ = &conn;

	scrollable_ = scroll_state::STMT_DEFAULT;
	batchs_ = nullptr;
}

statement::statement(
	class attachment_resources& att_resources, class nanoudr::connection& conn, const nanodbc::string& query, 
	const scroll_state scrollable_usage, long timeout)
	: nanodbc::statement(conn)
{
	att_resources_ = &att_resources;
	att_resources_->statements.retain(this);
	conn_ = &conn;

	scrollable_ = scroll_state::STMT_DEFAULT;
	scrollable(scrollable_usage);
	nanodbc::statement::prepare(query, timeout);
	prepare_parameters();
}

statement::~statement() 
{
	clear_parameters();
	att_resources_->statements.release(this);
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
	clear_parameters();
	scrollable_ = scroll_state::STMT_DEFAULT;
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
	clear_parameters();
	scrollable(scrollable_usage);
	nanodbc::statement::prepare(query, timeout);
	prepare_parameters();
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

void statement::bind_parameters(long batch_operations)
{
	if (batchs_)
	{
		bool batch_operation = !(batch_operations == 1);
		for (short parameter_index = 0; parameter_index < batchs_->count(); ++parameter_index)
		{
			bool* nulls_flag;
			if (!batch_operation && batchs_->is_null(parameter_index, 0))
			{
				bind_null(parameter_index);
				continue;
			}
			else
				nulls_flag = batchs_->nulls(parameter_index);

			switch (batchs_->touch(parameter_index))
			{
				case bind_type::NANODBC_SHORT:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<short>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<short>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_USHORT:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<unsigned short>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<unsigned short>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_INT:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<int>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<int>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_UINT:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<unsigned int>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<unsigned int>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_LONG:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<long int>(parameter_index), batch_operations,	nulls_flag);
					else
						bind(parameter_index, batchs_->value<long int>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_ULONG:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<unsigned long int>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<unsigned long int>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_INT64:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<long long>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<long long>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_UINT64:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<unsigned long long>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<unsigned long long>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_FLOAT:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<float>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<float>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_DOUBLE:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<double>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<double>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_DATE:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<nanodbc::date>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<nanodbc::date>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_TIME:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<nanodbc::time>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<nanodbc::time>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_TIMESTAMP:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<nanodbc::timestamp>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<nanodbc::timestamp>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_WSTRING_VALUE_TYPE:
				{
					if (batch_operation)
						bind(parameter_index, batchs_->values<nanodbc::wide_string::value_type>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<nanodbc::wide_string::value_type>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_WIDE_STRING:
				{
					if (batch_operation)
						bind_strings(parameter_index, *(batchs_->vvalues<nanodbc::wide_string>(parameter_index)), nulls_flag);
					else
						bind(parameter_index, (nanodbc::wide_char_t*)(batchs_->value<nanodbc::string>(parameter_index, 0)->c_str()));
					break;
				}
				case bind_type::NANODBC_STRING_VALUE_TYPE:
				{
					if (batch_operation) 
						bind(parameter_index, batchs_->values<nanodbc::string::value_type>(parameter_index), batch_operations, nulls_flag);
					else
						bind(parameter_index, batchs_->value<nanodbc::string::value_type>(parameter_index, 0));
					break;
				}
				case bind_type::NANODBC_STRING:
				{
					if (batch_operation)
						bind_strings(parameter_index, *(batchs_->vvalues<nanodbc::string>(parameter_index)), nulls_flag);
					else
						bind(parameter_index, (char*)(batchs_->value<nanodbc::string>(parameter_index, 0)->c_str()));
					break;
				}
				case bind_type::NANODBC_BINARY:
				{
					if (batch_operation)
						bind(parameter_index, *(batchs_->vvalues<std::vector<uint8_t>>(parameter_index)), nulls_flag);
					else
						bind(parameter_index, *(batchs_->vvalues<std::vector<uint8_t>>(parameter_index))); // multiple... todo
					break;
				}
				case bind_type::UNKNOWN: default:
				{
					throw std::runtime_error("Not supported NANODBC data type.");
					break;
				}
			}
		}
	}
}

void statement::prepare_parameters()
{
	batchs_ = nullptr;
	short parameters_count = parameters();
	if (parameters_count == 0 && 
		desc_params_.idx.size() != 0 &&
		desc_params_.idx.size() == desc_params_.type.size() && desc_params_.idx.size() == desc_params_.size.size() &&
		desc_params_.idx.size() == desc_params_.scale.size())
	{
		describe_parameters(desc_params_.idx, desc_params_.type, desc_params_.size, desc_params_.scale);
		parameters_count = parameters();
	}
	if (parameters_count != 0)
		batchs_ = new nanoudr::params_batchs(parameters_count);
}

void statement::clear_parameters()
{
	desc_params_.idx.clear();
	desc_params_.type.clear();
	desc_params_.size.clear();
	desc_params_.scale.clear();
	if (batchs_ != nullptr) delete batchs_;
	batchs_ = nullptr;
}

void statement::describe_parameter(
	const short idx, const short type, const unsigned long size, const short scale)
{
	desc_params_.idx.push_back(idx);
	desc_params_.type.push_back(type);
	desc_params_.size.push_back(size);
	desc_params_.scale.push_back(scale);
};

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
		ATTACHMENT_RESOURCES
		out->stmtNull = FB_TRUE;
		try
		{
			nanoudr::statement* stmt;
			if (!in->connNull)
			{
				nanoudr::connection* conn = udr_helper.native_ptr<connection>(in->conn.str);
				if (att_resources->connections.valid(conn))
				{
					if (!in->queryNull)
					{
						U8_VARIYNG(in, query)
						stmt = new nanoudr::statement(
							*att_resources, *conn,
							NANODBC_TEXT(in->query.str),
							in->scrollableNull ? scroll_state::STMT_DEFAULT : 
								in->scrollable == FB_TRUE ? scroll_state::STMT_SCROLLABLE : scroll_state::STMT_NONSCROLLABLE,
							in->timeout);
					}
					else
						stmt = new nanoudr::statement(*att_resources, *conn);
				}
				else
					NANOUDR_THROW(INVALID_CONNECTION)
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
		ATTACHMENT_RESOURCES
		out->valid = udr_helper.fb_bool(
			in->stmtNull ? false :
				att_resources->statements.valid(udr_helper.native_ptr<statement>(in->stmt.str))
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
		ATTACHMENT_RESOURCES
		out->stmtNull = FB_TRUE;
		if (!in->stmtNull)
		{
			nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->connectedNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->connNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.native_ptr<connection>(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					stmt->open(*conn);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONNECTION)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->closedNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.native_ptr<connection>(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					U8_VARIYNG(in, query)
					stmt->prepare(
						*conn, 
						NANODBC_TEXT(in->query.str),
						in->scrollableNull ? scroll_state::STMT_DEFAULT : 
							in->scrollable == FB_TRUE ? scroll_state::STMT_SCROLLABLE : scroll_state::STMT_NONSCROLLABLE,
						in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONNECTION)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				U8_VARIYNG(in, query)
				stmt->prepare(
					NANODBC_TEXT(in->query.str), 
					in->scrollableNull ? scroll_state::STMT_DEFAULT :
						in->scrollable == FB_TRUE ? scroll_state::STMT_SCROLLABLE : scroll_state::STMT_NONSCROLLABLE,
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->scrollableNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				if (!in->usageNull)
					stmt->scrollable(
						in->usage == FB_TRUE ? scroll_state::STMT_SCROLLABLE : scroll_state::STMT_NONSCROLLABLE
					);
				out->scrollableNull = FB_FALSE;
				switch (stmt->scrollable())
				{
					case scroll_state::STMT_SCROLLABLE: out->scrollable = FB_TRUE;
					case scroll_state::STMT_NONSCROLLABLE: out->scrollable = FB_FALSE;
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.native_ptr<connection>(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					U8_VARIYNG(in, query)
					nanoudr::result* rslt =
						stmt->execute_direct(
							*conn, 
							NANODBC_TEXT(in->query.str),
							in->scrollableNull ? scroll_state::STMT_DEFAULT :
								in->scrollable == FB_TRUE ? scroll_state::STMT_SCROLLABLE : scroll_state::STMT_NONSCROLLABLE,
							in->batch_operations, in->timeout);
					udr_helper.fb_ptr(out->rslt.str, (int64_t)rslt);
					out->rsltNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONNECTION)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			nanoudr::connection* 
				conn = in->connNull ? nullptr : udr_helper.native_ptr<connection>(in->conn.str);
			try
			{
				if (conn != nullptr && att_resources->connections.valid(conn))
				{
					U8_VARIYNG(in, query)
					stmt->just_execute_direct(*conn, NANODBC_TEXT(in->query.str), in->batch_operations, in->timeout);
					out->blank = BLANK;
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_CONNECTION)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->bind_parameters(in->batch_operations);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->bind_parameters(in->batch_operations);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->rsltNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				U8_VARIYNG(in, catalog)
				U8_VARIYNG(in, schema)
				U8_VARIYNG(in, procedure)
				U8_VARIYNG(in, column)
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->affectedNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->columnsNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
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
		ATTACHMENT_RESOURCES
		out->parametersNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
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
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function parameter_size  ( 
//	 stmt ty$pointer not null, 
//	 parameter_index smallint not null
//	) returns integer
//	external name 'nano!stmt$parameter_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$parameter_size)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, parameter_index)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		ATTACHMENT_RESOURCES
		out->sizeNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				out->size = stmt->parameter_size(in->parameter_index);
				if (out->size < 0) out->size = -1; // BLOB
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// template <class T> void bind (...
//
// create function bind_[fb_type] (
//	 stmt ty$pointer not null,
// 	 parameter_index smallint not null,
//   value_	native Firebird datatype,
//   param_size smallint default null
//	) returns ty$nano_blank
//	external name 'nano!stmt$bind'
//	engine udr; 

FB_UDR_BEGIN_FUNCTION(stmt$bind)

	unsigned in_count;

	enum in : short {
		stmt = 0, parameter_index, value, param_size
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>((char*)(in + in_offsets[in::stmt]));
		if (!*(ISC_SHORT*)(in + in_null_offsets[in::stmt]) && att_resources->statements.valid(stmt))
		{
			try
			{
				params_batchs* batchs = stmt->batchs();
				if (batchs)
				{
					short parameter_index = *(ISC_SHORT*)(in + in_offsets[in::parameter_index]);
					bool null_flag = *(ISC_SHORT*)(in + in_null_offsets[in::value]) ? true : false;
					switch (in_types[in::value])
					{
						case SQL_TEXT: // char
						case SQL_VARYING: // varchar
						{
							if (null_flag)
								batchs->push(parameter_index, std::move((nanodbc::string)(NANODBC_TEXT("\0"))), &null_flag);
							else
							{
								bool u8_string = (in_char_sets[in::value] == fb_char_set::CS_UTF8);
								ISC_USHORT length = 
									static_cast<ISC_USHORT>(
										(in_types[in::value] == SQL_TEXT ?
											in_lengths[in::value] :	// полный размер переданного CHAR(N) с учетом пробелов 
											*(ISC_USHORT*)(in + in_offsets[in::value])));
								ISC_USHORT param_size = 0;
								if (in_count > in::param_size && !*(ISC_SHORT*)(in + in_null_offsets[in::param_size]))
								{
									param_size = *(ISC_SHORT*)(in + in_offsets[in::param_size]);
									if (param_size < 0) 
										throw std::runtime_error("PARAM_SIZE, expected zero or positive value.");
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
								batchs->push(parameter_index, std::move((nanodbc::string)(NANODBC_TEXT(param))), &null_flag);
								delete[] param;
							}
							break;
						}
						case SQL_SHORT: // smallint
							batchs->push(parameter_index, *(ISC_SHORT*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_LONG: // integer 
							batchs->push(parameter_index, *(ISC_LONG*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_FLOAT: // float
							batchs->push(parameter_index, *(float*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_DOUBLE: // double precision 
						case SQL_D_FLOAT: // VAX ?
							batchs->push(parameter_index, *(double*)(in + in_offsets[in::value]), null_flag);
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
								struct nanoudr::timestamp ts;
								fb.date.decode(utl, &ts.d.year, &ts.d.month, &ts.d.day);
								fb.time.decode(utl, &ts.t.hour, &ts.t.min, &ts.t.sec, &ts.t.fract);
								param = udr_helper.set_timestamp(&ts);
							}
							batchs->push(parameter_index, (nanodbc::timestamp)(param), null_flag);
							break;
						}
						case SQL_BLOB: // blob
						{
							if (null_flag)
								batchs->push(parameter_index, std::move((nanodbc::string)(NANODBC_TEXT("\0"))), &null_flag);
							else
							{
								std::vector<std::uint8_t> param;
								udr_helper.read_blob(att_resources, (ISC_QUAD*)(in + in_offsets[in::value]), &param);
								batchs->push(parameter_index, std::move(param), &null_flag);
							}
							break;
						}
						case SQL_ARRAY: // array
						{
							throw std::runtime_error("Binding SQL_ARRAY not supported.");
							break;
						}
						case SQL_QUAD: // blob_id 
						{
							throw std::runtime_error("Binding SQL_QUAD not supported.");
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
								param = udr_helper.set_time(&t);
							}
							batchs->push(parameter_index, (nanodbc::time)(param), null_flag);
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
								param = udr_helper.set_date(&d);
							}
							batchs->push(parameter_index, (nanodbc::date)(param), null_flag);
							break;
						}
						case SQL_INT64: // bigint
							batchs->push(parameter_index, *(ISC_INT64*)(in + in_offsets[in::value]), null_flag);
							break;
						case SQL_BOOLEAN: // boolean
						{
							FB_BOOLEAN param;
							if (null_flag) 
								param = FB_FALSE; else
								param = *(FB_BOOLEAN*)(in + in_offsets[in::value]);
							batchs->push(parameter_index, (unsigned short)(param ? true : false), null_flag);
							break;
						}
						case SQL_NULL: // null
						{
							throw std::runtime_error("Binding SQL_NULL not supported.");
							break;
						}
						default:
						{
							throw std::runtime_error("Binding unknow Firebird SQL datatype.");
							break;
						}
					}
				}
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANOUDR_THROW(BINDING_ERROR, e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function bind_null (
//	 stmt ty$pointer not null,
// 	 parameter_index smallint not null,
// 	 batch_size integer not null default 1 
//	) returns ty$nano_blank
//	external name 'nano!stmt$bind_null'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$bind_null)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, stmt)
		(FB_SMALLINT, parameter_index)
		(FB_INTEGER, batch_size)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			if (in->batch_size == 1)
				try
				{
					stmt->batchs()->push(in->parameter_index, (unsigned short)(0), true);
				}
				catch (std::runtime_error const& e)
				{
					NANOUDR_THROW(BINDING_ERROR, e.what())
				}
			else
				try
				{ 
					stmt->bind_null(in->parameter_index, in->batch_size); // call nulls all batch
				}
				catch (std::runtime_error const& e)
				{
					NANODBC_THROW(e.what())
				}
			out->blank = BLANK;
			out->blankNull = FB_FALSE;
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function clear_bindings (
//	 stmt ty$pointer not null
//	) returns ty$nano_blank
//	external name 'nano!stmt$clear_bindings'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$clear_bindings)

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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->batchs()->clear();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANOUDR_THROW(BINDING_ERROR, e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION


//-----------------------------------------------------------------------------
// create function describe_parameter (
//	 stmt ty$pointer not null,
// 	 idx smallint not null,
// 	 type_ smallint not null,
// 	 size_ integer not null,
// 	 scale_ smallint not null default 0
//	) returns ty$nano_blank
//	external name 'nano!stmt$describe_parameter
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$describe_parameter)

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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->describe_parameter(in->idx, in->type, in->size, in->scale);
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANOUDR_THROW(BINDING_ERROR, e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function describe_parameters (
//	 stmt ty$pointer not null 
//	) returns ty$nano_blank
//	external name 'nano!stmt$describe_parameters'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(stmt$describe_parameters)

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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				statement::descr_parameters* desc = stmt->describe();
				stmt->describe_parameters(desc->idx, desc->type, desc->size, desc->scale);
				stmt->prepare_parameters();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function reset_parameters (
//	 stmt ty$pointer not null 
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
		ATTACHMENT_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::statement* stmt = udr_helper.native_ptr<statement>(in->stmt.str);
		if (!in->stmtNull && att_resources->statements.valid(stmt))
		{
			try
			{
				stmt->reset_parameters();
				stmt->clear_parameters();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_STATEMENT)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr

