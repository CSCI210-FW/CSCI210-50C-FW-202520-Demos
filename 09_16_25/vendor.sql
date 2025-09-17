-- SQLite
create table vendor (
    v_code integer primary key,
    v_name text not null,
    v_contact text,
    v_areacode text not null,
    v_phone text not null,
    v_state text,
    v_order text default('N')
) strict