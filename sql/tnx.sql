SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$TNX
AS
BEGIN
 
  FUNCTION transaction_(conn TY$POINTER NOT NULL) RETURNS TY$POINTER;

  FUNCTION release_(tnx ty$pointer NOT NULL) RETURNS TY$POINTER;

  FUNCTION is_valid(tnx TY$POINTER NOT NULL) RETURNS BOOLEAN;

  FUNCTION connection(tnx TY$POINTER NOT NULL) RETURNS TY$POINTER;

  FUNCTION commit_(tnx TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK;
  FUNCTION rollback_(tnx TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK;

END^

RECREATE PACKAGE BODY NANO$TNX
AS
BEGIN

  FUNCTION transaction_(conn TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!tnx_transaction'
    ENGINE UDR;

  FUNCTION release_(tnx ty$pointer NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!tnx_release'
    ENGINE UDR;

  FUNCTION is_valid(tnx TY$POINTER NOT NULL) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!tnx_is_valid'
    ENGINE UDR;

  FUNCTION connection(tnx TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!tnx_connection'
    ENGINE UDR;

  FUNCTION commit_(tnx TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!tnx_commit'
    ENGINE UDR;

  FUNCTION rollback_(tnx TY$POINTER NOT NULL) RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!tnx_rollback'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$TNX TO SYSDBA;
