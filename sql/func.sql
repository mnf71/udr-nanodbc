SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$FUNC
AS
BEGIN
  
  /*  Note:
        Result cursor by default ODBC driver (NANODBC implementation),
        scrollable into NANO$STMT
   */

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

END^

RECREATE PACKAGE BODY NANO$FUNC
AS
BEGIN

  FUNCTION execute_conn(
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!func$execute_conn'
    ENGINE UDR;

  FUNCTION just_execute_conn(
      conn TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      batch_operations INTEGER NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func$just_execute_conn'
    ENGINE UDR;

  FUNCTION execute_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!func$execute_stmt'
    ENGINE UDR;

  FUNCTION just_execute_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func$just_execute_stmt'
    ENGINE UDR;

  FUNCTION transact_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!func$transact_stmt'
    ENGINE UDR;

  FUNCTION just_transact_stmt(
      stmt TY$POINTER NOT NULL,
      batch_operations INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func$just_transact_stmt'
    ENGINE UDR;

  FUNCTION prepare_stmt(
      stmt TY$POINTER NOT NULL,
      query VARCHAR(8191) CHARACTER SET UTF8 NOT NULL,
      timeout INTEGER NOT NULL
    ) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!func$prepare_stmt'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$FUNC TO SYSDBA;
