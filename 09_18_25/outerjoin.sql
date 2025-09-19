-- SQLite
SELECT PRODUCT.*,
    v_name
from vendor
    left join product on PRODUCT.V_CODE = vendor.v_code