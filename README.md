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
  nano$udr.initialize(); -- maybe trigger on connect
  
  BEGIN
    conn = nano$conn.connection(ora_conn_str);
    WHEN EXCEPTION NANO$NANODBC_ERR_MESSAGE do
    BEGIN
      e = 'ORA connect failed: ' || nano$udr.error_message();
      SUSPEND;
      EXIT;
    END
  END
  
  BEGIN
    /* with transaction
      tnx = nano$tnx.transaction_(conn);
    */
    stmt = nano$stmt.statement_(conn);
    nano$stmt.prepare_(stmt, 'INSERT INTO myTABLE (myFIELD_1, myFIELD_2, myFIELD_3... ) VALUES (?, ?, ?...);');
    nano$stmt.bind_integer(stmt, 0, 1000);
    nano$stmt.bind_varchar(stmt, 1, '1000');
    nano$stmt.bind_timestamp(stmt, 2, 'now');
    /*
      other bind ...
    */
    nano$stmt.just_execute_(stmt); 
    /*  with transaction
      nano$tnx.commit_(tnx);
    */
    stmt = nano$stmt.release_(stmt); /* not necessarily */ -- NULL if success
    /*  with transaction
      tnx = nano$tnx.release_(tnx);
    */
    conn = nano$conn.release_(conn);
    
    nano$udr.finalize(); -- best, trigger on disconnect

    WHEN EXCEPTION nano$pointer_conn_invalid, EXCEPTION nano$pointer_stmt_invalid,
         EXCEPTION nano$nanodbc_err_message
    DO
    BEGIN
      e = 'Execute block failed: ' || nano$udr.error_message();
      stmt = nano$stmt.release_(stmt); /* not necessarily */
      /* with transaction, auto rollback
        tnx = nano$tnx.release_(tnx);
      */
      conn = nano$conn.release_(conn);
      nano$udr.finalize();
      SUSPEND;
      EXIT;
    END
  END
  
  e = 'ok!';
  SUSPEND;
END
```

