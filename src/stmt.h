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

#ifndef STMT_H
#define STMT_H

 //-----------------------------------------------------------------------------
 // package nano$stmt
 //

#include <nanodbc.h> 

#include <variant>

namespace nanoudr
{

//-----------------------------------------------------------------------------
//

class params_batch
{
public:
	params_batch(short size)
	{
		params = new param[size];
		size_ = size;
	};

	~params_batch() noexcept
	{
		clear();
		delete[] params;
	};

	template <class T>
	long push(short param_index, T const value)
	{
		params[param_index].values.push_back(value);
		return (long)(params[param_index].values.size() - 1); // batch_index 
	};

	void push_null(short param_index)
	{
		params[param_index].values.push_back('\0');
	};

	void clear()
	{
		for (std::size_t param_index = 0; param_index < size_; ++param_index)
			params[param_index].values.clear();
	};

	template <class T>
	T* value(short param_index, long batch_index)
	{
		T* p = (T*)(params[param_index].values.data());
		return &(p[batch_index]);
	};

	template <class T>
	std::vector<T> values(short param_index)
	{
		return params[param_index].values;
	};

private:
	struct param
	{
		std::vector<
			std::variant<
				short,
				unsigned short,
				int,
				unsigned int,
				long int,
				unsigned long int,
				long long,
				unsigned long long,
				float,
				double,
				nanodbc::date,
				nanodbc::time,
				nanodbc::timestamp,
				nanodbc::string,
				nanodbc::wide_string
			>
		> values;
	};

	param* params;
	short size_;
};

//-----------------------------------------------------------------------------
//

class statement : public nanodbc::statement
{
public:
	statement() : nanodbc::statement()
	{
		params_batch = nullptr;
	};

	explicit statement(class nanoudr::connection& conn) 
		: nanodbc::statement(conn)
	{
		params_batch = nullptr;
		conn_ = &conn;
	};

	statement(class nanoudr::connection& conn, const nanodbc::string& query, long timeout = 0) 
		: nanodbc::statement(conn, query, timeout) 
	{
		short parameters_count = parameters();
		params_batch = nullptr;
		if (parameters_count != 0)
			params_batch = new nanoudr::params_batch(parameters_count);
		conn_ = &conn;
	};

	~statement() noexcept
	{
		if (params_batch != nullptr) delete params_batch;
		nanodbc::statement::~statement();
	};

	void open(class connection& conn)
	{
		nanodbc::statement::open(conn);
		conn_ = &conn;
	};

	nanoudr::connection* connection()
	{
		return conn_;
	};

	void prepare(class nanoudr::connection& conn, const nanodbc::string& query, long timeout = 0)
	{
		nanodbc::statement::prepare(conn, query, timeout);
		conn_ = &conn;
	};

	class nanodbc::result execute_direct(class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations = 1, long timeout = 0)
	{
		nanodbc::result rslt = 
			nanodbc::statement::execute_direct(conn, query, batch_operations, timeout = 0);
		conn_ = &conn;
		return rslt;
	};

	void just_execute_direct(class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations = 1, long timeout = 0)
	{
		nanodbc::statement::just_execute_direct(conn, query, batch_operations, timeout);
		conn_ = &conn;
	};

	params_batch* params_batch;

private:
	nanoudr::connection* conn_;
};

} // namespace nanoudr

#endif	/* STMT_H */