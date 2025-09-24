-- SQLite
select count(*),
    movie.name,
    actor.first_name
from actor
    join cast as c on actor.id = c.actor_id
    join movie on c.movie_id = movie.id
where movie.name like '%shrek%'
group by movie.id
having count(*) > 10