CREATE OR ALTER PROCEDURE PERF_PUMP
RETURNS (
    STATUS VARCHAR(256) CHARACTER SET UTF8)
AS
DECLARE conn TY$POINTER;
  DECLARE stmt TY$POINTER;
  DECLARE rslt TY$POINTER;
  DECLARE sql VARCHAR(4096) CHARACTER SET UTF8 DEFAULT '
SELECT -- FIRST
       i1, i2, i3, i4, i5,
       s1, s2, s3, s4, s5,
       c1, c2, c3, c4, c5,
       f1, f2, f3, f4, f5,
       d1, d2, d3, d4, d5,
       b1
  FROM perf_src
';
  DECLARE eof BOOLEAN;
  DECLARE tr INTEGER DEFAULT 10e3;
  DECLARE records INTEGER DEFAULT 0;
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

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' fetched cursor...';
  SUSPEND;

  IF (nano$rslt.has_data(rslt)) THEN
  BEGIN
    eof = FALSE;
    WHILE (NOT eof) DO
    BEGIN
      i = 0;
      -- IN AUTONOMOUS TRANSACTION DO (uncomment...)
      BEGIN
        WHILE (i < tr AND NOT eof) DO -- COMMIT after 10000 records (uncomment...)
        BEGIN
          IF (nano$rslt.next_(rslt)) THEN
          BEGIN
            INSERT INTO perf_dest
            VALUES (-- int
                    nano$rslt.get_integer(:rslt, 0),
                    nano$rslt.get_integer(:rslt, 1),
                    nano$rslt.get_integer(:rslt, 2),
                    nano$rslt.get_integer(:rslt, 3),
                    nano$rslt.get_integer(:rslt, 4),
                    -- vc
                    nano$rslt.get_u8_string(:rslt, 5),
                    nano$rslt.get_u8_string(:rslt, 6),
                    nano$rslt.get_u8_string(:rslt, 7),
                    nano$rslt.get_u8_s_long(:rslt, 8),
                    nano$rslt.get_u8_s_huge(:rslt, 9),
                    -- ch
                    nano$rslt.get_u8_string(:rslt, 10),
                    nano$rslt.get_u8_string(:rslt, 11),
                    nano$rslt.get_u8_string(:rslt, 12),
                    nano$rslt.get_u8_s_long(:rslt, 13),
                    nano$rslt.get_u8_s_huge(:rslt, 14),
                    -- dbl
                    nano$rslt.get_double(:rslt, 15),
                    nano$rslt.get_double(:rslt, 16),
                    nano$rslt.get_double(:rslt, 17),
                    nano$rslt.get_double(:rslt, 18),
                    nano$rslt.get_double(:rslt, 19),
                    -- dt
                    nano$rslt.get_timestamp(:rslt, 20),
                    nano$rslt.get_timestamp(:rslt, 21),
                    nano$rslt.get_timestamp(:rslt, 22),
                    nano$rslt.get_timestamp(:rslt, 23),
                    nano$rslt.get_timestamp(:rslt, 24),
                    -- lob
                    nano$rslt.get_blob(:rslt, 25)
                   );
            i = i + 1;
          END
          ELSE
            eof = TRUE;
        END
      END
      IF (i > 0) THEN
      BEGIN
        records = records + i;
        status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' records: ' || records;
        SUSPEND;
      END
    END
  END

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' finished...';
  SUSPEND;

  nano$conn.release_(conn);
  nano$udr.finalize();
END