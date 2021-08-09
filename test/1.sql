execute block returns (e VARCHAR(512) character set utf8)
as
 declare conn ty$pointer;
begin
--  conn = nano$conn.connection('cp1251', 'LOGFDB',  'SYSDBA', 'masterkey');
  conn = nano$conn.connection('cp1251', 'срт8LOGFDB',  'SYSDBA', 'masterkey');
--  nano$func.just_execute_conn(conn, 'insert into sample (rec) values (1153)');
--  nano$conn.dispose(conn);
  when any do
  begin
    e = nano$conn.e_message();
    suspend;
  end
end