-- SQLite
select line.*,
    P_DESCRIPT
from PRODUCT
    join line on product.p_code = line.p_code