# udr-nanodbc

А new project opened... there will be something like ;)

CREATE PROCEDURE C_P
RETURNS (rc INTEGER)
AS
  DECLARE conn TYPE OF ty$pointer;
  DECLARE stmt TYPE OF ty$pointer;  
  DECLARE rslt TYPE OF ty$pointer;  
BEGIN
  conn = nano$conn.connection_('driver=SQL Server; server=localhost; database=mydb; uid=test; pwd=test1');
  stmt = nano$conn.statement_(conn, 'select * from adm' /*-- default 0*/);
  // txn = nano$txn.transaction_(conn);  
  IF (nano$conn.connected(conn)) THEN
  BEGIN
    rslt = nano$stmt.execute_(stmt);
    IF (nano$rslt.rows_(rslt) != 0) THEN
    BEGIN
      nano$rslt.next_(rslt); -- ??? fetch
      ...
      ...
      ...
    END
    nano$stmt.release(stmt);
    nano$conn.release(conn);
  END
  SUSPEND;
END
