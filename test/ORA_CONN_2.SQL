CREATE OR ALTER PROCEDURE ORA_CONN_2
RETURNS (
    STATUS VARCHAR(256) CHARACTER SET UTF8)
AS
DECLARE conn TY$POINTER;
  DECLARE stmt TY$POINTER;
  DECLARE rslt TY$POINTER;
  DECLARE tnx TY$POINTER;
  DECLARE ora_conn VARCHAR(512) =
    'DRIVER={OracleODBC-11g};DBQ=172.19.194.123:1521/asts11;UID=se;PWD=se;';
BEGIN
  nano$udr.initialize();
  nano$udr.locale('windows-1251');
  BEGIN
    conn = nano$conn.connection(ora_conn);
    nano$conn.isolation_level(conn, nano$conn.txn_read_committed());
    tnx = nano$tnx.transaction_(conn);
    stmt = nano$stmt.statement_(conn, 'SELECT ''12345'' AS s0 FROM dual;');
    rslt = nano$stmt.execute_(stmt);
    IF (nano$rslt.next_(rslt)) THEN
    BEGIN
      status = 'Result by index = ' || nano$rslt.get_integer(rslt, 0);
      SUSPEND;
      status = 'Result by name = ' || nano$rslt.get_integer(rslt, 'S0');
      SUSPEND;
      status = 'Result by ref = ' || nano$rslt.get_integer(rslt, nano$rslt.column_name(rslt, 0));
      SUSPEND;
    END
    nano$stmt.close_(stmt);
    nano$tnx.commit_(tnx);
    /* note - nano$conn.release_(conn) will check and free all resources a connection
    rslt = nano$rslt.release_(rslt);
    stmt = nano$stmt.release_(stmt);
    tnx = nano$tnx.release_(tnx);
    */
    WHEN EXCEPTION nano$nanodbc_error, EXCEPTION nano$fetching_error DO
    BEGIN
      IF (nano$tnx.valid(tnx)) THEN nano$tnx.rollback_(tnx);
      /* note - nano$conn.release_(conn) will check and free all resources a connection
      IF (nano$rslt.valid(rslt)) THEN rslt = nano$rslt.release_(rslt);
      IF (nano$stmt.valid(stmt)) THEN stmt = nano$stmt.release_(stmt);
      IF (nano$tnx.valid(tnx)) THEN tnx = nano$tnx.release_(tnx);
      */
      status = nano$udr.error_message();
      SUSPEND;
    END
  END
  IF (nano$conn.valid(conn)) THEN nano$conn.release_(conn);
  nano$udr.finalize();
END