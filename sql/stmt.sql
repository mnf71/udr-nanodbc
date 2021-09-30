SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$STMT
AS
BEGIN

  FUNCTION statement_(
      conn TY$POINTER DEFAULT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 DEFAULT NULL,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$POINTER;

  FUNCTION valid(stmt TY$POINTER NOT NULL) RETURNS BOOLEAN;

  FUNCTION release_(stmt TY$POINTER NOT NULL) RETURNS TY$POINTER;

  FUNCTION connected(stmt TY$POINTER NOT NULL) RETURNS BOOLEAN;
  FUNCTION connection(stmt TY$POINTER NOT NULL) RETURNS TY$POINTER;

  FUNCTION open_(
      stmt TY$POINTER NOT NULL, conn TY$POINTER NOT NULL
    ) RETURNS TY$NANO_BLANK;

  FUNCTION is_open(stmt TY$POINTER NOT NULL) RETURNS BOOLEAN;

  FUNCTION close_(stmt TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK;

  FUNCTION cancel(stmt TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK;

  FUNCTION prepare_direct(
      stmt TY$POINTER NOT NULL,
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION prepare_(
      stmt TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION timeout(
      stmt TY$POINTER NOT NULL,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION execute_direct(
      stmt TY$POINTER NOT NULL,
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL DEFAULT 1,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$POINTER;

  FUNCTION just_execute_direct(
      stmt TY$POINTER NOT NULL,
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL DEFAULT 1,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION execute_(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL DEFAULT 1,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$POINTER;

  FUNCTION just_execute(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL DEFAULT 1,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION procedure_columns(
      stmt TY$POINTER NOT NULL,
      catalog_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL,
      schema_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL,
      procedure_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL,
      column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TY$POINTER;

  FUNCTION affected_rows(stmt TY$POINTER NOT NULL) RETURNS INTEGER;
  FUNCTION columns(stmt TY$POINTER NOT NULL) RETURNS SMALLINT;
  FUNCTION reset_parameters(stmt TY$POINTER NOT NULL, timeout INTEGER NOT NULL DEFAULT 0)
    RETURNS TY$NANO_BLANK;
  FUNCTION parameters(stmt TY$POINTER NOT NULL) RETURNS SMALLINT;
  FUNCTION parameter_size(stmt TY$POINTER NOT NULL, param_index SMALLINT NOT NULL)
    RETURNS INTEGER;

  ------------------------------------------------------------------------------

  FUNCTION bind_smallint(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ SMALLINT
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_integer(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ INTEGER
    ) RETURNS TY$NANO_BLANK;

/*
  FUNCTION bind_bigint(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ BIGINT
    ) RETURNS TY$NANO_BLANK;
*/

  FUNCTION bind_float(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ FLOAT
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_double(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ DOUBLE PRECISION
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_varchar(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ VARCHAR(32765) CHARACTER SET NONE,
      param_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_char(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ CHAR(32767) CHARACTER SET NONE,
      param_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_u8_varchar(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ VARCHAR(8191) CHARACTER SET UTF8,
      param_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_u8_char(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ CHAR(8191) CHARACTER SET UTF8,
      param_size SMALLINT NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_blob(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ BLOB CHARACTER SET NONE
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_boolean(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ BOOLEAN
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_date(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ DATE
    ) RETURNS TY$NANO_BLANK;

/*
  FUNCTION bind_time(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ TIME
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;
*/

  FUNCTION bind_timestamp(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ TIMESTAMP
    ) RETURNS TY$NANO_BLANK;

  FUNCTION bind_null(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      batch_size INTEGER NOT NULL DEFAULT 1
    ) RETURNS TY$NANO_BLANK;

  ------------------------------------------------------------------------------

  FUNCTION describe_parameters(
      stmt TY$POINTER NOT NULL,
      idx SMALLINT NOT NULL,
      type_ SMALLINT NOT NULL,
      size_ INTEGER NOT NULL,
      scale_ SMALLINT NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;


END^

RECREATE PACKAGE BODY NANO$STMT
AS
BEGIN

  FUNCTION statement_(
      conn TY$POINTER,
      query VARCHAR(8191) CHARACTER SET UTF8,
      timeout INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!stmt_statement'
    ENGINE UDR;

  FUNCTION valid(stmt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!stmt_valid'
    ENGINE UDR;

  FUNCTION release_(stmt TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!stmt_release'
    ENGINE UDR;

  FUNCTION connected(stmt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!stmt_connected'
    ENGINE UDR;

  FUNCTION connection(stmt TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!stmt_connection'
    ENGINE UDR;

  FUNCTION open_(
      stmt TY$POINTER NOT NULL, conn TY$POINTER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_open'
    ENGINE UDR;

  FUNCTION is_open(stmt TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!stmt_is_open'
    ENGINE UDR;

  FUNCTION close_(stmt TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_close'
    ENGINE UDR;

  FUNCTION cancel(stmt TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_cancel'
    ENGINE UDR;

  FUNCTION prepare_direct(
      stmt TY$POINTER NOT NULL,
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_prepare_direct'
    ENGINE UDR;

  FUNCTION prepare_(
      stmt TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_prepare'
    ENGINE UDR;

  FUNCTION timeout(
      stmt TY$POINTER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_timeout'
    ENGINE UDR;

  FUNCTION execute_direct(
      stmt TY$POINTER NOT NULL,
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!stmt_execute_direct'
    ENGINE UDR;

  FUNCTION just_execute_direct(
      stmt TY$POINTER NOT NULL,
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_just_execute_direct'
    ENGINE UDR;

  FUNCTION execute_(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!stmt_execute'
    ENGINE UDR;

  FUNCTION just_execute(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_just_execute'
    ENGINE UDR;

  FUNCTION procedure_columns(
      stmt TY$POINTER NOT NULL,
      catalog_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL,
      schema_ VARCHAR(128) CHARACTER SET UTF8 NOT NULL,
      procedure_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL,
      column_ VARCHAR(63) CHARACTER SET UTF8 NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!stmt_procedure_columns'
    ENGINE UDR;

  FUNCTION affected_rows(stmt TY$POINTER NOT NULL) RETURNS INTEGER
    EXTERNAL NAME 'nano!stmt_affected_rows'
    ENGINE UDR;

  FUNCTION columns(stmt TY$POINTER NOT NULL) RETURNS SMALLINT
    EXTERNAL NAME 'nano!stmt_columns'
    ENGINE UDR;

  FUNCTION reset_parameters(stmt TY$POINTER NOT NULL, timeout INTEGER NOT NULL)
    RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_reset_parameters'
    ENGINE UDR;

  FUNCTION parameters(stmt TY$POINTER NOT NULL) RETURNS SMALLINT
    EXTERNAL NAME 'nano!stmt_parameters'
    ENGINE UDR;

  FUNCTION parameter_size(stmt TY$POINTER NOT NULL, param_index SMALLINT NOT NULL)
    RETURNS INTEGER
    EXTERNAL NAME 'nano!stmt_parameter_size'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION bind_smallint(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ SMALLINT
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_integer(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ INTEGER
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

/*
  FUNCTION bind_bigint(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ BIGINT
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;
*/

  FUNCTION bind_float(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ FLOAT
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_double(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ DOUBLE PRECISION
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_varchar(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ VARCHAR(32765) CHARACTER SET NONE,
      param_size SMALLINT NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_char(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ CHAR(32767) CHARACTER SET NONE,
      param_size SMALLINT NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_u8_varchar(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ VARCHAR(8191) CHARACTER SET UTF8,
      param_size SMALLINT NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_u8_char(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ CHAR(8191) CHARACTER SET UTF8,
      param_size SMALLINT NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_blob(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ BLOB CHARACTER SET NONE
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_boolean(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ BOOLEAN
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_date(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ DATE
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

/*
  FUNCTION bind_time(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ TIME
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;
*/

  FUNCTION bind_timestamp(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      value_ TIMESTAMP
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind'
    ENGINE UDR;

  FUNCTION bind_null(
      stmt TY$POINTER NOT NULL,
      param_index SMALLINT NOT NULL,
      batch_size INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_bind_null'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION describe_parameters(
      stmt TY$POINTER NOT NULL,
      idx SMALLINT NOT NULL,
      type_ SMALLINT NOT NULL,
      size_ INTEGER NOT NULL,
      scale_ SMALLINT NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!stmt_describe_parameters'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$STMT TO SYSDBA;
