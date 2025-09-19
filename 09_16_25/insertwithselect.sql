-- SQLite
insert into line
VALUES (
        (
            select max(inv_number)
            from invoice
        ),
        1,
(
            select p_code
            from product
            where p_qoh > 10
            limit 1
        ), 4,(
            select p_price
            from product
            where p_qoh > 10
            limit 1
        )
    )