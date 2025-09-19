-- SQLite
select *
from INVOICE
where inv_date not between '2018-01-16' and '2018-01-17';
select *
from invoice
where inv_date > '2018-01-16'
    and inv_date < '2018-01-18'