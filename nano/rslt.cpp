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
 
using namespace Firebird;

//-----------------------------------------------------------------------------
// package nano$rslt
//

using namespace nanodbc;

namespace nano
{

//-----------------------------------------------------------------------------
// create function dispose (
//	 tnx ty$pointer not null, 
// ) returns ty$pointer
// external name 'nano!rslt_dispose'
// engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_dispose)

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
		if (in->rsltNull == FB_FALSE)
		{
			try
			{
				delete nano::rslt_ptr(in->rslt.str);
				out->rsltNull = FB_TRUE;
			}
			catch (...)
			{
				nano::fb_ptr(out->rslt.str, nano::native_ptr(in->rslt.str));
				out->rsltNull = FB_FALSE;
				throw;
			}
		}
		else
		{
			 out->rsltNull = FB_TRUE;
			 throw tnx_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rowset_size  ( 
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->size = rslt->rowset_size();
				out->sizeNull = FB_FALSE;
			}
			catch (...)
			{
				out->sizeNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			out->sizeNull = FB_TRUE;
			throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function affected_rows (
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->affected = rslt->affected_rows();
				out->affectedNull = FB_FALSE;
			}
			catch (...)
			{
				out->affectedNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->affectedNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function has_affected_rows (
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->has_affected = nano::fb_bool(rslt->has_affected_rows());
				out->has_affectedNull = FB_FALSE;
			}
			catch (...)
			{
				out->has_affectedNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->has_affectedNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function rows (
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->rows = rslt->rows();
				out->rowsNull = FB_FALSE;
			}
			catch (...)
			{
				out->rowsNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->rowsNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->columns = rslt->columns();
				out->columnsNull = FB_FALSE;
			}
			catch (...)
			{
				out->columnsNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->columnsNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->first());
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->last());
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->next());
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->prior());
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function move_ (
//	 rslt ty$pointer not null, 
//	 row integer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->move(in->row));
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function skip_ (
//	 rslt ty$pointer not null, 
//	 row integer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->skip(in->rows));
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function position_ (
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->position = rslt->position();
				out->positionNull = FB_FALSE;
			}
			catch (...)
			{
				out->positionNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->positionNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->at_end = nano::fb_bool(rslt->at_end());
				out->at_endNull = FB_FALSE;
			}
			catch (...)
			{
				out->at_endNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->at_endNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function unbind (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 default null 
//	) returns ty$nano_blank
//	external name 'nano!rslt_unbind'
//	engine udr; 
//
// \brief
// unbind (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_unbind)

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
		if (in->rsltNull == FB_FALSE)
		{
			out->blank = BLANK;
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (in->columnNull == FB_FALSE)
				{
					if (!isdigit(in->column.str[0]))
						rslt->unbind(NANODBC_TEXT(in->column.str));
					else
						rslt->unbind((short)atoi(in->column.str));
				}
				else
					rslt->unbind();
				out->blankNull = FB_FALSE;
			}
			catch (...)
			{
				out->blankNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->blankNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// todo: template <class T> T get

//-----------------------------------------------------------------------------
// create function is_null (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 default null 
//	) returns boolean
//	external name 'nano!rslt_is_null'
//	engine udr; 
//
// \brief
// is_null (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_is_null)

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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (!isdigit(in->column.str[0]))
					out->is_null = nano::fb_bool(rslt->is_null(NANODBC_TEXT(in->column.str)));
				else
					out->is_null = nano::fb_bool(rslt->is_null((short)atoi(in->column.str)));
				out->is_nullNull = FB_FALSE;
			}
			catch (...)
			{
				out->is_nullNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->is_nullNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
// is_bound (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_is_bound)

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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (!isdigit(in->column.str[0]))
					out->is_null = nano::fb_bool(rslt->is_bound(NANODBC_TEXT(in->column.str)));
				else
					out->is_null = nano::fb_bool(rslt->is_bound((short)atoi(in->column.str)));
				out->is_nullNull = FB_FALSE;
			}
			catch (...)
			{
				out->is_nullNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->is_nullNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_ (
//	 rslt ty$pointer not null, 
//	 column_name varchar(63) character set utf8 not null 
//	) returns smallint
//	external name 'nano!rslt_column'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_column)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_VARCHAR(63 * 4), column_name)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_SMALLINT, column)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->column = rslt->column(NANODBC_TEXT(in->column_name.str));
				out->columnNull = FB_FALSE;
			}
			catch (...)
			{
				out->columnNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->columnNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_name (
//	 rslt ty$pointer not null, 
//	 column_name smallint not null 
//	) returns varchar(63) character set utf8
//	external name 'nano!rslt_column_name'
//	engine udr; 
//

FB_UDR_BEGIN_FUNCTION(rslt_column_name)

	FB_UDR_MESSAGE(
		InMessage,
		(NANO_POINTER, rslt)
		(FB_SMALLINT, column)
	);

	FB_UDR_MESSAGE(
		OutMessage,
		(FB_VARCHAR(63 * 4), column_name)
	);

	FB_UDR_EXECUTE_FUNCTION
	{
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				string column_name = rslt->column_name(NANODBC_TEXT(in->column));
				out->column_name.length =
					(ISC_USHORT)column_name.length() < (ISC_USHORT)sizeof(out->column_name.str) ?
						(ISC_USHORT)column_name.length() : (ISC_USHORT)sizeof(out->column_name.str);
				memcpy(out->column_name.str, column_name.c_str(), out->column_name.length);
				out->column_nameNull = FB_FALSE;
			}
			catch (...)
			{
				out->column_nameNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->column_nameNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (!isdigit(in->column.str[0]))
					out->size = rslt->column_size(NANODBC_TEXT(in->column.str));
				else
					out->size = rslt->column_size((short)atoi(in->column.str));
				out->sizeNull = FB_FALSE;
			}
			catch (...)
			{
				out->sizeNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->sizeNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (!isdigit(in->column.str[0]))
					out->digits = rslt->column_decimal_digits(NANODBC_TEXT(in->column.str));
				else
					out->digits = rslt->column_decimal_digits((short)atoi(in->column.str));
				out->digitsNull = FB_FALSE;
			}
			catch (...)
			{
				out->digitsNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->digitsNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (!isdigit(in->column.str[0]))
					out->datatype = rslt->column_datatype(NANODBC_TEXT(in->column.str));
				else
					out->datatype = rslt->column_datatype((short)atoi(in->column.str));
				out->datatypeNull = FB_FALSE;
			}
			catch (...)
			{
				out->datatypeNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->datatypeNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function column_datatype_name (
//	 rslt ty$pointer not null, 
//	 column_ varchar(63) character set utf8 not null 
//	) returns integer
//	external name 'nano!rslt_column_datatype_name'
//	engine udr; 
//
// \brief
// column_datatype_name (?, ?) testing covertion the character string into a integer and call associate method
//

FB_UDR_BEGIN_FUNCTION(rslt_column_datatype_name)

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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				string datatype_name;
				if (!isdigit(in->column.str[0]))
					datatype_name = rslt->column_datatype_name(NANODBC_TEXT(in->column.str));
				else
					datatype_name = rslt->column_datatype_name((short)atoi(in->column.str));
				out->datatype_name.length =
					(ISC_USHORT)datatype_name.length() < (ISC_USHORT)sizeof(out->datatype_name.str) ?
						(ISC_USHORT)datatype_name.length() : (ISC_USHORT)sizeof(out->datatype_name.str);
				memcpy(out->datatype_name.str, datatype_name.c_str(), out->datatype_name.length);
				out->datatype_nameNull = FB_FALSE;
			}
			catch (...)
			{
				out->datatype_nameNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->datatype_nameNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				if (!isdigit(in->column.str[0]))
					out->c_datatype = rslt->column_c_datatype(NANODBC_TEXT(in->column.str));
				else
					out->c_datatype = rslt->column_c_datatype((short)atoi(in->column.str));
				out->c_datatypeNull = FB_FALSE;
			}
			catch (...)
			{
				out->c_datatypeNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->c_datatypeNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function next_result (
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				out->succes = nano::fb_bool(rslt->next_result());
				out->succesNull = FB_FALSE;
			}
			catch (...)
			{
				out->succesNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->succesNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

//-----------------------------------------------------------------------------
// create function exist (
//	 rslt ty$pointer not null, 
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
		if (in->rsltNull == FB_FALSE)
		{
			nanodbc::result* rslt = nano::rslt_ptr(in->rslt.str);
			try
			{
				bool bool_ = rslt;
				out->exist = nano::fb_bool(bool_);
				out->existNull = FB_FALSE;
			}
			catch (...)
			{
				out->existNull = FB_TRUE;
				throw;
			}
		}
		else
		{
			 out->existNull = FB_TRUE;
			 throw rslt_POINTER_INVALID;
		}
	}

FB_UDR_END_FUNCTION

} // namespace nano

