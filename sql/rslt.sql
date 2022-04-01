SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$RSLT
AS
BEGIN

  FUNCTION valid(rslt TY$POINTER) RETURNS BOOLEAN;

  FUNCTION release_(rslt TY$POINTER) RETURNS TY$POINTER;

  FUNCTION connection(rslt TY$POINTER NOT NULL) RETURNS TY$POINTER;

  FUNCTION rowset_size(rslt TY$POINTER NOT NULL) RETURNS INTEGER;
  FUNCTION affected_rows(rslt TY$POINTER NOT NULL) RETURNS INTEGER;
  FUNCTION has_affected_rows(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;
  FUNCTION rows_(rslt TY$POINTER NOT NULL) RETURNS INTEGER;
  FUNCTION columns(rslt TY$POINTER NOT NULL) RETURNS SMALLINT;

  ------------------------------------------------------------------------------

  FUNCTION first_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;
  FUNCTION last_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;
  FUNCTION next_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;
  FUNCTION prior_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;
  FUNCTION move(rslt TY$POINTER NOT NULL, row_ INTEGER NOT NULL) RETURNS BOOLEAN;
  FUNCTION skip_(rslt TY$POINTER NOT NULL, row_ INTEGER NOT NULL) RETURNS BOOLEAN;
  FUNCTION position_(rslt TY$POINTER NOT NULL) RETURNS INTEGER;
  FUNCTION at_end(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;

  ------------------------------------------------------------------------------

  FUNCTION get_smallint(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS SMALLINT;

  FUNCTION get_integer(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER;

/*
  FUNCTION get_bigint(
      -- none rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BIGINT;
*/

  FUNCTION get_float(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS FLOAT;

  FUNCTION get_double(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS DOUBLE PRECISION;

  /*
     Optimal result size in execution time up to 1 Kb with 4 Kb and large worse.
  */

  FUNCTION get_s_small(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(256) CHARACTER SET NONE;

  FUNCTION get_string(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(1024) CHARACTER SET NONE;

  FUNCTION get_s_large(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(32765) CHARACTER SET NONE;

  FUNCTION get_u8_s_small(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(256) CHARACTER SET UTF8;

  FUNCTION get_u8_string(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(1024) CHARACTER SET UTF8;

  FUNCTION get_u8_s_large(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(8191) CHARACTER SET UTF8;

  /*
     Two template a function show as use declaration char(n) at your discretion.
     get_char_8(rslt, 0) equal cast(get_string(rslt, 0) as char(8)))
     Returned value padded spaces.
  */

  FUNCTION get_char_8 (
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS CHAR(8) CHARACTER SET NONE;

  FUNCTION get_u8_char_2(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS CHAR(2) CHARACTER SET UTF8;

  /*
     ...
  */

  FUNCTION get_blob(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BLOB SUB_TYPE BINARY;

  FUNCTION get_boolean(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BOOLEAN;

  FUNCTION get_date(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS DATE;

/*
  FUNCTION get_time(
      -- none rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TIME;
*/

  FUNCTION get_timestamp(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TIMESTAMP;

  ------------------------------------------------------------------------------

  FUNCTION pump (
      rslt TY$POINTER NOT NULL,
      /* -- none */ query VARCHAR(8191) CHARACTER SET NONE NOT NULL,
      -- utf8 query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      transaction_pack INTEGER NOT NULL DEFAULT 0 -- common transaction
    ) RETURNS INTEGER;

  ------------------------------------------------------------------------------
  -- convert_size = 0 by value_ declaration
  --

  FUNCTION convert_s_small(
      value_ VARCHAR(1024) CHARACTER SET NONE,
      from_ VARCHAR(20) CHARACTER SET NONE NOT NULL, to_ VARCHAR(20) CHARACTER SET NONE NOT NULL,
      convert_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS VARCHAR(1024) CHARACTER SET NONE;

  FUNCTION convert_string(
      value_ VARCHAR(4096) CHARACTER SET NONE,
      from_ VARCHAR(20) CHARACTER SET NONE NOT NULL, to_ VARCHAR(20) CHARACTER SET NONE NOT NULL,
      convert_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS VARCHAR(4096) CHARACTER SET NONE;

  FUNCTION convert_s_large(
      value_ VARCHAR(32765) CHARACTER SET NONE,
      from_ VARCHAR(20) CHARACTER SET NONE NOT NULL, to_ VARCHAR(20) CHARACTER SET NONE NOT NULL,
      convert_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS VARCHAR(32765) CHARACTER SET NONE;

  ------------------------------------------------------------------------------

  FUNCTION unbind(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TY$NANO_BLANK;

  FUNCTION is_null(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BOOLEAN;

  FUNCTION is_bound( -- now hiding exception out of range
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BOOLEAN;

  FUNCTION column_(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS SMALLINT;

  FUNCTION column_name(rslt TY$POINTER NOT NULL, index_ SMALLINT NOT NULL)
    /* -- none */ RETURNS VARCHAR(128) CHARACTER SET UTF8;
    -- utf8 RETURNS VARCHAR(128) CHARACTER SET UTF8;

  FUNCTION column_size(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER;

  FUNCTION column_decimal_digits(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER;

  FUNCTION column_datatype(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER;

  FUNCTION column_datatype_name(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(128) CHARACTER SET UTF8;

  FUNCTION column_c_datatype(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER;

  FUNCTION next_result(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;

  ------------------------------------------------------------------------------

  FUNCTION has_data(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN;

END^

RECREATE PACKAGE BODY NANO$RSLT
AS
BEGIN

  FUNCTION valid(rslt TY$POINTER) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$valid'
    ENGINE UDR;

  FUNCTION release_(rslt TY$POINTER) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!rslt$release'
    ENGINE UDR;

  FUNCTION connection(rslt TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!rslt$connection'
    ENGINE UDR;

  FUNCTION rowset_size(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$rowset_size'
    ENGINE UDR;

  FUNCTION affected_rows(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$affected_rows'
    ENGINE UDR;

  FUNCTION has_affected_rows(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$has_affected_rows'
    ENGINE UDR;

  FUNCTION rows_(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$rows'
    ENGINE UDR;

  FUNCTION columns(rslt TY$POINTER NOT NULL) RETURNS SMALLINT
    EXTERNAL NAME 'nano!rslt$columns'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION first_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$first'
    ENGINE UDR;

  FUNCTION last_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$last'
    ENGINE UDR;

  FUNCTION next_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$next'
    ENGINE UDR;

  FUNCTION prior_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$prior'
    ENGINE UDR;

  FUNCTION move(rslt TY$POINTER NOT NULL, row_ INTEGER NOT NULL) RETURNS BOOLEAN
     EXTERNAL NAME 'nano!rslt$move'
     ENGINE UDR;

  FUNCTION skip_(rslt TY$POINTER NOT NULL, row_ INTEGER NOT NULL) RETURNS BOOLEAN
     EXTERNAL NAME 'nano!rslt$skip'
     ENGINE UDR;

  FUNCTION position_(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$position'
    ENGINE UDR;

  FUNCTION at_end(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$at_end'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION get_smallint(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS SMALLINT
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_integer(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

/*
  FUNCTION get_bigint(
      -- none rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BIGINT
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;
*/

  FUNCTION get_float(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS FLOAT
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_double(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS DOUBLE PRECISION
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_s_small(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(256) CHARACTER SET NONE
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_string(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(1024) CHARACTER SET NONE
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_s_large(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(32765) CHARACTER SET NONE
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_u8_s_small(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(256) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_u8_string(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(1024) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_u8_s_large(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(8191) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_char_8 (
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS CHAR(8) CHARACTER SET NONE
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_u8_char_2(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS CHAR(2) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_blob(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BLOB SUB_TYPE BINARY
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_boolean(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  FUNCTION get_date(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS DATE
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

/*
  FUNCTION get_time(
      -- none rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TIME
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;
*/

  FUNCTION get_timestamp(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TIMESTAMP
    EXTERNAL NAME 'nano!rslt$get'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION pump (
      rslt TY$POINTER NOT NULL,
      /* -- none */ query VARCHAR(8191) CHARACTER SET NONE NOT NULL,
      -- utf8 query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      transaction_pack INTEGER NOT NULL
    ) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$pump'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION convert_s_small(
      value_ VARCHAR(1024) CHARACTER SET NONE,
      from_ VARCHAR(20) CHARACTER SET NONE NOT NULL, to_ VARCHAR(20) CHARACTER SET NONE NOT NULL,
      convert_size SMALLINT NOT NULL
    ) RETURNS VARCHAR(1024) CHARACTER SET NONE
    EXTERNAL NAME 'nano!udr$convert'
    ENGINE UDR;

  FUNCTION convert_string(
      value_ VARCHAR(4096) CHARACTER SET NONE,
      from_ VARCHAR(20) CHARACTER SET NONE NOT NULL, to_ VARCHAR(20) CHARACTER SET NONE NOT NULL,
      convert_size SMALLINT NOT NULL
    ) RETURNS VARCHAR(4096) CHARACTER SET NONE
    EXTERNAL NAME 'nano!udr$convert'
    ENGINE UDR;

  FUNCTION convert_s_large(
      value_ VARCHAR(32765) CHARACTER SET NONE,
      from_ VARCHAR(20) CHARACTER SET NONE NOT NULL, to_ VARCHAR(20) CHARACTER SET NONE NOT NULL,
      convert_size SMALLINT NOT NULL
    ) RETURNS VARCHAR(32765) CHARACTER SET NONE
    EXTERNAL NAME 'nano!udr$convert'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION unbind(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!rslt$unbind'
    ENGINE UDR;

  FUNCTION is_null(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$is_null'
    ENGINE UDR;

  FUNCTION is_bound(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$is_bound'
    ENGINE UDR;

  FUNCTION column_(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS SMALLINT
    EXTERNAL NAME 'nano!rslt$column'
    ENGINE UDR;

  FUNCTION column_name(rslt TY$POINTER NOT NULL, index_ SMALLINT NOT NULL)
    /* -- none */ RETURNS VARCHAR(128) CHARACTER SET UTF8
    -- utf8 RETURNS VARCHAR(128) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt$column_name'
    ENGINE UDR;

  FUNCTION column_size(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$column_size'
    ENGINE UDR;

  FUNCTION column_decimal_digits(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$column_decimal_digits'
    ENGINE UDR;

  FUNCTION column_datatype(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$column_datatype'
    ENGINE UDR;

  FUNCTION column_datatype_name(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS VARCHAR(128) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt$column_datatype_name'
    ENGINE UDR;

  FUNCTION column_c_datatype(
      /* -- none */ rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET NONE NOT NULL
      -- utf8 rslt TY$POINTER NOT NULL, column_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL
    ) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt$column_c_datatype'
    ENGINE UDR;

  FUNCTION next_result(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$next_result'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION has_data(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt$has_data'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$RSLT TO SYSDBA;