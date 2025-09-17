-- SQLite
alter table product
add column v_code integer references vendor(v_code)