execute block returns (e varCHAR(8191) character set utf8)
as
  DECLARE c SCROLL CURSOR FOR (select i32 from sample);

begin

  OPEN c;
  FETCH FIRST FROM c;
  WHILE (ROW_COUNT > 0) DO
  BEGIN
    e = c.i32;
    SUSPEND;
    FETCH NEXT FROM c;
  END

end

/*

integer

------ Performance info ------
Prepare time = 15ms
Execute time = 1m 43s 250ms
Avg fetch time = 0,10 ms
Current memory = 37 909 888
Max memory = 95 568 336
Memory buffers = 2 048
Reads from disk to cache = 43 487
Writes from cache to disk = 1
Fetches from cache = 1 173 914

string

------ Performance info ------
Prepare time = 31ms
Execute time = 1m 7s 687ms
Avg fetch time = 0,07 ms
Current memory = 37 891 792
Max memory = 95 568 336
Memory buffers = 2 048
Reads from disk to cache = 43 487
Writes from cache to disk = 1
Fetches from cache = 1 173 914
*/