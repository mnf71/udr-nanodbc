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
// create function release_ (
//	 tnx ty$pointer not null 
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
		if (!in->rsltNull)
		{
			try
			{
				delete nanoudr::rslt_ptr(in->rslt.str);
				out->rsltNull = FB_TRUE;
			}
			catch (std::runtime_error const& e)
			{
				nanoudr::fb_ptr(out->rslt.str, nanoudr::native_ptr(in->rslt.str));
				out->rsltNull = FB_FALSE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->rsltNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->size = rslt->rowset_size();
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->sizeNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			out->sizeNull = FB_TRUE;
			NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->affected = rslt->affected_rows();
				out->affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->affectedNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->affectedNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->has_affected = nanoudr::fb_bool(rslt->has_affected_rows());
				out->has_affectedNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->has_affectedNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->has_affectedNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->rows = rslt->rows();
				out->rowsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->rowsNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->rowsNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->columns = rslt->columns();
				out->columnsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->columnsNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->columnsNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->first());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->last());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->next());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->prior());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->move(in->row));
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->skip(in->rows));
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->position = rslt->position();
				out->positionNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->positionNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->positionNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->at_end = nanoudr::fb_bool(rslt->at_end());
				out->at_endNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->at_endNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->at_endNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			out->blank = BLANK;
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!in->column_Null)
				{
					if (!isdigit(in->column_.str[0]))
						rslt->unbind(NANODBC_TEXT(in->column_.str));
					else
						rslt->unbind((short)atoi(in->column_.str));
				}
				else
					rslt->unbind();
				out->blankNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->blankNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->blankNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!isdigit(in->column_.str[0]))
					out->is_null = nanoudr::fb_bool(rslt->is_null(NANODBC_TEXT(in->column_.str)));
				else
					out->is_null = nanoudr::fb_bool(rslt->is_null((short)atoi(in->column_.str)));
				out->is_nullNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->is_nullNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->is_nullNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!isdigit(in->column_.str[0]))
					out->is_null = nanoudr::fb_bool(rslt->is_bound(NANODBC_TEXT(in->column_.str)));
				else
					out->is_null = nanoudr::fb_bool(rslt->is_bound((short)atoi(in->column_.str)));
				out->is_nullNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->is_nullNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->is_nullNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		(FB_SMALLINT, index)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				out->index = rslt->column(NANODBC_TEXT(in->column_.str));
				out->indexNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->indexNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->indexNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		column_ = 0
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
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				FB_STRING(out->column_, rslt->column_name(NANODBC_TEXT(in->index)));
				out->column_Null = FB_FALSE;
				UTF8_OUT(column_);
			}
			catch (std::runtime_error const& e)
			{
				out->column_Null = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->column_Null = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		(FB_INTEGER, size)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!isdigit(in->column_.str[0]))
					out->size = rslt->column_size(NANODBC_TEXT(in->column_.str));
				else
					out->size = rslt->column_size((short)atoi(in->column_.str));
				out->sizeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->sizeNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->sizeNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		(FB_INTEGER, digits)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!isdigit(in->column_.str[0]))
					out->digits = rslt->column_decimal_digits(NANODBC_TEXT(in->column_.str));
				else
					out->digits = rslt->column_decimal_digits((short)atoi(in->column_.str));
				out->digitsNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->digitsNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->digitsNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!isdigit(in->column_.str[0]))
					out->datatype = rslt->column_datatype(NANODBC_TEXT(in->column_.str));
				else
					out->datatype = rslt->column_datatype((short)atoi(in->column_.str));
				out->datatypeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->datatypeNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->datatypeNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		rslt = 0, column_
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
		(FB_VARCHAR(63 * 4), column_)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(128 * 4), datatype_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				nanodbc::string datatype_name;
				if (!isdigit(in->column_.str[0]))
					datatype_name = rslt->column_datatype_name(NANODBC_TEXT(in->column_.str));
				else
					datatype_name = rslt->column_datatype_name((short)atoi(in->column_.str));
				FB_STRING(out->datatype_name, datatype_name);
				out->datatype_nameNull = FB_FALSE;
				UTF8_OUT(datatype_name);
			}
			catch (std::runtime_error const& e)
			{
				out->datatype_nameNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->datatype_nameNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		(FB_INTEGER, c_datatype)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				UTF8_IN(column_);
				if (!isdigit(in->column_.str[0]))
					out->c_datatype = rslt->column_c_datatype(NANODBC_TEXT(in->column_.str));
				else
					out->c_datatype = rslt->column_c_datatype((short)atoi(in->column_.str));
				out->c_datatypeNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->c_datatypeNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->c_datatypeNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nanoudr::fb_bool(rslt->next_result());
				out->succesNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->succesNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
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
		if (!in->rsltNull)
		{
			nanodbc::result* rslt = nanoudr::rslt_ptr(in->rslt.str);
			try
			{
				bool bool_ = rslt;
				out->exist = nanoudr::fb_bool(bool_);
				out->existNull = FB_FALSE;
			}
			catch (std::runtime_error const& e)
			{
				out->existNull = FB_TRUE;
				RANDOM_THROW(e.what())
			}
		}
		else
		{
			 out->existNull = FB_TRUE;
			 NANOUDR_THROW(INVALID_RSLT_POINTER)
		}
	}

FB_UDR_END_FUNCTION

} // namespace nanoudr::

