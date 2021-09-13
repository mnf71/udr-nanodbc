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
// Params batch (values pointers for binding) see NANODBC_INSTANTIATE_BINDS
//

enum nanodbc_type : short {
	NANODBC_SHORT = 0, NANODBC_USHORT, NANODBC_INT, NANODBC_UINT,
	NANODBC_LONG, NANODBC_ULONG, NANODBC_INT64, NANODBC_UINT64,	NANODBC_FLOAT, NANODBC_DOUBLE, 
	NANODBC_DATE, NANODBC_TIME, NANODBC_TIMESTAMP,
	NANODBC_STRING, NANODBC_WIDE_STRING,
	NANODBC_UNKNOWN
};

typedef std::variant <
	short, unsigned short, int, unsigned int, 
	long int, unsigned long int, long long, unsigned long long, float, double, 
	nanodbc::date, nanodbc::time, nanodbc::timestamp,
	nanodbc::string, nanodbc::wide_string
> nanodbc_types;

class params_batch
{
public:
	params_batch(short count);
	~params_batch() noexcept;

	template <class T>
	long push(short param_index, T const value, const bool null = false);
	
	void clear();

	nanodbc_type touch(short param_index);

	template <typename T> T* batch(short param_index);
	template <class T> std::vector<T>* batch_vec(short param_index);
	bool* nulls(short param_index);

	template <typename T> T* value(short param_index, long batch_index);
	bool is_null(short param_index, long batch_index);

	short count();

private:
	struct parameter
	{
		std::vector<nanodbc_types> values;
		std::vector<uint8_t> nulls; 
	};

	parameter* params_;
	short count_;
};

//-----------------------------------------------------------------------------
// UDR Statement class implementation
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
	
	void bind_params(long batch_operations = 1);

	params_batch* params();

protected:
	void init_params();
	void release_params();

private:
	nanoudr::connection* conn_;
	params_batch* params_;
};

} // namespace nanoudr

#endif	/* STMT_H */