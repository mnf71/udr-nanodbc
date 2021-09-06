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

#include <nanodbc.h> 

#include <variant>

namespace nano
{

class params_array
{
public:
	params_array(short size)
	{
		data_ = new param_values[size];
		size_ = size;
	};

	~params_array() noexcept
	{
		clear();
		delete[] data_;
	};

	template <class T>
	long push(short param_index, T const value)
	{
		data_[param_index].values.push_back(value);
		return (long)(data_[param_index].values.size() - 1); // batch_index 
	};

	void push_null(short param_index)
	{
		data_[param_index].values.push_back('\0');
	};

	void clear()
	{
		for (std::size_t param_index = 0; param_index < size_; ++param_index)
			data_[param_index].values.clear();
	}

	template <class T>
	T* value(short param_index, long batch_index)
	{
		T* p = (T*)(data_[param_index].values.data());
		return &(p[batch_index]);
	};

	template <class T>
	std::vector<T> values(short param_index)
	{
		return data_[param_index].values;
	};

private:
	struct param_values
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
				wide_string
			>
		> values;
	};

	param_values* data_;
	short size_;
};

//-----------------------------------------------------------------------------
//

class statement
{
public:
	statement()
	{
		impl_ = new nanodbc::statement();
		batch_array = nullptr;
	};

	explicit statement(class connection& conn)
	{
		impl_ = new nanodbc::statement(conn);
		batch_array = nullptr;
	};

	statement(class connection& conn, const string& query, long timeout = 0)
	{
		impl_ = new nanodbc::statement(conn, query, timeout);
		short parameters_count = impl_->parameters();
		batch_array = nullptr;
		if (parameters_count != 0)
			batch_array = new nano::params_array(parameters_count);
	};

	~statement() noexcept
	{
		if (batch_array != nullptr)
		{
			delete batch_array;
		}
		delete impl_;
	};

	nanodbc::statement* impl()
	{
		return impl_;
	};

	params_array* batch_array;

private:
	nanodbc::statement* impl_;
};

} // namespace nano