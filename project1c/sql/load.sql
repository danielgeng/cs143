load data local infile '~/data/movie.del' into table Movie
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/actor1.del' into table Actor
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/actor2.del' into table Actor
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/actor3.del' into table Actor
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/director.del' into table Director
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/moviegenre.del' into table MovieGenre
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/moviedirector.del' into table MovieDirector
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/movieactor1.del' into table MovieActor
fields terminated by ',' optionally enclosed by '"';

load data local infile '~/data/movieactor2.del' into table MovieActor
fields terminated by ',' optionally enclosed by '"';

insert into MaxPersonID values
	((select max(id)+1
	from
		(select max(id) as id
		from Actor
		union
		select max(id) as id
		from Director) t));

insert into MaxMovieID values
	((select max(id)+1
	from Movie));
