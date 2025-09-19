-- SQLite
select p_descript as `Description`,
    p_qoh as `Quantity on Hand`,
    p_price as `Price`,
    p_qoh * p_price as `Total Value`
from product