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

#include <variant>

namespace nanoudr
{

class connection;

//-----------------------------------------------------------------------------
//

class params_batch
{
public:
	params_batch(short size);
	~params_batch() noexcept;

	template <class T>
	long push(short param_index, T const value);

	void push_null(short param_index);
	
	void clear();

	template <class T>
	T* value(short param_index, long batch_index);

	template <class T>
	std::vector<T> values(short param_index);

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
	statement();
	explicit statement(class connection& conn);
	statement(class nanoudr::connection& conn, const nanodbc::string& query, long timeout = 0);
	~statement() noexcept;

	void open(class connection& conn);
	nanoudr::connection* connection();

	void prepare(class nanoudr::connection& conn, const nanodbc::string& query, long timeout = 0);
	void prepare(const nanodbc::string& query, long timeout = 0);

	class nanodbc::result execute_direct(class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations = 1, long timeout = 0);
	void just_execute_direct(class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations = 1, long timeout = 0);

	params_batch* params_batch;

protected:
	void initialize_params_batch();
	void dispose_params_batch();

private:
	nanoudr::connection* conn_;
};

} // namespace nanoudr

#endif	/* STMT_H */