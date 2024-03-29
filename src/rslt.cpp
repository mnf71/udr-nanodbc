/*
 *  The contents of this file are subject to the Initial
 *  Developer's Public License Version 1.0 (the "License");
 *  you may not use this file except in compliance with the
 *  License. You may obtain a copy of the License at
 *  http://www.ibphoenix.com/main.nfs?a=ibphoenix&page=ibp_idpl.
 *
 *  Software distributed under the License is distributed AS IS,r
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
// package nano$rslt
//

namespace nanoudr
{

//-----------------------------------------------------------------------------
// UDR Result class implementation
//

result::result(
	class attachment_resources& att_resources, class nanoudr::connection& conn, class nanodbc::result&& rslt)
	: nanodbc::result(std::move(rslt))
{
	att_resources_ = &att_resources;
	att_resources_->results.retain(this);
	conn_ = &conn;
}

result::~result()
{
	att_resources_->results.release(this);
}

nanoudr::connection* result::connection()
{
	return conn_;
}

void result::get_value
(
	short column, 
	unsigned char* message, 
	unsigned message_type, unsigned message_length, unsigned message_char_set, unsigned message_offset,	unsigned message_null_offset,
	IUtil* utl
)
{
	ISC_SHORT& null_flag = *(reinterpret_cast<ISC_SHORT*>(message + message_null_offset));
	if (is_null(column)) null_flag = FB_TRUE;
	else
	{
		switch (message_type)
		{
			case SQL_TEXT: // char
			{
				nanodbc::string value;
				get_ref<nanodbc::string>(column, value);
				ISC_USHORT length = static_cast<ISC_USHORT>(value.length());

				if (message_char_set == fb_char_set::CS_UTF8)
					length =
						helper.utf8_out(att_resources_, 
							reinterpret_cast<char*>(message + message_offset), static_cast<ISC_USHORT>(message_length), 
							value.c_str(), length);
				else
					memcpy(reinterpret_cast<char*>(message + message_offset), value.c_str(), std::min(static_cast<ISC_USHORT>(message_length), length));
				
				if (message_length > length)
					memset(reinterpret_cast<char*>(message + message_offset) + length, ' ', message_length - length);

				null_flag = FB_FALSE;
				break;
			}
			case SQL_VARYING: // varchar
			{
				nanodbc::string value;
				get_ref<nanodbc::string>(column, value);
				ISC_USHORT length = static_cast<ISC_USHORT>(value.length());

				if (message_char_set == fb_char_set::CS_UTF8)
					*(reinterpret_cast<ISC_USHORT*>(message + message_offset)) =
						helper.utf8_out(att_resources_, 
							reinterpret_cast<char*>(message + sizeof(ISC_USHORT) + message_offset), static_cast<ISC_USHORT>(message_length), 
							value.c_str(), length);
				else
				{
					length = std::min(static_cast<ISC_USHORT>(message_length), length);
					memcpy(reinterpret_cast<char*>(message + sizeof(ISC_USHORT) + message_offset), value.c_str(), length);
					*(reinterpret_cast<ISC_USHORT*>(message + message_offset)) = length;
				}

				null_flag = FB_FALSE;
				break;
			}
			case SQL_SHORT: // smallint
			{
				*(reinterpret_cast<ISC_SHORT*>(message + message_offset)) = get<ISC_SHORT>(column);
				null_flag = FB_FALSE;
				break;
			}
			case SQL_LONG: // integer 
			{
				*(reinterpret_cast<ISC_LONG*>(message + message_offset)) = get<ISC_LONG>(column);
				null_flag = FB_FALSE;
				break;
			}
			case SQL_FLOAT: // float
			{
				*(reinterpret_cast<float*>(message + message_offset)) = get<float>(column);
				null_flag = FB_FALSE;
				break;
			}
			case SQL_DOUBLE: // double precision 
			case SQL_D_FLOAT:
			{
				*(reinterpret_cast<double*>(message + message_offset)) = get<double>(column);
				null_flag = FB_FALSE;
				break;
			}
			case SQL_TIMESTAMP: // timestamp
			{
				nanodbc::timestamp value;
				get_ref<nanodbc::timestamp>(column, value);
				nanoudr::timestamp ts = helper.get_timestamp(&value);

				Firebird::FbTimestamp fb;
				fb.date.encode(utl, ts.d.year, ts.d.month, ts.d.day);
				fb.time.encode(utl, ts.t.hour, ts.t.min, ts.t.sec, ts.t.fract);
				// This class has memory layout identical to ISC_TIMESTAMP
				memcpy(reinterpret_cast<ISC_TIMESTAMP*>(message + message_offset), &fb, sizeof(FbTimestamp));

				null_flag = FB_FALSE;
				break;
			}
			case SQL_BLOB: // blob
			{
				if (column_c_datatype(column) == -2) // SQL_C_BINARY...
				{
					std::vector<std::uint8_t> value;
					get_ref<std::vector<std::uint8_t>>(column, value);
					helper.write_blob(att_resources_, &value, reinterpret_cast<ISC_QUAD*>(message + message_offset));
				}
				else // SQL_C_[W]CHAR... char datatype return BLOB Subtype TEXT
				{
					nanodbc::string value;
					get_ref<nanodbc::string>(column, value);
					helper.write_blob(att_resources_, &value, reinterpret_cast<ISC_QUAD*>(message + message_offset));
				}
				null_flag = FB_FALSE;
				break;
			}
			case SQL_ARRAY: // array
			{
				throw std::runtime_error("Fetching SQL_ARRAY not supported.");
				break;
			}
			case SQL_QUAD: // blob_id 
			{
				throw std::runtime_error("Fetching SQL_QUAD not supported.");
				break;
			}
			case SQL_TYPE_TIME: // time
			{
				nanodbc::time value;
				get_ref<nanodbc::time>(column, value);
				nanoudr::time t = helper.get_time(&value);

				Firebird::FbTime fb;
				fb.encode(utl, t.hour, t.min, t.sec, t.fract);
				// This class has memory layout identical to ISC_TIME
				memcpy(reinterpret_cast<ISC_TIME*>(message + message_offset), &fb, sizeof(FbTime));

				null_flag = FB_FALSE;
				break;
			}
			case SQL_TYPE_DATE: // date
			{
				nanodbc::date value;
				get<nanodbc::date>(column, value);
				nanoudr::date d = helper.get_date(&value);

				Firebird::FbDate fb;
				fb.encode(utl, d.year, d.month, d.day);
				// This class has memory layout identical to ISC_TIME
				memcpy(reinterpret_cast<ISC_DATE*>(message + message_offset), &fb, sizeof(FbDate));

				null_flag = FB_FALSE;
				break;
			}
			case SQL_INT64: // bigint
			{
				*(reinterpret_cast<ISC_INT64*>(message + message_offset)) = get<ISC_INT64>(column);
				null_flag = FB_FALSE;
				break;
			}
			case SQL_BOOLEAN: // boolean
			{
				bool value = get<unsigned short>(column) ? true : false;
				*(reinterpret_cast<FB_BOOLEAN*>(message + message_offset)) = helper.fb_bool(value);
				null_flag = FB_FALSE;
				break;
			}
			case SQL_NULL: // null, nothing
			{
				break;
			}
			default:
			{
				throw std::runtime_error("Fetching unknow Firebird SQL datatype.");
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// create function valid (
//	rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$valid)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, valid)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->valid = helper.fb_bool(
			in->rsltNull ? false :
				att_resources->results.valid(helper.native_ptr<result>(in->rslt.str))
			);
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function release_ (
//	rslt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!rslt$release'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$release)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->rsltNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
			try
			{
				if (att_resources->results.valid(rslt)) delete (nanoudr::result*)(rslt);
			}
			catch (std::runtime_error const& e)
			{
				helper.fb_ptr(out->rslt.str, rslt);
				out->rsltNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	rslt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!rslt$connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$connection)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, conn)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->connNull = FB_TRUE;
		nanoudr::result * rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				nanoudr::connection* conn = rslt->connection();
				helper.fb_ptr(out->conn.str, conn);
				out->connNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rowset_size  ( 
//	rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$rowset_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$rowset_size)
	
	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->sizeNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->size = rslt->rowset_size();
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$affected_rows)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, affected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->affectedNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->affected = rslt->affected_rows();
				out->affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function has_affected_rows (
//	rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt$has_affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$has_affected_rows)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, has_affected)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->has_affectedNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->has_affected = helper.fb_bool(rslt->has_affected_rows());
				out->has_affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rows_ (
//	rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$rows)
	
	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, rows)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->rowsNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->rows = rslt->rows();
				out->rowsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function columns (
//	rslt ty$pointer not null, 
//	) returns smallint
//	external name 'nano!rslt$columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$columns)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, columns)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->columnsNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->columns = rslt->columns();
				out->columnsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function first_ (
//	rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$first'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$first)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->first());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function last_ (
//	rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$last'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$last)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->last());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_ (
//	rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$next'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$next)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->next());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prior_ (
//	rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$prior'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$prior)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->prior());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function move_ (
//	rslt ty$pointer not null, 
//	row_ integer not null 
//	) returns boolean
//	external name 'nano!rslt$move'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$move)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_INTEGER, row)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->move(in->row));
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function skip_ (
//	rslt ty$pointer not null, 
//	row_ integer not null 
//	) returns boolean
//	external name 'nano!rslt$skip'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$skip)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_INTEGER, rows)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->skip(in->rows));
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function position_ (
//	rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$position'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$position)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, position)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->positionNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->position = rslt->position();
				out->positionNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function at_end (
//	rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$at_end'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$at_end)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, at_end)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->at_endNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->at_end = helper.fb_bool(rslt->at_end());
				out->at_endNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function unbind (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns ty$nano_blank
//	external name 'nano!rslt$unbind'
//	engine udr; 
//
// unbind (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$unbind)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!in->columnNull)
				{
					if (!isdigit(in->column.str[0]))
						rslt->unbind(NANODBC_TEXT(in->column.str));
					else
						rslt->unbind(static_cast<short>(atoi(in->column.str)));
				}
				else
					rslt->unbind();
				out->blank = BLANK;
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// template <class T> T get (...
//
// create function get (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns native Firebird datatype
//	external name 'nano!rslt$get'
//	engine udr; 
//
// get (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$get)

	DECLARE_RESOURCE

	const unsigned in_count = 2; 

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	const unsigned out_count = 1; // function returns only one value

	enum out : short {
		value = 0
	};

	AutoArrayDelete<unsigned> value_type;
	AutoArrayDelete<unsigned> value_length;
	AutoArrayDelete<unsigned> value_char_set;
	AutoArrayDelete<unsigned> value_offset;
	AutoArrayDelete<unsigned> value_null_offset;

	IUtil* utl = master->getUtilInterface();

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));
		AutoRelease<IMessageMetadata> out_metadata(metadata->getOutputMetadata(status));

		in_char_sets.reset(new unsigned[in_count]);
		in_char_sets[in::rslt] = fb_char_set::CS_NONE;
		in_char_sets[in::column] = in_metadata->getCharSet(status, in::column);

		value_type.reset(new unsigned[out_count]);
		value_offset.reset(new unsigned[out_count]);
		value_null_offset.reset(new unsigned[out_count]);

		value_type[out::value] = out_metadata->getType(status, out::value);
		value_offset[out::value] = out_metadata->getOffset(status, out::value);
		value_null_offset[out::value] = out_metadata->getNullOffset(status, out::value);

		if (value_type[out::value] == SQL_TEXT || value_type[out::value] == SQL_VARYING)
		{
			value_length.reset(new unsigned[out_count]);
			value_char_set.reset(new unsigned[out_count]);
			value_length[out::value] = out_metadata->getLength(status, out::value);
			value_char_set[out::value] = out_metadata->getCharSet(status, out::value);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		*(reinterpret_cast<ISC_SHORT*>(out + value_null_offset[out::value])) = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)

				short column_position = -1;
				if (isdigit(in->column.str[0]))
					column_position = static_cast<short>(atoi(in->column.str)); else
					column_position = rslt->column(NANODBC_TEXT(in->column.str));
				try 
				{
					rslt->get_value(
						column_position,
						out, 
						value_type[out::value], 
						value_length ? value_length[out::value] : -1, 
						value_char_set ? value_char_set[out::value] : fb_char_set::CS_NONE, 
						value_offset[out::value],	
						value_null_offset[out::value],
						utl
					);
				}
				catch (std::runtime_error const& e) {
					NANOUDR_THROW(FETCHING_ERROR, e.what())
				}
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION


//-----------------------------------------------------------------------------
// create function pump (
//	rslt ty$pointer not null, 
//	query varchar(8191) character set none [utf8] not null,
//	transaction_pack integer not null default 0
//	) returns integer
//	external name 'nano!rslt$pump'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$pump)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, query, transaction_pack
	};

	AutoArrayDelete<unsigned> in_char_sets;

	AutoRelease<IAttachment> att;
	AutoRelease<ITransaction> tra;
	
	AutoDispose<IXpbBuilder> tpb;

	AutoRelease<IStatement> pump_stmt;
	AutoRelease<IMessageMetadata> pump_meta;
	
	unsigned pump_count;

	AutoArrayDelete<unsigned> pump_type;
	AutoArrayDelete<unsigned> pump_length;
	AutoArrayDelete<unsigned> pump_char_set;
	AutoArrayDelete<unsigned> pump_offset;
	AutoArrayDelete<unsigned> pump_null_offset;
	AutoArrayDelete<unsigned char> pump_buffer;

	IUtil* utl = master->getUtilInterface();

	bool transaction_packed;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(8191 * 4), query)
		(FB_INTEGER, transaction_pack)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, pumped_records)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->pumped_records = 0;
		out->pumped_recordsNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				if (in->queryNull || !strcmp(in->query.str, ""))
					throw std::runtime_error("Statement of pump null or empty.");
				else
				{
					U8_VARIYNG(in, query)
					transaction_packed = 
						(!in->transaction_packNull && in->transaction_pack > 0) ? true : false;
					try
					{
						att.reset(context->getAttachment(status));
						if (!transaction_packed)
							tra.reset(context->getTransaction(status));
						else
						{
							tpb.reset(utl->getXpbBuilder(status, IXpbBuilder::TPB, NULL, 0));
							tpb->insertTag(status, isc_tpb_read_committed);
							tpb->insertTag(status, isc_tpb_rec_version);
							tpb->insertTag(status, isc_tpb_nowait);
							tpb->insertTag(status, isc_tpb_write);
							tra.reset(att->startTransaction(status, tpb->getBufferLength(status), tpb->getBuffer(status)));
						}
						att_resources->current_transaction(tra); // BLOB context of operation (needed erase after using)
						pump_stmt.reset(att->prepare(status, tra, 0, in->query.str, SQL_DIALECT_CURRENT, IStatement::PREPARE_PREFETCH_METADATA));
						pump_meta.reset(pump_stmt->getInputMetadata(status));

						pump_count = pump_meta->getCount(status);
						if (pump_count != static_cast<unsigned>(rslt->columns())) throw std::runtime_error("Pump of read-write columns does not equal.");
						pump_type.reset(new unsigned[pump_count]);
						pump_length.reset(new unsigned[pump_count]);
						pump_char_set.reset(new unsigned[pump_count]);
						pump_offset.reset(new unsigned[pump_count]);
						pump_null_offset.reset(new unsigned[pump_count]);
						for (unsigned i = 0; i < pump_count; ++i)
						{
							pump_type[i] = pump_meta->getType(status, i);
							pump_length[i] = pump_meta->getLength(status, i);
							pump_char_set[i] = pump_meta->getCharSet(status, i);
							pump_offset[i] = pump_meta->getOffset(status, i);
							pump_null_offset[i] = pump_meta->getNullOffset(status, i);
						}
						pump_buffer.reset(new unsigned char[pump_meta->getMessageLength(status)]);

						ISC_LONG pack_index = 0;
						while (rslt->next())
						{
							pack_index += 1; 
							for (short column = 0, columns = rslt->columns(); column < columns; ++column)
							{
								rslt->get_value(
									column,
									pump_buffer,
									pump_type[column],
									pump_length ? pump_length[column] : -1,
									pump_char_set ? pump_char_set[column] : fb_char_set::CS_NONE,
									pump_offset[column],
									pump_null_offset[column],
									utl
								);
							}
							pump_stmt->execute(status, tra, pump_meta, pump_buffer, NULL, NULL);

							// commit pack pamped records
							if (transaction_packed && pack_index == in->transaction_pack)
							{
								tra->commitRetaining(status); // commit transaction retaining
								pack_index = 0;
							}

							out->pumped_records += 1;
						}
						pump_stmt->free(status); // will close interface
						pump_stmt.release();
						att_resources->current_transaction(nullptr);
						if (transaction_packed)
						{
							tra->commit(status); // will close interface
							tra.release();
						}
					}
					catch (const FbException& e)
					{
						char what[256];
						utl->formatStatus(what, sizeof(what), e.getStatus());
						throw std::runtime_error(what);
					}
				}
				out->pumped_recordsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANOUDR_THROW(PUMPING_ERROR, e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_null (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns boolean
//	external name 'nano!rslt$is_null'
//	engine udr; 
//
// is_null (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$is_null)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, is_null)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->is_nullNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!isdigit(in->column.str[0]))
					out->is_null = 
						helper.fb_bool(rslt->is_null(NANODBC_TEXT(in->column.str)));
				else
					out->is_null = 
						helper.fb_bool(rslt->is_null(static_cast<short>(atoi(in->column.str))));
				out->is_nullNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_bound (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns boolean
//	external name 'nano!rslt$is_bound'
//	engine udr; 
//
// is_bound (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$is_bound)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, is_bound)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->is_boundNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!isdigit(in->column.str[0]))
					out->is_bound =
						helper.fb_bool(rslt->is_bound(NANODBC_TEXT(in->column.str)));
				else
					out->is_bound =
						helper.fb_bool(rslt->is_bound(static_cast<short>(atoi(in->column.str))));
				out->is_boundNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				if (strcmp(e.what(), "index out of range") == 0) {
					out->is_bound = false; // hide this exception, simply check availability column
					out->is_boundNull = FB_FALSE;
				}
				else
					NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_ (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns smallint
//	external name 'nano!rslt$column'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$column)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, index)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->indexNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				out->index = rslt->column(NANODBC_TEXT(in->column.str));
				out->indexNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_name (
//	rslt ty$pointer not null, 
//	index smallint not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!rslt$column_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$column_name)

	DECLARE_RESOURCE

	unsigned out_count;

	enum out : short {
		column = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

		AutoRelease<IMessageMetadata> out_metadata(metadata->getOutputMetadata(status));

		out_count = out_metadata->getCount(status);
		out_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			out_char_sets[i] = out_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_SMALLINT, index)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->columnNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				nanodbc::string column_name = rslt->column_name(NANODBC_TEXT(in->index));
				FB_VARIYNG(out->column, column_name)
				U8_VARIYNG(out, column)
				out->columnNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_size (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns integer
//	external name 'nano!rslt$column_size'
//	engine udr; 
//
// column_size (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_size)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->sizeNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!isdigit(in->column.str[0]))
					out->size = 
						rslt->column_size(NANODBC_TEXT(in->column.str));
				else
					out->size = 
						rslt->column_size(static_cast<short>(atoi(in->column.str)));
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_decimal_digits (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns integer
//	external name 'nano!rslt$column_decimal_digits'
//	engine udr; 
//
// column_decimal_digits (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_decimal_digits)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, digits)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->digitsNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!isdigit(in->column.str[0]))
					out->digits = 
						rslt->column_decimal_digits(NANODBC_TEXT(in->column.str));
				else
					out->digits = 
						rslt->column_decimal_digits(static_cast<short>(atoi(in->column.str)));
				out->digitsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns integer
//	external name 'nano!rslt$column_datatype'
//	engine udr; 
//
// column_datatype (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_datatype)

	DECLARE_RESOURCE	

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, datatype)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->datatypeNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!isdigit(in->column.str[0]))
					out->datatype = 
						rslt->column_datatype(NANODBC_TEXT(in->column.str));
				else
					out->datatype = 
						rslt->column_datatype(static_cast<short>(atoi(in->column.str)));
				out->datatypeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype_name (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns varchar(128) character set none [utf8]
//	external name 'nano!rslt$column_datatype_name'
//	engine udr; 
//
// column_datatype_name (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_datatype_name)

	DECLARE_RESOURCE

	unsigned in_count, out_count;

	enum in : short {
		rslt = 0, column
	};

	enum out : short {
		datatype_name = 0
	};

	AutoArrayDelete<unsigned> in_char_sets;
	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

		AutoRelease<IMessageMetadata> in_metadata(metadata->getInputMetadata(status));
		AutoRelease<IMessageMetadata> out_metadata(metadata->getOutputMetadata(status));

		in_count = in_metadata->getCount(status);
		in_char_sets.reset(new unsigned[in_count]);
		for (unsigned i = 0; i < in_count; ++i)
		{
			in_char_sets[i] = in_metadata->getCharSet(status, i);
		}

		out_count = out_metadata->getCount(status);
		out_char_sets.reset(new unsigned[out_count]);
		for (unsigned i = 0; i < out_count; ++i)
		{
			out_char_sets[i] = out_metadata->getCharSet(status, i);
		}
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), datatype_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->datatype_nameNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				nanodbc::string datatype_name;
				if (!isdigit(in->column.str[0]))
					datatype_name = 
						rslt->column_datatype_name(NANODBC_TEXT(in->column.str));
				else
					datatype_name = 
						rslt->column_datatype_name(static_cast<short>(atoi(in->column.str)));
				FB_VARIYNG(out->datatype_name, datatype_name)
				U8_VARIYNG(out, datatype_name)
				out->datatype_nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_c_datatype (
//	rslt ty$pointer not null, 
//	column_ varchar(128) character set none [utf8] not null 
//	) returns integer
//	external name 'nano!rslt$column_c_datatype'
//	engine udr; 
//
// column_c_datatype (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_c_datatype)

	DECLARE_RESOURCE

	unsigned in_count;

	enum in : short {
		rslt = 0, column
	};

	AutoArrayDelete<unsigned> in_char_sets;

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES

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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(128 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, c_datatype)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->c_datatypeNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column)
				if (!isdigit(in->column.str[0]))
					out->c_datatype = 
						rslt->column_c_datatype(NANODBC_TEXT(in->column.str));
				else
					out->c_datatype = 
						rslt->column_c_datatype(static_cast<short>(atoi(in->column.str)));
				out->c_datatypeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_result (
//	rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt$next_result'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$next_result)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, succes)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = helper.fb_bool(rslt->next_result());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function has_data (
//	rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt$has_data'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$has_data)

	DECLARE_RESOURCE

	FB_UDR_CONSTRUCTOR
	{
		INITIALIZE_RESORCES
	}

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, has_data)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		FUNCTION_RESOURCES
		out->has_dataNull = FB_TRUE;
		nanoudr::result* rslt = helper.native_ptr<result>(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				bool has_data = static_cast<nanodbc::result*>(rslt);
				out->has_data = helper.fb_bool(has_data);
				out->has_dataNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RESULT)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr::


