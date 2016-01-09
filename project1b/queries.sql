-- Names of all actors in the movie 'Die Another Day'
select distinct concat(first, ' ', last)
from Actor inner join MovieActor on Actor.id = MovieActor.aid
		   inner join Movie on MovieActor.mid = Movie.id
where title = 'Die Another Day';

-- Number of actors that starred in multiple movies
select count(*)
from
	(select *
	from MovieActor
	group by aid
	having count(mid) > 1) t;

-- Lists actors and directors who have the same date of birth and are not the same person
-- (May take a while to run)
select concat(a1.first, ' ', a1.last), concat(a2.first, ' ', a2.last)
from Actor a1 inner join Director a2 on a1.dob = a2.dob
where a1.id != a2.id;
