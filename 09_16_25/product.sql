-- SQLite
create table product (
    p_code text primary key,
    p_descript text not null,
    p_indate text default(date('now')),
    p_qoh integer not null default(0),
    p_min integer not null default(0),
    p_price real not null,
    p_discount real not null default(0)
) strict