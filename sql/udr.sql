CREATE OR ALTER PACKAGE NANO$UDR
AS
BEGIN

  FUNCTION set_locale(
    udr_locale VARCHAR(20) CHARACTER SET NONE NOT NULL
  ) RETURNS TY$NANO_BLANK;

  FUNCTION err_msg RETURNS VARCHAR(512) CHARACTER SET UTF8;

END