RECREATE PACKAGE BODY NANO$RSLT
AS
BEGIN

  FUNCTION dispose(tnx TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!rslt_dispose'
    ENGINE UDR;

  FUNCTION rowset_size(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_rowset_size'
    ENGINE UDR;

  FUNCTION affected_rows(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_affected_rows'
    ENGINE UDR;

  FUNCTION has_affected_rows(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_has_affected_rows'
    ENGINE UDR;

  FUNCTION rows_(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_rows'
    ENGINE UDR;

  FUNCTION columns(rslt TY$POINTER NOT NULL) RETURNS SMALLINT
    EXTERNAL NAME 'nano!rslt_columns'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION first_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_first'
    ENGINE UDR;

  FUNCTION last_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_last'
    ENGINE UDR;

  FUNCTION next_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_next'
    ENGINE UDR;

  FUNCTION prior_(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_prior'
    ENGINE UDR;

  FUNCTION move_(rslt TY$POINTER NOT NULL, row_ INTEGER NOT NULL) RETURNS BOOLEAN
     EXTERNAL NAME 'nano!rslt_move'
     ENGINE UDR;

  FUNCTION skip_(rslt TY$POINTER NOT NULL, row_ INTEGER NOT NULL) RETURNS BOOLEAN
     EXTERNAL NAME 'nano!rslt_skip'
     ENGINE UDR;

  FUNCTION position_(rslt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_position'
    ENGINE UDR;

  FUNCTION at_end(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_at_end'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION unbind(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!rslt_unbind'
    ENGINE UDR;

  FUNCTION is_null(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_is_null'
    ENGINE UDR;

  FUNCTION is_bound(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_is_bound'
    ENGINE UDR;

  FUNCTION column_(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS SMALLINT
    EXTERNAL NAME 'nano!rslt_column'
    ENGINE UDR;

  FUNCTION column_name(rslt TY$POINTER NOT NULL, index_ SMALLINT NOT NULL)
    RETURNS VARCHAR(63) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt_column_name'
    ENGINE UDR;

  FUNCTION column_size(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_column_size'
    ENGINE UDR;

  FUNCTION column_decimal_digits(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_column_decimal_digits'
    ENGINE UDR;

  FUNCTION column_datatype(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_column_datatype'
    ENGINE UDR;

  FUNCTION column_datatype_name(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS VARCHAR(63) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!rslt_column_datatype_name'
    ENGINE UDR;

  FUNCTION column_c_datatype(rslt TY$POINTER NOT NULL, column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL)
    RETURNS INTEGER
    EXTERNAL NAME 'nano!rslt_column_c_datatype'
    ENGINE UDR;

  FUNCTION next_result(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_next_result'
    ENGINE UDR;

  FUNCTION exist(rslt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!rslt_exist'
    ENGINE UDR;

END