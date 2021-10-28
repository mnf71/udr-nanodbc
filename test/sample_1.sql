execute block
as
  DECLARE i INTEGER = 1000000;
BEGIN
  while (i > 0) do
  begin
     insert into sample (    REC,
    I32,
    DBL,
    STR,
    DMY,
    L,
    B) values (:i / 100, :i, :i * 1.5, :i, 'now', true, :i || :i || :i || :i || :i);

     i = i - 1;
  end
END