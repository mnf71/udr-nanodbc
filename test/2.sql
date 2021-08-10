-- delete from sample;

execute block returns (e VARCHAR(512) character set utf8)
as
  declare conn ty$pointer;
  declare stmt ty$pointer;
begin

  begin
    conn = nano$conn.connection('cp1251', 'LOGFDB',  'SYSDBA', 'masterkey');
    when any do
    begin
      e = nano$conn.e_message();
      suspend;
      exit;
    end
  end

-- insert into sample (rec, i32, dbl, str)
  begin
    stmt = nano$stmt.statement_(conn, 'insert into sample (rec, i32) values (?, ?);');
--  stmt = nano$stmt.statement_(conn);
--  nano$stmt.prepare_(stmt, conn, 'insert into sample (rec, i32, dbl, str) values (?, ?);');
    when any do
    begin
      e = nano$conn.e_message();
      nano$conn.dispose(conn); /* todo: список активных ptr */
      suspend;
      exit;
    end
  end

  nano$stmt.bind_smallint(stmt, 0, 9123);
  nano$stmt.bind_integer(stmt, 1, 15);
  nano$stmt.execute_(stmt);

  nano$stmt.dispose(stmt);
  nano$conn.dispose(conn);

  when any do
  begin
    e = nano$conn.e_message();
    nano$stmt.dispose(stmt);
    nano$conn.dispose(conn);
    suspend;
    exit;
  end

end


