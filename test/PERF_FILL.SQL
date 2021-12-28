CREATE OR ALTER PROCEDURE PERF_FILL
AS
  DECLARE idx INTEGER DEFAULT 10e5;
  DECLARE s TYPE OF COLUMN perf_src.s1;
BEGIN
  WHILE (idx > 0) DO
  BEGIN
    s =  CAST(idx AS TYPE OF COLUMN perf_src.s1);
    INSERT INTO perf_src
    VALUES (-- int
            :idx,
            :idx + 1,
            :idx + 2,
            :idx - 1,
            :idx - 2,
            -- vc
            :s, 
            :s || :s,
            :s || :s || :s || :s,
            :s || :s || :s || :s || :s || :s || :s || :s,
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s,
            -- ch
            :s, 
            :s || :s,
            :s || :s || :s || :s,
            :s || :s || :s || :s || :s || :s || :s || :s,
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s,
            -- dbl
            :idx + 0.5,
            :idx + 1.5,
            :idx + 2.5,
            :idx - 1.5,
            :idx - 2.5,
            -- dt
            CAST('today' AS TIMESTAMP),
            CAST('now' AS TIMESTAMP) - 2,
            CAST('now' AS TIMESTAMP) - 1,
            CAST('now' AS TIMESTAMP) + 1,
            CAST('now' AS TIMESTAMP),
            -- lob
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s ||
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s ||
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s ||
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s ||
            :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s || :s
           );
    idx = idx - 1;
  END 
END