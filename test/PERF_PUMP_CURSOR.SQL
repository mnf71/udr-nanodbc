CREATE OR ALTER PROCEDURE PERF_PUMP_CURSOR
RETURNS (
    STATUS VARCHAR(256) CHARACTER SET UTF8)
AS
  DECLARE c_sql CURSOR FOR (
SELECT -- FIRST
       i1, i2, i3, i4, i5,
       s1, s2, s3, s4, s5,
       c1, c2, c3, c4, c5,
       f1, f2, f3, f4, f5,
       d1, d2, d3, d4, d5,
       b1
  FROM perf_src
);
  DECLARE eof BOOLEAN;
  DECLARE tr INTEGER DEFAULT 10e3;
  DECLARE records INTEGER DEFAULT 0;
  DECLARE i INTEGER;
BEGIN

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' started...';
  SUSPEND;

  IF (EXISTS (SELECT * FROM perf_src)) THEN
  BEGIN
    status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' open cursor...';
    SUSPEND;

    OPEN c_sql;

    status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' fetched cursor...';
    SUSPEND;

    eof = FALSE;
    WHILE (NOT eof) DO
    BEGIN
      --
      -- FETCH CURSOR is not supported inside IN AUTONOMOUS TRANSACTION block.
      --
      -- IN AUTONOMOUS TRANSACTION DO
      -- BEGIN
        i = 0;
        WHILE (i < tr AND NOT eof) DO
        BEGIN
          FETCH NEXT FROM c_sql;
          IF (ROW_COUNT > 0) THEN
          BEGIN
            INSERT INTO perf_dest
            VALUES (-- int
                    c_sql.i1,
                    c_sql.i2,
                    c_sql.i3,
                    c_sql.i4,
                    c_sql.i5,
                    -- vc
                    c_sql.s1,
                    c_sql.s2,
                    c_sql.s3,
                    c_sql.s4,
                    c_sql.s5,
                    -- ch
                    c_sql.c1,
                    c_sql.c2,
                    c_sql.c3,
                    c_sql.c4,
                    c_sql.c5,
                    -- dbl
                    c_sql.f1,
                    c_sql.f2,
                    c_sql.f3,
                    c_sql.f4,
                    c_sql.f5,
                    -- dt
                    c_sql.d1,
                    c_sql.d2,
                    c_sql.d3,
                    c_sql.d4,
                    c_sql.d5,
                    -- lob
                    c_sql.b1
                   );
            i = i + 1;
          END
          ELSE
            eof = TRUE;
        END
      -- END
      IF (i > 0) THEN
      BEGIN
        records = records + i;
        status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' records: ' || records;
        SUSPEND;
      END
    END
    CLOSE c_sql;
  END

  status = 'stamp: ' || CAST('now' AS TIMESTAMP) || ' finished...';
  SUSPEND;
END
