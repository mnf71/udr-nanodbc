CREATE OR ALTER PROCEDURE PERF_PUMP_2
RETURNS (
    STATUS VARCHAR(256) CHARACTER SET UTF8)
AS
DECLARE conn TY$POINTER;
  DECLARE stmt TY$POINTER;
  DECLARE rslt TY$POINTER;
  DECLARE sql VARCHAR(4096) CHARACTER SET UTF8 DEFAULT '
SELECT -- FIRST 1
       i1, i2, i3, i4, i5,
       s1, s2, s3, s4, s5,
       c1, c2, c3, c4, c5,
       f1, f2, f3, f4, f5,
       d1, d2, d3, d4, d5,
       b1
  FROM perf_src
';
  DECLARE i INTEGER;
BEGIN
  nano$udr.initialize();

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' started...';
  SUSPEND;

  conn = nano$conn.connection(
      'Driver={Firebird/Interbase(r) driver};' ||
      'DataSource=localhost;Port=3050;Database=C:\Db\.fdb\LOG$.FDB;Dialect=1;' ||
      'User=SYSDBA;Password=masterkey;Role=;Charset=UTF8;' /* windows-1251 */
    );
  nano$conn.isolation_level(conn, nano$conn.txn_read_committed());
  nano$udr.locale('UTF-8'); /* windows-1251 */

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' open cursor...';
  SUSPEND;

  stmt = nano$stmt.statement_(conn, sql, FALSE, 0);
  rslt = nano$stmt.execute_(stmt);

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' pumping cursor...';
  SUSPEND;

  IF (nano$rslt.has_data(rslt)) THEN
  BEGIN
    i = nano$rslt.pump(
      rslt,
'INSERT INTO perf_dest
        (i1, i2, i3, i4, i5,
         s1, s2, s3, s4, s5,
         c1, c2, c3, c4, c5,
         f1, f2, f3, f4, f5,
         d1, d2, d3, d4, d5,
         b1)
VALUES (?, ?, ?, ?, ?,
        ?, ?, ?, ?, ?,
        ?, ?, ?, ?, ?,
        ?, ?, ?, ?, ?,
        ?, ?, ?, ?, ?,
        ?)',
      10e3);
    status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' pumped ' || i || ' records...';
    SUSPEND;
  END

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' finished...';
  SUSPEND;

  nano$conn.release_(conn);
  nano$udr.finalize();
END