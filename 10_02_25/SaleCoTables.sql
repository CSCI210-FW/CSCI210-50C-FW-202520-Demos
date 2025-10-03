CREATE TABLE CUSTOMER (
    CUST_NUMBER integer primary key,
    CUST_LNAME varchar2(15) not null,
    CUST_FNAME varchar2(15) not null,
    CUST_INITIAL varchar2(1),
    CUST_AREACODE varchar2(3),
    CUST_PHONE varchar2(8) not null,
    CUST_BALANCE real
);
CREATE TABLE ACCT_TRANSACTION (
    ACCT_TRANS_NUM integer primary key,
    ACCT_TRANS_DATE date,
    CUST_NUMBER integer references customer(cust_number),
    ACCT_TRANS_TYPE varchar2(8),
    ACCT_TRANS_AMOUNT real
);
CREATE TABLE INVOICE (
    INV_NUMBER integer primary key,
    CUST_NUMBER integer references customer(cust_number) not null,
    INV_DATE date not null,
    INV_SUBTOTAL real,
    INV_TAX real,
    INV_TOTAL real,
    INV_PAY_TYPE varchar2(5),
    INV_PAY_AMOUNT real,
    INV_BALANCE real
);
CREATE TABLE VENDOR (
    VEND_NUMBER integer primary key,
    VEND_NAME varchar2(15) not null,
    VEND_CONTACT varchar2(50) not null,
    VEND_AREACODE varchar2(3) not null,
    VEND_PHONE varchar2(8) not null,
    VEND_STATE varchar2(2) not null,
    VEND_ORDER varchar2(1) not null
);
CREATE TABLE PRODUCT (
    PROD_CODE varchar2(10) primary key,
    PROD_DESCRIPT varchar2(35) not null,
    PROD_INDATE date not null,
    PROD_QOH integer not null,
    PROD_MIN integer not null,
    PROD_PRICE real not null,
    PROD_DISCOUNT real not null,
    VEND_NUMBER integer references vendor(vend_number)
);
CREATE TABLE LINE (
    INV_NUMBER integer references invoice(inv_number),
    LINE_NUMBER integer,
    PROD_CODE varchar2(10) references product(prod_code) not null,
    LINE_UNITS real not null,
    LINE_PRICE real not null,
    LINE_AMOUNT real,
    primary key (inv_number, line_number)
);