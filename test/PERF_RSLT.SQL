--DELETE FROM perf_src
--DELETE FROM perf_dest

EXECUTE PROCEDURE perf_fill;
/*

1000000 record(s) was(were) inserted into PERF_SRC

------ Performance info ------
Prepare time = 0ms
Execute time = 52s 687ms
Current memory = 133 915 792
Max memory = 337 748 016
Memory buffers = 4 096
Reads from disk to cache = 0
Writes from cache to disk = 73 820
Fetches from cache = 7 465 627

*/

SELECT * FROM perf_pump

/*
Plan
PLAN (PERF_PUMP NATURAL)

1000000 record(s) was(were) inserted into PERF_DEST

------ Performance info ------
Prepare time = 32ms
Execute time = 7m 23s 234ms
Avg fetch time = 4 675,33 ms
Current memory = 106 718 400
Max memory = 401 555 808
Memory buffers = 4 096
Reads from disk to cache = 164 099
Writes from cache to disk = 79 211
Fetches from cache = 15 053 379
*/

SELECT * FROM perf_pump_cursor

/*
Plan
PLAN (PERF_PUMP_CURSOR NATURAL)

1000000 record(s) was(were) inserted into PERF_DEST

------ Performance info ------
Prepare time = 32ms
Execute time = 37s 187ms
Avg fetch time = 357,57 ms
Current memory = 75 703 344
Max memory = 401 555 808
Memory buffers = 4 096
Reads from disk to cache = 77 520
Writes from cache to disk = 77 064
Fetches from cache = 11 635 665
*/