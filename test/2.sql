-- delete from sample;

execute block returns (e VARCHAR(512) character set utf8)
as
  declare conn ty$pointer;
  declare stmt ty$pointer;
  declare stmt2 ty$pointer;
begin

  nano$.set_locale('cp1251');
  begin
    conn = nano$conn.connection('LOGFDB',  'SYSDBA', 'masterkey');
    when any do
    begin
      e = nano$.e_message();
      suspend;
      exit;
    end
  end

-- insert into sample (rec, i32, dbl, str)
  begin
--    stmt = nano$stmt.statement_(conn, 'insert into sample (dbl, rec, i32, dmy) values (?, ?, ?, ?);');
--    stmt = nano$stmt.statement_(conn, 'insert into sample (dbl, rec, i32) values (?, ?, ?);');
--    stmt = nano$stmt.statement_(conn, 'insert into sample (str) values (?);');
--    stmt = nano$stmt.statement_(conn, 'insert into sample (dmy) values (?);');

--  stmt = nano$stmt.statement_(conn, 'insert into sample (rec, i32, dbl, str) values (?, ?, ?, ?);');
--  nano$stmt.prepare_(stmt, null, 'insert into sample (rec, i32, dbl, str) values (?, ?, ?, ?);');

    stmt = nano$stmt.statement_(conn);
    nano$stmt.prepare_(stmt, null, 'insert into sample (rec, i32, dbl, str, dmy) values (?, ?, ?, ?, ?);');
    stmt2 = nano$stmt.statement_(conn);
--    nano$stmt.prepare_(stmt2, null, 'insert into sample (rec, i32, dbl, str) values (?, ?, ?, ?);');

    when any do
    begin
      e = nano$.e_message();
      nano$conn.dispose(conn); /* todo: список активных ptr */
      suspend;
      exit;
    end
  end

  nano$stmt.bind_smallint(stmt, 0, 1);
  nano$stmt.bind_integer(stmt, 1, 15);
  nano$stmt.bind_double(stmt, 2, 1.5);
  nano$stmt.bind_string(stmt, 3, 'строка');
  nano$stmt.bind_date(stmt, 4, '06.09.2021 17:59:33');
/*
  nano$stmt.bind_smallint(stmt, 0, 211);
  nano$stmt.bind_integer(stmt, 1, 1115);
  nano$stmt.bind_double(stmt, 2, 111.5);
  nano$stmt.bind_string(stmt, 3, 'ст11рока');
  nano$stmt.bind_date(stmt, 4, '6.09.2021 17:01:33');
*/
  nano$stmt.just_execute_(stmt, 1);
--  nano$stmt.just_execute_(stmt2);

  nano$stmt.dispose(stmt);
  nano$stmt.dispose(stmt2);
  nano$conn.dispose(conn);

  when any do
  begin
    e = nano$.e_message();
    nano$stmt.dispose(stmt);
    nano$stmt.dispose(stmt2);
    nano$conn.dispose(conn);
    suspend;
    exit;
  end

end


