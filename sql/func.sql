SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$FUNC
AS
BEGIN

  FUNCTION set_locale(
    udr_locale VARCHAR(20) CHARACTER SET NONE NOT NULL
  ) RETURNS TY$NANO_BLANK;

  ------------------------------------------------------------------------------

  FUNCTION execute_conn(
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL DEFAULT 1,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$POINTER;

  FUNCTION just_execute_conn(
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL DEFAULT 1,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  FUNCTION execute_stmt(
      stmt TY$POINTER NOT NULL, batch_operations INTEGER NOT NULL DEFAULT 1
    ) RETURNS TY$POINTER;

  FUNCTION just_execute_stmt(
      stmt TY$POINTER NOT NULL, batch_operations INTEGER NOT NULL DEFAULT 1
    ) RETURNS TY$NANO_BLANK;

  FUNCTION transact_stmt(
      stmt TY$POINTER NOT NULL, batch_operations INTEGER NOT NULL DEFAULT 1
    ) RETURNS TY$POINTER;

  FUNCTION just_transact_stmt(
      stmt TY$POINTER NOT NULL, batch_operations INTEGER NOT NULL DEFAULT 1
    ) RETURNS TY$NANO_BLANK;

  FUNCTION prepare_stmt(
      stmt TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL DEFAULT 0
    ) RETURNS TY$NANO_BLANK;

  ------------------------------------------------------------------------------

  FUNCTION e_message RETURNS VARCHAR(512) CHARACTER SET UTF8;

END^

RECREATE PACKAGE BODY NANO$FUNC
AS
BEGIN

  FUNCTION set_locale(
    udr_locale VARCHAR(20) CHARACTER SET NONE NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!set_locale'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION execute_conn(
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!func_execute_conn'
    ENGINE UDR;

  FUNCTION just_execute_conn(
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func_just_execute_conn'
    ENGINE UDR;

  FUNCTION execute_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!func_execute_stmt'
    ENGINE UDR;

  FUNCTION just_execute_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func_just_execute_stmt'
    ENGINE UDR;

  FUNCTION transact_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!func_transact_stmt'
    ENGINE UDR;

  FUNCTION just_transact_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func_just_transact_stmt'
    ENGINE UDR;

  FUNCTION prepare_stmt(
      stmt TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func_prepare_stmt'
    ENGINE UDR;

  ------------------------------------------------------------------------------

  FUNCTION e_message RETURNS VARCHAR(512) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!e_message'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$FUNC TO SYSDBA;
