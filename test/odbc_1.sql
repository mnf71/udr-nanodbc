execute block returns (e varCHAR(8191) character set utf8)
as
  declare conn ty$pointer;
  declare stmt ty$pointer;
  declare rslt ty$pointer;
  DECLARE str VARCHAR(40);
begin
  nano$udr.initialize();
  conn = nano$conn.connection('LOGFDB', 'SYSDBA', 'masterkey');
  stmt = nano$stmt.statement_();
  nano$stmt.open_(stmt, conn);
  nano$stmt.prepare_(stmt, 'select i32 from sample ', true);
  rslt = nano$stmt.execute_(stmt);
  if (nano$rslt.has_data(rslt)) then
  begin
    while (nano$rslt.next_(rslt)) do
    begin
      e = nano$rslt.get_integer(rslt, 0);
      suspend;
    end
  end

  nano$conn.release_(conn);
  nano$udr.finalize();
end

/*

get_integer

------ Performance info ------
Prepare time = 31ms
Execute time = 1m 9s 938ms
Avg fetch time = 0,07 ms
Current memory = 37 984 192
Max memory = 95 568 336
Memory buffers = 2 048
Reads from disk to cache = 43 612
Writes from cache to disk = 9
Fetches from cache = 1 175 028

get_varchar

------ Performance info ------
Prepare time = 31ms
Execute time = 2m 21s 516ms
Avg fetch time = 0,14 ms
Current memory = 37 968 400
Max memory = 40 198 912
Memory buffers = 2 048
Reads from disk to cache = 43 565
Writes from cache to disk = 12
Fetches from cache = 1 175 314

*/

