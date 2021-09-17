# udr-nanodbc

Firebird user defined routine based on a small C++ wrapper for the native C ODBC API https://github.com/nanodbc/nanodbc

Idea:

```sql
EXECUTE BLOCK (
  ora_conn_str VARCHAR(512) CHARACTER SET UTF8 = :ocs
) RETURNS (
  e VARCHAR(1024) CHARACTER SET UTF8
)
AS
  DECLARE conn ty$pointer;
  DECLARE stmt ty$pointer;
  DECLARE tnx ty$pointer;
BEGIN
  nano$udr.make_ready();
  BEGIN
    conn = nano$conn.connection(ora_conn_str);
    WHEN EXCEPTION NANO$NANODBC_ERR_MESSAGE do
    BEGIN
      e = 'ORA connect failed: ' || nano$udr.error_message();
      SUSPEND;
      EXIT;
    END
  END
  /* with transaction
    tnx = nano$tnx.transaction_(conn);
  */
  stmt = nano$stmt.statement_(conn);
  nano$stmt.prepare_(stmt, 'INSERT INTO myTABLE (myFIELD_1, myFIELD_2, myFIELD_3... ) VALUES (?, ?, ?...);');
  nano$stmt.bind_integer(stmt, 0, 1000);
  nano$stmt.bind_string(stmt, 1, '1000');
  nano$stmt.bind_timestamp(stmt, 2, 'now');
  /*
    other bind ...
  */
  nano$stmt.just_execute_(stmt);
  /*  with transaction
    nano$tnx.commit_(tnx);
  */
  nano$conn.release_(stmt); /* not necessarily */
  /*  with transaction
    nano$tnx.release_(tnx);
  */
  nano$conn.release_(conn);

  WHEN EXCEPTION NANO$INVALID_CONN_POINTER, EXCEPTION NANO$INVALID_STMT_POINTER,
       EXCEPTION NANO$NANODBC_ERR_MESSAGE
  DO
  BEGIN
    e = 'Execute block failed: ' || nano$udr.error_message();
    nano$conn.release_(stmt); /* not necessarily */
    /* with transaction, auto rollback
      nano$tnx.release_(tnx);
    */
    nano$conn.release_(conn);
    SUSPEND;
    EXIT;
  END
END
```

