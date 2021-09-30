SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$UDR
AS
BEGIN

  FUNCTION initialize RETURNS TY$NANO_BLANK;
  FUNCTION finalize RETURNS TY$NANO_BLANK;
  FUNCTION expunge RETURNS TY$NANO_BLANK;

  FUNCTION locale(
      set_locale VARCHAR(20) CHARACTER SET NONE DEFAULT NULL /* NULL - Get */
    ) RETURNS VARCHAR(20);

  FUNCTION error_message RETURNS VARCHAR(512) CHARACTER SET UTF8;

END^

RECREATE PACKAGE BODY NANO$UDR
AS
BEGIN

  FUNCTION initialize RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!initialize'
    ENGINE UDR;

  FUNCTION finalize RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!finalize'
    ENGINE UDR;

  FUNCTION expunge RETURNS TY$NANO_BLANK
    EXTERNAL NAME 'nano!expunge'
    ENGINE UDR;

  FUNCTION locale(
      set_locale VARCHAR(20) CHARACTER SET NONE
    ) RETURNS VARCHAR(20)
    EXTERNAL NAME 'nano!locale'
    ENGINE UDR;

  FUNCTION error_message RETURNS VARCHAR(512) CHARACTER SET UTF8
    EXTERNAL NAME 'nano!error_message'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$UDR TO SYSDBA;
