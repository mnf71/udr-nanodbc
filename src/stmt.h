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

//-----------------------------------------------------------------------------
// Params batch (values pointers for binding) see NANODBC_INSTANTIATE_BINDS
//

enum bind_type : short {
	NANODBC_SHORT = 0, NANODBC_USHORT, NANODBC_INT, NANODBC_UINT,
	NANODBC_LONG, NANODBC_ULONG, NANODBC_INT64, NANODBC_UINT64, 
	NANODBC_FLOAT, NANODBC_DOUBLE,
	NANODBC_DATE, NANODBC_TIME, NANODBC_TIMESTAMP,
	NANODBC_WSTRING_VALUE_TYPE, NANODBC_WIDE_STRING,
	NANODBC_STRING_VALUE_TYPE, NANODBC_STRING,
	NANODBC_BINARY,
	UNKNOWN
};

using bind_types = std::variant <
	std::vector<short>, std::vector<unsigned short>, std::vector<int>, std::vector<unsigned int>,
	std::vector<long int>, std::vector<unsigned long int>, std::vector<long long>, std::vector<unsigned long long>, 
	std::vector<float>, std::vector<double>,
	std::vector<nanodbc::date>, std::vector<nanodbc::time>, std::vector<nanodbc::timestamp>,
	std::vector<nanodbc::wide_string::value_type>, std::vector<nanodbc::wide_string>,
	std::vector<nanodbc::string::value_type>, std::vector<nanodbc::string>,
	std::vector<std::vector<uint8_t>>
>;

class params_batch
{
public:
	params_batch(short count);
	~params_batch() noexcept;

	template <class T>
	long push(short param_index, T const value, const bool null = false);

	bind_type touch(short param_index);

	template <class T> T* values(const short param_index);
	template <class T> std::vector<T>* vector(const short param_index);
	template <class T> T* value(const short param_index, const long batch_index);

	bool* nulls(const short param_index);

	bool is_null(const short param_index, const long batch_index);

	short count();

	void clear();

private:

	struct param
	{
		bind_types batch;
		std::vector<uint8_t> nulls;
	};
	
	param* params;
	short count_;
};

//-----------------------------------------------------------------------------
// UDR Statement class implementation
//

/* SQL_ATTR_CURSOR_SCROLLABLE values */
#if (ODBCVER >= 0x0300)
#define SQL_NONSCROLLABLE			0
#define SQL_SCROLLABLE				1
#endif  /* ODBCVER >= 0x0300 */ 

enum scroll_state : short {
	DEFAULT = 0 /* SQL_NULL */, SCROLLABLE /* TRUE */, NONSCROLLABLE /* FALSE*/
};

#ifndef RSRS_H
	class attachment_resources;
#endif

#ifndef CONN_H
	class connection;
#endif

#ifndef RSLT_H
	class result;
#endif

class statement : public nanodbc::statement
{
public:
	statement(class attachment_resources& att_resources);
	explicit statement(class attachment_resources& att_resources, class nanoudr::connection& conn);
	statement(class attachment_resources& att_resources, class nanoudr::connection& conn, const nanodbc::string& query, 
		const scroll_state scrollable_usage, long timeout = 0);
	~statement() noexcept;

	void scrollable(const scroll_state scrollable_usage);

	void open(class nanoudr::connection& conn);
	nanoudr::connection* connection();
	void close();

	void prepare(class nanoudr::connection& conn, const nanodbc::string& query, const scroll_state scrollable_usage, long timeout = 0);
	void prepare(const nanodbc::string& query, const scroll_state scrollable_usage, long timeout = 0);

	nanoudr::result* execute_direct(
		class nanoudr::connection& conn, const nanodbc::string& query, const scroll_state scrollable_usage, long batch_operations = 1, long timeout = 0);
	void just_execute_direct(
		class nanoudr::connection& conn, const nanodbc::string& query, long batch_operations = 1, long timeout = 0);

	nanoudr::result* execute(long batch_operations = 1, long timeout = 0);
	void just_execute(long batch_operations = 1, long timeout = 0);

	void bind_params(long batch_operations = 1);

	params_batch* params();

	void prepare_params();
	void release_params();

	attachment_resources* attachment() { return att_resources_; };
	scroll_state scrollable() { return scrollable_; };

private:
	attachment_resources* att_resources_;
	nanoudr::connection* conn_;
	scroll_state scrollable_;
	params_batch* params_;
};

} // namespace nanoudr

#endif	/* STMT_H */