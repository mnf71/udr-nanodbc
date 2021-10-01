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
	nanodbc::result::~result();
}

nanoudr::connection* result::connection()
{
	return conn_;
}

//-----------------------------------------------------------------------------
// create function release_ (
//	 rslt ty$pointer not null 
// ) returns ty$pointer
// external name 'nano!rslt$release'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$release)

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
		NANOUDR_RESOURCES
		out->rsltNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				att_resources->results.release(rslt);
			}
			catch (std::runtime_error const& e)
			{
				udr_helper.fb_ptr(out->rslt.str, udr_helper.native_ptr(in->rslt.str));
				out->rsltNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function valid (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$valid)

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
		NANOUDR_RESOURCES
		out->valid =
			in->rsltNull ?
			udr_helper.fb_bool(false) :
			att_resources->results.valid(udr_helper.rslt_ptr(in->rslt.str));
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 rslt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!rslt$connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$connection)

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
		NANOUDR_RESOURCES
		out->connNull = FB_TRUE;
		nanoudr::result * rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				nanoudr::connection* conn = rslt->connection();
				udr_helper.fb_ptr(out->conn.str, (int64_t)conn);
				out->connNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rowset_size  ( 
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$rowset_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$rowset_size)

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
		NANOUDR_RESOURCES
		out->sizeNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
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
			NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$affected_rows)

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
		NANOUDR_RESOURCES
		out->affectedNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
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
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function has_affected_rows (
//	 rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt$has_affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$has_affected_rows)

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
		NANOUDR_RESOURCES
		out->has_affectedNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->has_affected = udr_helper.fb_bool(rslt->has_affected_rows());
				out->has_affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rows_ (
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$rows)

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
		NANOUDR_RESOURCES
		out->rowsNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
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
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function columns (
//	 rslt ty$pointer not null, 
//	) returns smallint
//	external name 'nano!rslt$columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$columns)

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
		NANOUDR_RESOURCES
		out->columnsNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
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
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function first_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$first'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$first)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->first());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function last_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$last'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$last)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->last());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$next'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$next)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->next());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prior_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$prior'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$prior)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->prior());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function move_ (
//	 rslt ty$pointer not null, 
//	 row_ integer not null 
//	) returns boolean
//	external name 'nano!rslt$move'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$move)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->move(in->row));
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function skip_ (
//	 rslt ty$pointer not null, 
//	 row_ integer not null 
//	) returns boolean
//	external name 'nano!rslt$skip'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$skip)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->skip(in->rows));
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function position_ (
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt$position'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$position)

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
		NANOUDR_RESOURCES
		out->positionNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
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
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function at_end (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt$at_end'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$at_end)

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
		NANOUDR_RESOURCES
		out->at_endNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->at_end = udr_helper.fb_bool(rslt->at_end());
				out->at_endNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function unbind (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns ty$nano_blank
//	external name 'nano!rslt$unbind'
//	engine udr; 
//
// unbind (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$unbind)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->blankNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!in->columnNull)
				{
					if (!isdigit(in->column.str[0]))
						rslt->unbind(NANODBC_TEXT(in->column.str));
					else
						rslt->unbind((short)atoi(in->column.str));
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
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// todo: template <class T> T get
//
// create function get (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns boolean
//	external name 'nano!rslt$is_null'
//	engine udr; 
//

//-----------------------------------------------------------------------------
// create function is_null (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns boolean
//	external name 'nano!rslt$is_null'
//	engine udr; 
//
// is_null (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$is_null)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, is_null)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->is_nullNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!isdigit(in->column.str[0]))
					out->is_null = 
						udr_helper.fb_bool(rslt->is_null(NANODBC_TEXT(in->column.str)));
				else
					out->is_null = 
						udr_helper.fb_bool(rslt->is_null((short)atoi(in->column.str)));
				out->is_nullNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_bound (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns boolean
//	external name 'nano!rslt$is_bound'
//	engine udr; 
//
// is_bound (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$is_bound)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, is_null)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->is_nullNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!isdigit(in->column.str[0]))
					out->is_null = 
						udr_helper.fb_bool(rslt->is_bound(NANODBC_TEXT(in->column.str)));
				else
					out->is_null = 
						udr_helper.fb_bool(rslt->is_bound((short)atoi(in->column.str)));
				out->is_nullNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_ (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns smallint
//	external name 'nano!rslt$column'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$column)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, index)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->indexNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				out->index = rslt->column(NANODBC_TEXT(in->column.str));
				out->indexNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_name (
//	 rslt ty$pointer not null, 
//	 index smallint not null 
//	) returns varchar(63) character set utf8
//	external name 'nano!rslt$column_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$column_name)

	unsigned out_count;

	enum out : short {
		column = 0
	};

	AutoArrayDelete<unsigned> out_char_sets;

	FB_UDR_CONSTRUCTOR
	{
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
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->columnNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				nanodbc::string column_name = rslt->column_name(NANODBC_TEXT(in->index));
				FB_VARIYNG(out->column, column_name);
				U8_VARIYNG(out, column);
				out->columnNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_size (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt$column_size'
//	engine udr; 
//
// column_size (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_size)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->sizeNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!isdigit(in->column.str[0]))
					out->size = 
						rslt->column_size(NANODBC_TEXT(in->column.str));
				else
					out->size = 
						rslt->column_size((short)atoi(in->column.str));
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_decimal_digits (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt$column_decimal_digits'
//	engine udr; 
//
// column_decimal_digits (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_decimal_digits)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, digits)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->digitsNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!isdigit(in->column.str[0]))
					out->digits = 
						rslt->column_decimal_digits(NANODBC_TEXT(in->column.str));
				else
					out->digits = 
						rslt->column_decimal_digits((short)atoi(in->column.str));
				out->digitsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt$column_datatype'
//	engine udr; 
//
// column_datatype (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_datatype)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, datatype)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->datatypeNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!isdigit(in->column.str[0]))
					out->datatype = 
						rslt->column_datatype(NANODBC_TEXT(in->column.str));
				else
					out->datatype = 
						rslt->column_datatype((short)atoi(in->column.str));
				out->datatypeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype_name (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns varchar(63) character set utf8
//	external name 'nano!rslt$column_datatype_name'
//	engine udr; 
//
// column_datatype_name (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_datatype_name)

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
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), datatype_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->datatype_nameNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				nanodbc::string datatype_name;
				if (!isdigit(in->column.str[0]))
					datatype_name = 
						rslt->column_datatype_name(NANODBC_TEXT(in->column.str));
				else
					datatype_name = 
						rslt->column_datatype_name((short)atoi(in->column.str));
				FB_VARIYNG(out->datatype_name, datatype_name);
				U8_VARIYNG(out, datatype_name);
				out->datatype_nameNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_c_datatype (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt$column_c_datatype'
//	engine udr; 
//
// column_c_datatype (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt$column_c_datatype)

	unsigned in_count;

	enum in : short {
		rslt = 0, column
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
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, c_datatype)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		NANOUDR_RESOURCES
		out->c_datatypeNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				U8_VARIYNG(in, column);
				if (!isdigit(in->column.str[0]))
					out->c_datatype = 
						rslt->column_c_datatype(NANODBC_TEXT(in->column.str));
				else
					out->c_datatype = 
						rslt->column_c_datatype((short)atoi(in->column.str));
				out->c_datatypeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_result (
//	 rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt$next_result'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$next_result)

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
		NANOUDR_RESOURCES
		out->succesNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				out->succes = udr_helper.fb_bool(rslt->next_result());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function has_data (
//	 rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt$has_data'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt$has_data)

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
		NANOUDR_RESOURCES
		out->has_dataNull = FB_TRUE;
		nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
		if (!in->rsltNull && att_resources->results.valid(rslt))
		{
			try
			{
				bool has_data = static_cast<nanodbc::result*>(rslt);
				out->has_data = udr_helper.fb_bool(has_data);
				out->has_dataNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(POINTER_RSLT_INVALID)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr::

