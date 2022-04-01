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
// package nano$ctlg
//

namespace nanoudr
{

//-----------------------------------------------------------------------------
// UDR Catalog class implementation
//

catalog::catalog(class attachment_resources& att_resources, class nanoudr::connection& conn)
	: nanodbc::catalog(conn)
{
	att_resources_ = &att_resources;
	att_resources_->catalogs.retain(this);
	conn_ = &conn;
}

catalog::~catalog()
{
	att_resources_->catalogs.release(this);
}

nanoudr::connection* catalog::connection()
{
	return conn_;
}

//-----------------------------------------------------------------------------
// create function catalog_ (
//	conn ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!ctlg$catalog'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(ctlg$catalog)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, ctlg)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->ctlgNull = FB_TRUE;
		nanoudr::connection* conn = helper.native_ptr<connection>(in->conn.str);
		if (!in->connNull && att_resources->connections.valid(conn))
		{
			try
			{
				nanoudr::catalog* ctlg = new nanoudr::catalog(*att_resources, *conn);
				helper.fb_ptr(out->ctlg.str, ctlg);
				out->ctlgNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CONNECTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function valid (
//	ctlg ty$pointer not null, 
//	) returns boolean
//	external name 'nano!ctlg$valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(ctlg$valid)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, valid)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->valid = helper.fb_bool(
			in->ctlgNull ? false :
				att_resources->catalogs.valid(helper.native_ptr<catalog>(in->ctlg.str))
			);
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	ctlg ty$pointer not null, 
//	) returns ty$pointer
//	external name 'nano!ctlg$release'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(ctlg$release)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, ctlg)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->ctlgNull = FB_TRUE;
		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<catalog>(in->ctlg.str);
			try
			{
				if (att_resources->catalogs.valid(ctlg)) delete (nanoudr::catalog*)(ctlg);
			}
			catch (std::runtime_error const& e)
			{
				helper.fb_ptr(out->ctlg.str, ctlg);
				out->ctlgNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_TRANSACTION)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------

#define GET_STRING(parameter)	\
	nanodbc::string parameter = procedure->result_set->parameter();	\
	if (parameter.empty()) out->parameter##Null = FB_TRUE;	\
	else	\
	{	\
		FB_VARIYNG(out->parameter, parameter)	\
		U8_VARIYNG(out, parameter)	\
		out->parameter##Null = FB_FALSE;	\
	}

#define GET_NUMBER(parameter)	\
	out->parameter = procedure->result_set->parameter();	\
	out->parameter##Null = FB_FALSE;

//-----------------------------------------------------------------------------
// create procedure find_tables (
//	ctlg ty$pointer not null,
//	table_ varchar(128) character set none [utf8] default null,
//	type_ varchar(128) character set none [utf8] default null,
//	schema_ varchar(128) character set none [utf8] default null,
//	catalog_ varchar(128) character set none [utf8] default null
//	) returns (
//	table_catalog varchar(128) character set none [utf8],
//	table_schema varchar(128) character set none [utf8],
//	table_name varchar(128) character set none [utf8],
//	table_type varchar(128) character set none [utf8],
//	table_remarks varchar(2048) character set none [utf8]
//	)
//	external name 'nano!ctlg$find_tables'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$find_tables)

	DECLARE_RESOURCE
	
	enum in : short {
		ctlg = 0, table, type, schema, catalog
	};

	enum out : short {
		table_catalog = 0, table_schema, table_name, table_type, table_remarks
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
		(FB_VARCHAR(128 * 4), table)
		(FB_VARCHAR(128 * 4), type)
		(FB_VARCHAR(128 * 4), schema)
		(FB_VARCHAR(128 * 4), catalog)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), table_catalog)
		(FB_VARCHAR(128 * 4), table_schema)
		(FB_VARCHAR(128 * 4), table_name)
		(FB_VARCHAR(128 * 4), table_type)
		(FB_VARCHAR(2048 * 4), table_remarks)
	);

	catalog::tables* result_set;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned in_count;

		AutoArrayDelete<unsigned> in_char_sets;

		AutoRelease<IMessageMetadata> in_metadata(procedure->metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				if (!in->tableNull) { U8_VARIYNG(in, table) }
				if (!in->typeNull) { U8_VARIYNG(in, type) }
				if (!in->schemaNull) { U8_VARIYNG(in, schema) }
				if (!in->catalogNull) U8_VARIYNG(in, catalog)
				procedure->result_set = new catalog::tables(
					std::move(
						ctlg->find_tables(
							in->tableNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->table.str),
							in->typeNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->type.str),
							in->schemaNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->schema.str),
							in->catalogNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->catalog.str)
						)
					)
				);
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set->next())
		{
			try
			{
				GET_STRING(table_catalog)
				GET_STRING(table_schema)
				GET_STRING(table_name)
				GET_STRING(table_type)
				GET_STRING(table_remarks)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
			suspend = FB_TRUE;
		}
		else
			delete procedure->result_set;

		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure find_table_privileges (
//	ctlg ty$pointer not null,
//	catalog_ varchar(128) character set none [utf8] default null,
//	table_ varchar(128) character set none [utf8] default null,
//	schema_ varchar(128) character set none[utf8] default null
//	) returns (
//	table_catalog varchar(128) character set none [utf8],
//	table_schema varchar(128) character set none [utf8],
//	table_name varchar(128) character set none [utf8],
//	grantor varchar(128) character set none [utf8],
//	grantee varchar(128) character set none [utf8],
//	privilege varchar(128) character set none [utf8],
//	is_grantable varchar(4) character set none [utf8]
//	)
//	external name 'nano!ctlg$find_table_privileges'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$find_table_privileges)

	DECLARE_RESOURCE
	
	enum in : short {
		ctlg = 0, catalog, table, schema
	};

	enum out : short {
		table_catalog = 0, table_schema, table_name, grantor, grantee, privilege, is_grantable
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
		(FB_VARCHAR(128 * 4), catalog)
		(FB_VARCHAR(128 * 4), table)
		(FB_VARCHAR(128 * 4), schema)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), table_catalog)
		(FB_VARCHAR(128 * 4), table_schema)
		(FB_VARCHAR(128 * 4), table_name)
		(FB_VARCHAR(128 * 4), grantor)
		(FB_VARCHAR(128 * 4), grantee)
		(FB_VARCHAR(128 * 4), privilege)
		(FB_VARCHAR(4 * 4), is_grantable)
	);

	catalog::table_privileges* result_set;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned in_count;

		AutoArrayDelete<unsigned> in_char_sets;

		AutoRelease<IMessageMetadata> in_metadata(procedure->metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				if (!in->catalogNull) U8_VARIYNG(in, catalog)
				if (!in->tableNull) U8_VARIYNG(in, table)
				if (!in->schemaNull) U8_VARIYNG(in, schema)
				procedure->result_set = new catalog::table_privileges(
					std::move(
						ctlg->find_table_privileges(
							in->tableNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->table.str),
							in->schemaNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->schema.str),
							in->catalogNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->catalog.str)
						)
					)
				);
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set->next())
		{
			try
			{
				GET_STRING(table_catalog)
				GET_STRING(table_schema)
				GET_STRING(table_name)
				GET_STRING(grantor)
				GET_STRING(grantee)
				GET_STRING(privilege)
				GET_STRING(is_grantable)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
			suspend = FB_TRUE;
		}
		else
			delete procedure->result_set;

		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure find_columns (
//	ctlg ty$pointer not null,
//	column_ varchar(128) character set none [utf8] default null,
//	table_ varchar(128) character set none [utf8] default null,
//	schema_ varchar(128) character set none [utf8] default null,
//	catalog_ varchar(128) character set none [utf8] default null
//	) returns (
//	table_catalog varchar(128) character set none [utf8],
//	table_schema varchar(128) character set none [utf8],
//	table_name varchar(128) character set none [utf8],
//	column_name varchar(128) character set none [utf8],
//	data_type smallint,
//	type_name varchar(128) character set none [utf8],
//	column_size integer,
//	buffer_length integer,
//	decimal_digits smallint,
//	numeric_precision_radix smallint,
//	nullable smallint,
//	remarks varchar(2048) character set none [utf8],
//	column_default varchar(256) character set none [utf8],
//	sql_data_type smallint,
//	sql_datetime_subtype smallint,
//	char_octet_length integer,
//	ordinal_position integer,
//	is_nullable varchar(4) character set none [utf8]
//	)
//	external name 'nano!ctlg$find_columns'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$find_columns)

	DECLARE_RESOURCE
	
	enum in : short {
		ctlg = 0, column, table, schema, catalog
	};

	enum out : short {
		table_catalog = 0, table_schema, table_name, column_name, data_type, type_name, column_size, buffer_length,
		decimal_digits, numeric_precision_radix, nullable, remarks, column_default, sql_data_type, sql_datetime_subtype,
		char_octet_length, ordinal_position, is_nullable
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
		(FB_VARCHAR(128 * 4), column)
		(FB_VARCHAR(128 * 4), table)
		(FB_VARCHAR(128 * 4), schema)
		(FB_VARCHAR(128 * 4), catalog)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), table_catalog)
		(FB_VARCHAR(128 * 4), table_schema)
		(FB_VARCHAR(128 * 4), table_name)
		(FB_VARCHAR(128 * 4), column_name)
		(FB_SMALLINT, data_type)
		(FB_VARCHAR(128 * 4), type_name)
		(FB_INTEGER, column_size)
		(FB_INTEGER, buffer_length)
		(FB_SMALLINT, decimal_digits)
		(FB_SMALLINT, numeric_precision_radix)
		(FB_SMALLINT, nullable)
		(FB_VARCHAR(2048 * 4), remarks)
		(FB_VARCHAR(256 * 4), column_default)
		(FB_SMALLINT, sql_data_type)
		(FB_SMALLINT, sql_datetime_subtype)
		(FB_INTEGER, char_octet_length)
		(FB_INTEGER, ordinal_position)
		(FB_VARCHAR(4 * 4), is_nullable)
	);

	catalog::columns* result_set;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned in_count;

		AutoArrayDelete<unsigned> in_char_sets;

		AutoRelease<IMessageMetadata> in_metadata(procedure->metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				if (!in->columnNull) U8_VARIYNG(in, column)
				if (!in->tableNull) U8_VARIYNG(in, table)
				if (!in->schemaNull) U8_VARIYNG(in, schema)
				if (!in->catalogNull) U8_VARIYNG(in, catalog)
				procedure->result_set = new catalog::columns(
					std::move(
						ctlg->find_columns(
							in->columnNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->column.str),
							in->tableNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->table.str),
							in->schemaNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->schema.str),
							in->catalogNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->catalog.str)
						)
					)
				);
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set->next())
		{
			try
			{
				GET_STRING(table_catalog)
				GET_STRING(table_schema)
				GET_STRING(table_name)
				GET_STRING(column_name)
				GET_NUMBER(data_type);
				GET_STRING(type_name)
				GET_NUMBER(column_size);
				GET_NUMBER(buffer_length);
				GET_NUMBER(decimal_digits);
				GET_NUMBER(numeric_precision_radix);
				GET_NUMBER(nullable);
				GET_STRING(remarks)
				GET_STRING(column_default)
				GET_NUMBER(sql_data_type);
				GET_NUMBER(sql_datetime_subtype);
				GET_NUMBER(char_octet_length);
				GET_NUMBER(ordinal_position);
				GET_STRING(is_nullable)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
			suspend = FB_TRUE;
		}
		else
			delete procedure->result_set;

		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure find_primary_keys (
//	ctlg ty$pointer not null,
//	table_ varchar(128) character set none [utf8] default null,
//	schema_ varchar(128) character set none [utf8] default null,
//	catalog_ varchar(128) character set none [utf8] default null
//	) returns(
//	table_catalog varchar(128) character set none [utf8],
//	table_schema varchar(128) character set none [utf8],
//	table_name varchar(128) character set none [utf8],
//	primary_key_name varchar(128) character set none [utf8]
//	column_name varchar(128) character set none [utf8],
//	column_number smallint,
//	)
//	external name 'nano!ctlg$find_primary_keys'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$find_primary_keys)

	DECLARE_RESOURCE
	
	enum in : short {
		ctlg = 0, table, schema, catalog
	};

	enum out : short {
		table_catalog = 0, table_schema, table_name, primary_key_name, column_name, column_number
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
		(FB_VARCHAR(128 * 4), table)
		(FB_VARCHAR(128 * 4), schema)
		(FB_VARCHAR(128 * 4), catalog)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), table_catalog)
		(FB_VARCHAR(128 * 4), table_schema)
		(FB_VARCHAR(128 * 4), table_name)
		(FB_VARCHAR(128 * 4), primary_key_name)
		(FB_VARCHAR(128 * 4), column_name)
		(FB_SMALLINT, column_number)
	);

	catalog::primary_keys* result_set;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned in_count;

		AutoArrayDelete<unsigned> in_char_sets;

		AutoRelease<IMessageMetadata> in_metadata(procedure->metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				if (!in->tableNull) U8_VARIYNG(in, table)
				if (!in->schemaNull) U8_VARIYNG(in, schema)
				if (!in->catalogNull) U8_VARIYNG(in, catalog)
				procedure->result_set = new catalog::primary_keys(
					std::move(
						ctlg->find_primary_keys(
							in->tableNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->table.str),
							in->schemaNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->schema.str),
							in->catalogNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->catalog.str)
						)
					)
				);
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set->next())
		{
			try
			{
				GET_STRING(table_catalog)
				GET_STRING(table_schema)
				GET_STRING(table_name)
				GET_STRING(primary_key_name)
				GET_STRING(column_name)
				GET_NUMBER(column_number)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
			suspend = FB_TRUE;
		}
		else
			delete procedure->result_set;

		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure find_procedures(
//	ctlg ty$pointer not null,
//	procedure_ varchar(128) character set none [utf8],
//	schema_ varchar(128) character set none [utf8],
//	catalog_ varchar(128) character set none [utf8]
//	) returns(
//	procedure_catalog varchar(128) character set none [utf8],
//	procedure_schema varchar(128) character set none [utf8],
//	procedure_name varchar(128) character set none [utf8],
//	procedure_type smallint,
//	procedure_remarks varchar(2048) character set none [utf8]
//	)
//	external name 'nano!ctlg$find_procedures'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$find_procedures)

	DECLARE_RESOURCE
	
	enum in : short {
		ctlg = 0, procedure, schema, catalog
	};

	enum out : short {
		procedure_catalog = 0, procedure_schema, procedure_name, procedure_type, procedure_remarks
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
		(FB_VARCHAR(128 * 4), procedure)
		(FB_VARCHAR(128 * 4), schema)
		(FB_VARCHAR(128 * 4), catalog)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), procedure_catalog)
		(FB_VARCHAR(128 * 4), procedure_schema)
		(FB_VARCHAR(128 * 4), procedure_name)
		(FB_SMALLINT, procedure_type)
		(FB_VARCHAR(2048 * 4), procedure_remarks)
	);

	catalog::procedures* result_set;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned in_count;

		AutoArrayDelete<unsigned> in_char_sets;

		AutoRelease<IMessageMetadata> in_metadata(procedure->metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				if (!in->procedureNull) U8_VARIYNG(in, procedure)
				if (!in->schemaNull) U8_VARIYNG(in, schema)
				if (!in->catalogNull) U8_VARIYNG(in, catalog)
				procedure->result_set = new catalog::procedures(
					std::move(
						ctlg->find_procedures(
							in->procedureNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->procedure.str),
							in->schemaNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->schema.str),
							in->catalogNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->catalog.str)
						)
					)
				);
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set->next())
		{
			try
			{
				GET_STRING(procedure_catalog)
				GET_STRING(procedure_schema)
				GET_STRING(procedure_name)
				GET_NUMBER(procedure_type)
				GET_STRING(procedure_remarks)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
			suspend = FB_TRUE;
		}
		else
			delete procedure->result_set;

		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure find_procedure_columns(
//	ctlg ty$pointer not null,
//	column_ varchar(128) character set none [utf8] default null,
//	procedure_ varchar(128) character set none [utf8] default null,
//	schema_ varchar(128) character set none [utf8] default null,
//	catalog_ varchar(128) character set none [utf8] default null
//	) returns (
//	procedure_catalog varchar(128) character set none [utf8],
//	procedure_schema varchar(128) character set none [utf8],
//	procedure_name varchar(128) character set none [utf8],
//	column_name varchar(128) character set none [utf8],
//	column_type smallint,
//	data_type smallint,
//	type_name varchar(128) character set none [utf8],
//	column_size integer,
//	buffer_length integer,
//	decimal_digits smallint,
//	numeric_precision_radix smallint,
//	nullable smallint,
//	remarks varchar(2048) character set none [utf8],
//	column_default varchar(256) character set none [utf8],
//	sql_data_type smallint,
//	sql_datetime_subtype smallint,
//	char_octet_length integer,
//	ordinal_position integer,
//	is_nullable varchar(4) character set none [utf8]
//	)
//	external name 'nano!ctlg$find_procedure_columns'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$find_procedure_columns)

	DECLARE_RESOURCE
	
	enum in : short {
		ctlg = 0, column, procedure, schema, catalog
	};

	enum out : short {
		procedure_catalog = 0, procedure_schema, procedure_name, column_name, column_type, data_type, type_name, column_size, buffer_length,
		decimal_digits, numeric_precision_radix, nullable, remarks, column_default, sql_data_type, sql_datetime_subtype,
		char_octet_length, ordinal_position, is_nullable

	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
		(FB_VARCHAR(128 * 4), column)
		(FB_VARCHAR(128 * 4), procedure)
		(FB_VARCHAR(128 * 4), schema)
		(FB_VARCHAR(128 * 4), catalog)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), procedure_catalog)
		(FB_VARCHAR(128 * 4), procedure_schema)
		(FB_VARCHAR(128 * 4), procedure_name)
		(FB_VARCHAR(128 * 4), column_name)
		(FB_SMALLINT, column_type)
		(FB_SMALLINT, data_type)
		(FB_VARCHAR(128 * 4), type_name)
		(FB_INTEGER, column_size)
		(FB_INTEGER, buffer_length)
		(FB_SMALLINT, decimal_digits)
		(FB_SMALLINT, numeric_precision_radix)
		(FB_SMALLINT, nullable)
		(FB_VARCHAR(2048 * 4), remarks)
		(FB_VARCHAR(256 * 4), column_default)
		(FB_SMALLINT, sql_data_type)
		(FB_SMALLINT, sql_datetime_subtype)
		(FB_INTEGER, char_octet_length)
		(FB_INTEGER, ordinal_position)
		(FB_VARCHAR(4 * 4), is_nullable)
	);

	catalog::procedure_columns* result_set;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned in_count;

		AutoArrayDelete<unsigned> in_char_sets;

		AutoRelease<IMessageMetadata> in_metadata(procedure->metadata->getInputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				if (!in->columnNull) U8_VARIYNG(in, column)
				if (!in->procedureNull) U8_VARIYNG(in, procedure)
				if (!in->schemaNull) U8_VARIYNG(in, schema)
				if (!in->catalogNull) U8_VARIYNG(in, catalog)
				procedure->result_set = new catalog::procedure_columns(
					std::move(
						ctlg->find_procedure_columns(
							in->columnNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->column.str),
							in->procedureNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->procedure.str),
							in->schemaNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->schema.str),
							in->catalogNull ? NANODBC_TEXT("") : NANODBC_TEXT(in->catalog.str)
						)
					)
				);
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set->next())
		{
			try
			{
				GET_STRING(procedure_catalog)
				GET_STRING(procedure_schema)
				GET_STRING(procedure_name)
				GET_STRING(column_name)
				GET_NUMBER(column_type)
				GET_NUMBER(data_type)
				GET_STRING(type_name)
				GET_NUMBER(column_size)
				GET_NUMBER(buffer_length)
				GET_NUMBER(decimal_digits)
				GET_NUMBER(numeric_precision_radix)
				GET_NUMBER(nullable)
				GET_STRING(remarks)
				GET_STRING(column_default)
				GET_NUMBER(sql_data_type)
				GET_NUMBER(sql_datetime_subtype)
				GET_NUMBER(char_octet_length)
				GET_NUMBER(ordinal_position)
				GET_STRING(is_nullable)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
			suspend = FB_TRUE;
		}
		else
			delete procedure->result_set;
		
		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure list_catalogs (
//	ctlg ty$pointer not null, 
//	) returns (
//	catalog_name varchar(128) character set none [utf8]
//	)
//	external name 'nano!ctlg$list_catalogs'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$list_catalogs)

	DECLARE_RESOURCE
	
	enum out : short {
		catalog_name = 0
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), catalog_name)
	);

	std::list<std::string> result_set;
	std::list<std::string>::iterator result_set_it;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				procedure->result_set = std::move(ctlg->list_catalogs());
				procedure->result_set_it = procedure->result_set.begin();
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set_it != procedure->result_set.end())
		{
			std::string catalog_name = *procedure->result_set_it;
			FB_VARIYNG(out->catalog_name, catalog_name)
			U8_VARIYNG(out, catalog_name)
			out->catalog_nameNull = FB_FALSE;
			suspend = FB_TRUE;
		}
		else
		{
			if (!procedure->result_set.empty())	procedure->result_set.clear();
		}
		return;
	}
			
FB_UDR_END_PROCEDURE

//-----------------------------------------------------------------------------
// create procedure list_schemas (
//	ctlg ty$pointer not null, 
//	) returns (
//	schema_name varchar(128) character set none [utf8]
//	)
//	external name 'nano!ctlg$list_schemas'
//	engine udr;

FB_UDR_BEGIN_PROCEDURE(ctlg$list_schemas)

	DECLARE_RESOURCE
	
	enum out : short {
		schema_name = 0
	};

	AutoArrayDelete<unsigned> fetch_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, ctlg)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), schema_name)
	);

	std::list<std::string> result_set;
	std::list<std::string>::iterator result_set_it;

	FB_UDR_EXECUTE_PROCEDURE
	{
		PROCEDURE_RESOURCES

		unsigned out_count;
		
		AutoRelease<IMessageMetadata> out_metadata(procedure->metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		procedure->fetch_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			procedure->fetch_char_sets[i] = out_metadata->getCharSet(status, i);
		}

		if (!in->ctlgNull)
		{
			nanoudr::catalog* ctlg = helper.native_ptr<nanoudr::catalog>(in->ctlg.str);
			try
			{
				procedure->result_set = std::move(ctlg->list_schemas());
				procedure->result_set_it = procedure->result_set.begin();
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_CATALOG)
	}

	NANOUDR_FETCH_PROCEDURE
	{
		FETCH_RESOURCES
		suspend = FB_FALSE;
		if (procedure->result_set_it != procedure->result_set.end())
		{
			std::string schema_name = *procedure->result_set_it;
			FB_VARIYNG(out->schema_name, schema_name)
			U8_VARIYNG(out, schema_name)
			out->schema_nameNull = FB_FALSE;
			++procedure->result_set_it;
			suspend = FB_TRUE;
		}
		else
		{
			if (!procedure->result_set.empty()) procedure->result_set.clear();
		}
		return;
	}
			
FB_UDR_END_PROCEDURE

} // namespace nanoudr