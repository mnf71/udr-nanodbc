SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$CONN
AS
BEGIN

  FUNCTION connection(
    udr_locale varchar(20) character set none not null default 'cp1251',
    attr varchar(512) character set utf8 default null,
    user_ varchar(63) character set utf8 default null,
    password_ varchar(63) character set utf8 default null,
    timeout integer not null default 0
  ) returns ty$pointer;

  FUNCTION dispose(
    conn ty$pointer not null
  ) returns ty$pointer;

  function e_message (
  ) returns varchar(512) character set utf8;

END^

RECREATE PACKAGE BODY NANO$CONN
AS
BEGIN

  FUNCTION connection(
    udr_locale varchar(20) character set none not null ,
    attr varchar(512) character set utf8,
    user_ varchar(63) character set utf8,
    password_ varchar(63) character set utf8,
    timeout integer not null
  ) returns ty$pointer
  external name 'nano!conn_connection'
  engine udr;

  FUNCTION dispose(
    conn ty$pointer not null
  ) returns ty$pointer
  external name 'nano!conn_dispose'
  engine udr;

  function e_message (
  ) returns varchar(512) character set utf8
  external name 'nano!conn_e_message'
  engine udr;


END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$CONN TO SYSDBA;

SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$FUNC
AS
BEGIN

 function execute_conn (
   conn ty$pointer not null,
   query varchar(8191) character set utf8 not null,
    batch_operations integer not null default 1,
    timeout integer not null default 0
  ) returns ty$pointer;

  function just_execute_conn (
   conn ty$pointer not null,
   query varchar(8191) character set utf8 not null,
    batch_operations integer not null default 1,
    timeout integer not null default 0
  ) returns ty$nano_blank;

END^

RECREATE PACKAGE BODY NANO$FUNC
AS
BEGIN

 function execute_conn (
   conn ty$pointer not null,
   query varchar(8191) character set utf8 not null,
    batch_operations integer not null,
    timeout integer not null
  ) returns ty$pointer
  external name 'nano!func_execute_conn'
  engine udr;

  function just_execute_conn (
   conn ty$pointer not null,
   query varchar(8191) character set utf8 not null,
    batch_operations integer not null,
    timeout integer not null
  ) returns ty$nano_blank
  external name 'nano!func_just_execute_conn'
  engine udr;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$FUNC TO SYSDBA;


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