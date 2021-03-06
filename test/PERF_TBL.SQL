/******************************************************************************/
/***         Generated by IBExpert 2020.8.31.2 28.12.2021 17:20:39          ***/
/******************************************************************************/

/******************************************************************************/
/***      Following SET SQL DIALECT is just for the Database Comparer       ***/
/******************************************************************************/


/******************************************************************************/
/***                                 Tables                                 ***/
/******************************************************************************/



CREATE TABLE PERF_DEST (
    I1  INTEGER,
    I2  INTEGER,
    I3  INTEGER,
    I4  INTEGER,
    I5  INTEGER,
    S1  VARCHAR(20) CHARACTER SET UTF8,
    S2  VARCHAR(80) CHARACTER SET UTF8,
    S3  VARCHAR(160) CHARACTER SET UTF8,
    S4  VARCHAR(320) CHARACTER SET UTF8,
    S5  VARCHAR(640) CHARACTER SET UTF8,
    C1  CHAR(20) CHARACTER SET UTF8,
    C2  CHAR(80) CHARACTER SET UTF8,
    C3  CHAR(160) CHARACTER SET UTF8,
    C4  CHAR(320) CHARACTER SET UTF8,
    C5  CHAR(640) CHARACTER SET UTF8,
    F1  DOUBLE PRECISION,
    F2  DOUBLE PRECISION,
    F3  DOUBLE PRECISION,
    F4  DOUBLE PRECISION,
    F5  DOUBLE PRECISION,
    D1  DATE,
    D2  DATE,
    D3  DATE,
    D4  DATE,
    D5  DATE,
    B1  BLOB SUB_TYPE 1 SEGMENT SIZE 16384 CHARACTER SET KOI8U
);

CREATE TABLE PERF_SRC (
    I1  INTEGER,
    I2  INTEGER,
    I3  INTEGER,
    I4  INTEGER,
    I5  INTEGER,
    S1  VARCHAR(20) CHARACTER SET UTF8,
    S2  VARCHAR(80) CHARACTER SET UTF8,
    S3  VARCHAR(160) CHARACTER SET UTF8,
    S4  VARCHAR(320) CHARACTER SET UTF8,
    S5  VARCHAR(640) CHARACTER SET UTF8,
    C1  CHAR(20) CHARACTER SET UTF8,
    C2  CHAR(80) CHARACTER SET UTF8,
    C3  CHAR(160) CHARACTER SET UTF8,
    C4  CHAR(320) CHARACTER SET UTF8,
    C5  CHAR(640) CHARACTER SET UTF8,
    F1  DOUBLE PRECISION,
    F2  DOUBLE PRECISION,
    F3  DOUBLE PRECISION,
    F4  DOUBLE PRECISION,
    F5  DOUBLE PRECISION,
    D1  DATE,
    D2  DATE,
    D3  DATE,
    D4  DATE,
    D5  DATE,
    B1  BLOB SUB_TYPE 1 SEGMENT SIZE 16384 CHARACTER SET KOI8U
);



/******************************************************************************/
/***                                Indices                                 ***/
/******************************************************************************/

CREATE INDEX PERF_DEST_I1 ON PERF_DEST (I1);
CREATE INDEX PERF_SRC_I1 ON PERF_SRC (I1);


/******************************************************************************/
/***                               Privileges                               ***/
/******************************************************************************/



/******************************************************************************/
/***                             DDL privileges                             ***/
/******************************************************************************/

