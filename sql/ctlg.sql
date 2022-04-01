SET TERM ^ ;

CREATE OR ALTER PACKAGE NANO$CTLG
AS
BEGIN

  FUNCTION catalog_(conn TY$POINTER NOT NULL) RETURNS TY$POINTER;

  FUNCTION valid(ctlg TY$POINTER) RETURNS BOOLEAN;

  FUNCTION release_(ctlg TY$POINTER) RETURNS TY$POINTER;

  PROCEDURE find_tables(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ type_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 type_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_type VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_remarks VARCHAR(2048) CHARACTER SET NONE
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_type VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_remarks VARCHAR(2048) CHARACTER SET UTF8
    );

  PROCEDURE find_table_privileges(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ grantor VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ grantee VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ privilege VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ is_grantable VARCHAR(4) CHARACTER SET NONE
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 grantor VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 grantee VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 privilege VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 is_grantable VARCHAR(4) CHARACTER SET UTF8
    );

  PROCEDURE find_columns(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ column_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL
      -- utf8 column_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ column_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 column_name VARCHAR(128) CHARACTER SET UTF8,
      data_type SMALLINT,
      /* -- none */ type_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 type_name VARCHAR(128) CHARACTER SET UTF8,
      column_size INTEGER,
      buffer_length INTEGER,
      decimal_digits SMALLINT,
      numeric_precision_radix SMALLINT,
      nullable SMALLINT,
      /* -- none */ remarks VARCHAR(2048) CHARACTER SET NONE,
      /* -- none */ column_default VARCHAR(256) CHARACTER SET NONE,
      -- utf8 remarks VARCHAR(2048) CHARACTER SET UTF8,
      -- utf8 column_default VARCHAR(256) CHARACTER SET UTF8,
      sql_data_type SMALLINT,
      sql_datetime_subtype SMALLINT,
      char_octet_length INTEGER,
      ordinal_position INTEGER,
      /* -- none */ is_nullable VARCHAR(4) CHARACTER SET NONE
      -- utf8 is_nullable VARCHAR(4) CHARACTER SET UTF8
    );

  PROCEDURE find_primary_keys(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ primary_key_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ column_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 primary_key_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 column_name VARCHAR(128) CHARACTER SET UTF8,
      column_number SMALLINT
    );

  PROCEDURE find_procedures(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ procedure_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL
      -- utf8 procedure_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL
    ) RETURNS (
      /* -- none */ procedure_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 procedure_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_name VARCHAR(128) CHARACTER SET UTF8,
      procedure_type SMALLINT,
      /* -- none */ procedure_remarks VARCHAR(2048) CHARACTER SET NONE
      -- utf8 procedure_remarks VARCHAR(2048) CHARACTER SET UTF8
    );

  PROCEDURE find_procedure_columns(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ column_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ procedure_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE DEFAULT NULL
      -- utf8 column_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 procedure_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8 DEFAULT NULL
    ) RETURNS (
      /* -- none */ procedure_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ column_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 procedure_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 column_name VARCHAR(128) CHARACTER SET UTF8,
      column_type SMALLINT,
      data_type SMALLINT,
      /* -- none */ type_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 type_name VARCHAR(128) CHARACTER SET UTF8,
      column_size INTEGER,
      buffer_length INTEGER,
      decimal_digits SMALLINT,
      numeric_precision_radix SMALLINT,
      nullable SMALLINT,
      /* -- none */ remarks VARCHAR(2048) CHARACTER SET NONE,
      /* -- none */ column_default VARCHAR(256) CHARACTER SET NONE,
      -- utf8 remarks VARCHAR(2048) CHARACTER SET UTF8,
      -- utf8 column_default VARCHAR(256) CHARACTER SET UTF8,
      sql_data_type SMALLINT,
      sql_datetime_subtype SMALLINT,
      char_octet_length INTEGER,
      ordinal_position INTEGER,
      /* -- none */ is_nullable VARCHAR(4) CHARACTER SET NONE
      -- utf8 is_nullable VARCHAR(4) CHARACTER SET UTF8
    );

  PROCEDURE list_catalogs(
      ctlg TY$POINTER NOT NULL
    ) RETURNS (
      /* -- none */ catalog_name VARCHAR(128) CHARACTER SET NONE
      -- utf8 catalog_name VARCHAR(128) CHARACTER SET UTF8
    );

  PROCEDURE list_schemas(
      ctlg TY$POINTER NOT NULL
    ) RETURNS (
      /* -- none */ schema_name CHAR(128) CHARACTER SET NONE
      -- utf8 schema_name VARCHAR(128) CHARACTER SET UTF8
    );

END^

RECREATE PACKAGE BODY NANO$CTLG
AS
BEGIN

  FUNCTION catalog_(conn TY$POINTER NOT NULL) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!ctlg$catalog'
    ENGINE UDR;

  FUNCTION valid(ctlg TY$POINTER) RETURNS BOOLEAN
    EXTERNAL NAME 'nano!ctlg$valid'
    ENGINE UDR;

  FUNCTION release_(ctlg TY$POINTER) RETURNS TY$POINTER
    EXTERNAL NAME 'nano!ctlg$release'
    ENGINE UDR;
 
  PROCEDURE find_tables(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ type_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 type_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_type VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_remarks VARCHAR(2048) CHARACTER SET NONE
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_type VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_remarks VARCHAR(2048) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$find_tables'
    ENGINE UDR;

  PROCEDURE find_table_privileges(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ grantor VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ grantee VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ privilege VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ is_grantable VARCHAR(4) CHARACTER SET NONE
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 grantor VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 grantee VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 privilege VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 is_grantable VARCHAR(4) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$find_table_privileges'
    ENGINE UDR;

  PROCEDURE find_columns(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ column_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE
      -- utf8 column_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ column_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 column_name VARCHAR(128) CHARACTER SET UTF8,
      data_type SMALLINT,
      /* -- none */ type_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 type_name VARCHAR(128) CHARACTER SET UTF8,
      column_size INTEGER,
      buffer_length INTEGER,
      decimal_digits SMALLINT,
      numeric_precision_radix SMALLINT,
      nullable SMALLINT,
      /* -- none */ remarks VARCHAR(2048) CHARACTER SET NONE,
      /* -- none */ column_default VARCHAR(256) CHARACTER SET NONE,
      -- utf8 remarks VARCHAR(2048) CHARACTER SET UTF8,
      -- utf8 column_default VARCHAR(256) CHARACTER SET UTF8,
      sql_data_type SMALLINT,
      sql_datetime_subtype SMALLINT,
      char_octet_length INTEGER,
      ordinal_position INTEGER,
      /* -- none */ is_nullable VARCHAR(4) CHARACTER SET NONE
      -- utf8 is_nullable VARCHAR(4) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$find_columns'
    ENGINE UDR;

  PROCEDURE find_primary_keys(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ table_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE
      -- utf8 table_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8
    ) RETURNS (
      /* -- none */ table_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ table_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ primary_key_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ column_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 table_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 table_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 primary_key_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 column_name VARCHAR(128) CHARACTER SET UTF8,
      column_number SMALLINT
    )
    EXTERNAL NAME 'nano!ctlg$find_primary_keys'
    ENGINE UDR;

  PROCEDURE find_procedures(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ procedure_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE
      -- utf8 procedure_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8
    ) RETURNS (
      /* -- none */ procedure_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 procedure_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_name VARCHAR(128) CHARACTER SET UTF8,
      procedure_type SMALLINT,
      /* -- none */ procedure_remarks VARCHAR(2048) CHARACTER SET NONE
      -- utf8 procedure_remarks VARCHAR(2048) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$find_procedures'
    ENGINE UDR;

  PROCEDURE find_procedure_columns(
      ctlg TY$POINTER NOT NULL,
      /* -- none */ column_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ schema_ VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ catalog_ VARCHAR(128) CHARACTER SET NONE
      -- utf8 column_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 schema_ VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 catalog_ VARCHAR(128) CHARACTER SET UTF8
    ) RETURNS (
      /* -- none */ procedure_catalog VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_schema VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ procedure_name VARCHAR(128) CHARACTER SET NONE,
      /* -- none */ column_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 procedure_catalog VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_schema VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 procedure_name VARCHAR(128) CHARACTER SET UTF8,
      -- utf8 column_name VARCHAR(128) CHARACTER SET UTF8,
      column_type SMALLINT,
      data_type SMALLINT,
      /* -- none */ type_name VARCHAR(128) CHARACTER SET NONE,
      -- utf8 type_name VARCHAR(128) CHARACTER SET UTF8,
      column_size INTEGER,
      buffer_length INTEGER,
      decimal_digits SMALLINT,
      numeric_precision_radix SMALLINT,
      nullable SMALLINT,
      /* -- none */ remarks VARCHAR(2048) CHARACTER SET NONE,
      /* -- none */ column_default VARCHAR(256) CHARACTER SET NONE,
      -- utf8 remarks VARCHAR(2048) CHARACTER SET UTF8,
      -- utf8 column_default VARCHAR(256) CHARACTER SET UTF8,
      sql_data_type SMALLINT,
      sql_datetime_subtype SMALLINT,
      char_octet_length INTEGER,
      ordinal_position INTEGER,
      /* -- none */ is_nullable VARCHAR(4) CHARACTER SET NONE
      -- utf8 is_nullable VARCHAR(4) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$find_procedure_columns'
    ENGINE UDR;

  PROCEDURE list_catalogs(
      ctlg TY$POINTER NOT NULL
    ) RETURNS (
      /* -- none */ catalog_name VARCHAR(128) CHARACTER SET NONE
      -- utf8 catalog_name VARCHAR(128) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$list_catalogs'
    ENGINE UDR;

  PROCEDURE list_schemas(
      ctlg TY$POINTER NOT NULL
    ) RETURNS (
      /* -- none */ schema_name CHAR(128) CHARACTER SET NONE
      -- utf8 schema_name VARCHAR(128) CHARACTER SET UTF8
    )
    EXTERNAL NAME 'nano!ctlg$list_schemas'
    ENGINE UDR;

END^

SET TERM ; ^

/* Existing privileges on this package */

GRANT EXECUTE ON PACKAGE NANO$CTLG TO SYSDBA;