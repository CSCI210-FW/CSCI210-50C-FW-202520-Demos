-- SQLite
create table customer (
    cus_code integer primary key,
    cus_lname text not null,
    cus_fname text not null,
    cus_initial text,
    cus_areacode text not null,
    cus_phone text not null unique,
    cus_balance real not null default(0)
) strict