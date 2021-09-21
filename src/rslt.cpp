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

nanodbc::result* result::native()
{
	return &rslt_;
}

result::result(class nanoudr::connection& conn, class nanodbc::result& rslt)
{
	udr_resources.results.retain(this);
	rslt_ = std::move(rslt);
	conn_ = &conn;
}

result::~result()
{
}

nanoudr::connection* result::connection()
{
	return conn_;
}

//-----------------------------------------------------------------------------
// create function release_ (
//	 rslt ty$pointer not null 
// ) returns ty$pointer
// external name 'nano!rslt_release'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_release)

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
		out->rsltNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				udr_resources.results.release(rslt);
			}
			catch (std::runtime_error const& e)
			{
				udr_helper.fb_ptr(out->rslt.str, udr_helper.native_ptr(in->rslt.str));
				out->rsltNull = FB_FALSE;
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_valid (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt_is_valid'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_is_valid)

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
		out->valid =
			in->rsltNull ?
			udr_helper.fb_bool(false) :
			udr_resources.results.is_valid(udr_helper.rslt_ptr(in->rslt.str));
		out->validNull = FB_FALSE;
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function connection (
//	 rslt ty$pointer not null 
//	) returns ty$pointer
//	external name 'nano!rslt_connection'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_connection)

FB_UDR_MESSAGE(
	InMessage,
	(NANO_POINTER, rslt)
);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_POINTER, stmt)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->stmtNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					nanoudr::connection* conn = rslt->connection();
					udr_helper.fb_ptr(out->stmt.str, (int64_t)conn);
					out->stmtNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rowset_size  ( 
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt_rowset_size'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_rowset_size)

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
		out->sizeNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->size = rslt->native()->rowset_size();
					out->sizeNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt_affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_affected_rows)

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
		out->affectedNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->affected = rslt->native()->affected_rows();
					out->affectedNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function has_affected_rows (
//	 rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt_has_affected_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_has_affected_rows)

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
		out->has_affectedNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->has_affected = 
						udr_helper.fb_bool(rslt->native()->has_affected_rows());
					out->has_affectedNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rows_ (
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt_rows'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_rows)

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
		out->rowsNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->rows = rslt->native()->rows();
					out->rowsNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function columns (
//	 rslt ty$pointer not null, 
//	) returns smallint
//	external name 'nano!rslt_columns'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_columns)

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
		out->columnsNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->columns = rslt->native()->columns();
					out->columnsNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function first_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt_first'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_first)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->first());
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function last_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt_last'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_last)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->last());
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt_next'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_next)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->next());
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function prior_ (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt_prior'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_prior)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->prior());
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function move_ (
//	 rslt ty$pointer not null, 
//	 row_ integer not null 
//	) returns boolean
//	external name 'nano!rslt_move'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_move)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->move(in->row));
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function skip_ (
//	 rslt ty$pointer not null, 
//	 row_ integer not null 
//	) returns boolean
//	external name 'nano!rslt_skip'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_skip)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->skip(in->rows));
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function position_ (
//	 rslt ty$pointer not null 
//	) returns integer
//	external name 'nano!rslt_position'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_position)

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
		out->positionNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->position = rslt->native()->position();
					out->positionNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function at_end (
//	 rslt ty$pointer not null, 
//	) returns boolean
//	external name 'nano!rslt_at_end'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_at_end)

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
		out->at_endNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->at_end = udr_helper.fb_bool(rslt->native()->at_end());
					out->at_endNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function unbind (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns ty$nano_blank
//	external name 'nano!rslt_unbind'
//	engine udr; 
//
// \brief
// unbind (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_unbind)

	unsigned in_count;

	enum in : short {
		rslt = 0, column_
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
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, blank)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->blankNull = FB_TRUE;
		if (!in->rsltNull)
		{
			out->blank = BLANK;
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column_);
					if (!in->column_Null)
					{
						if (!isdigit(in->column_.str[0]))
							rslt->native()->unbind(NANODBC_TEXT(in->column_.str));
						else
							rslt->native()->unbind((short)atoi(in->column_.str));
					}
					else
						rslt->native()->unbind();
					out->blankNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// todo: template <class T> T get

//-----------------------------------------------------------------------------
// create function is_null (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns boolean
//	external name 'nano!rslt_is_null'
//	engine udr; 
//
// \brief
// is_null (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_is_null)

	unsigned in_count;

	enum in : short {
		rslt = 0, column_
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
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, is_null)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->is_nullNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column_);
					if (!isdigit(in->column_.str[0]))
						out->is_null = 
							udr_helper.fb_bool(rslt->native()->is_null(NANODBC_TEXT(in->column_.str)));
					else
						out->is_null = 
							udr_helper.fb_bool(rslt->native()->is_null((short)atoi(in->column_.str)));
					out->is_nullNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function is_bound (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns boolean
//	external name 'nano!rslt_is_bound'
//	engine udr; 
//
// \brief
// is_bound (?, ?) testing convertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_is_bound)

	unsigned in_count;

	enum in : short {
		rslt = 0, column_
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
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(NANO_BLANK, is_null)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->is_nullNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column_);
					if (!isdigit(in->column_.str[0]))
						out->is_null = 
							udr_helper.fb_bool(rslt->native()->is_bound(NANODBC_TEXT(in->column_.str)));
					else
						out->is_null = 
							udr_helper.fb_bool(rslt->native()->is_bound((short)atoi(in->column_.str)));
					out->is_nullNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_ (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns smallint
//	external name 'nano!rslt_column'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_column)

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
		out->indexNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column);
					out->index = rslt->native()->column(NANODBC_TEXT(in->column.str));
					out->indexNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_name (
//	 rslt ty$pointer not null, 
//	 index smallint not null 
//	) returns varchar(63) character set utf8
//	external name 'nano!rslt_column_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_column_name)

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
		out->columnNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					FB_STRING(out->column, rslt->native()->column_name(NANODBC_TEXT(in->index)));
					out->columnNull = FB_FALSE;
					UTF8_OUT(column);
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_size (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt_column_size'
//	engine udr; 
//
// \brief
// column_size (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_column_size)

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
		out->sizeNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column);
					if (!isdigit(in->column.str[0]))
						out->size = 
							rslt->native()->column_size(NANODBC_TEXT(in->column.str));
					else
						out->size = 
							rslt->native()->column_size((short)atoi(in->column.str));
					out->sizeNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_decimal_digits (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt_column_decimal_digits'
//	engine udr; 
//
// \brief
// column_decimal_digits (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_column_decimal_digits)

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
		out->digitsNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column);
					if (!isdigit(in->column.str[0]))
						out->digits = 
							rslt->native()->column_decimal_digits(NANODBC_TEXT(in->column.str));
					else
						out->digits = 
							rslt->native()->column_decimal_digits((short)atoi(in->column.str));
					out->digitsNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt_column_datatype'
//	engine udr; 
//
// \brief
// column_datatype (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_column_datatype)

	unsigned in_count;

	enum in : short {
		rslt = 0, column_
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
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_INTEGER, datatype)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->datatypeNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column_);
					if (!isdigit(in->column_.str[0]))
						out->datatype = 
							rslt->native()->column_datatype(NANODBC_TEXT(in->column_.str));
					else
						out->datatype = 
							rslt->native()->column_datatype((short)atoi(in->column_.str));
					out->datatypeNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype_name (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns varchar(63) character set utf8
//	external name 'nano!rslt_column_datatype_name'
//	engine udr; 
//
// \brief
// column_datatype_name (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_column_datatype_name)

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
		if (!in->rsltNull)
		{
			out->datatype_nameNull = FB_TRUE;
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column);
					nanodbc::string datatype_name;
					if (!isdigit(in->column.str[0]))
						datatype_name = 
							rslt->native()->column_datatype_name(NANODBC_TEXT(in->column.str));
					else
						datatype_name = 
							rslt->native()->column_datatype_name((short)atoi(in->column.str));
					FB_STRING(out->datatype_name, datatype_name);
					out->datatype_nameNull = FB_FALSE;
					UTF8_OUT(datatype_name);
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_c_datatype (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt_column_c_datatype'
//	engine udr; 
//
// \brief
// column_c_datatype (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_column_c_datatype)

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
		out->c_datatypeNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					UTF8_IN(column);
					if (!isdigit(in->column.str[0]))
						out->c_datatype = 
							rslt->native()->column_c_datatype(NANODBC_TEXT(in->column.str));
					else
						out->c_datatype = 
							rslt->native()->column_c_datatype((short)atoi(in->column.str));
					out->c_datatypeNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_result (
//	 rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt_next_result'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_next_result)

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
		out->succesNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					out->succes = udr_helper.fb_bool(rslt->native()->next_result());
					out->succesNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function exist (
//	 rslt ty$pointer not null 
//	) returns boolean
//	external name 'nano!rslt_exist'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_exist)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_BOOLEAN, exist)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		out->existNull = FB_TRUE;
		if (!in->rsltNull)
		{
			nanoudr::result* rslt = udr_helper.rslt_ptr(in->rslt.str);
			try
			{
				if (udr_resources.results.is_valid(rslt))
				{
					bool bool_ = rslt;
					out->exist = udr_helper.fb_bool(bool_);
					out->existNull = FB_FALSE;
				}
				else
					NANOUDR_THROW(INVALID_RSLT_POINTER)
			}
			catch (std::runtime_error const& e)
			{
				NANODBC_THROW(e.what())
			}
		}
		else
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr::

