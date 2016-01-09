create table Movie(
	id int not null,
	title varchar(100) not null,
	year int,
	rating varchar(50),
	company varchar(50),
	primary key (id),
	-- non-negative id
	check(id >= 0),
	-- a movie must have a title
	check(length(title) > 0),
	-- no movies created before 0 AD
	check(year > 0)
) engine = innodb;

create table Actor(
	id int not null,
	last varchar(20),
	first varchar(20),
	sex varchar(6),
	dob date not null,
	dod date,
	primary key (id),
	-- non-negative id
	check(id >= 0),
	-- only 2 genders in the given dataset
	check(sex = 'Male' or sex = 'Female'),
	-- can't die before you're born
	check((dod is not null and dob < dod) or dod is null),
	-- must have some type of name
	check((first is not null and length(first) > 0) or (last is not null and length(last) > 0))
) engine = innodb;

create table Director(
	id int not null,
	last varchar(20),
	first varchar(20),
	dob date not null,
	dod date,
	primary key (id),
	-- can't die before you're born
	check((dod is not null and dob < dod) or dod is null),
	-- must have some type of name
	check((first is not null and length(first) > 0) or (last is not null and length(last) > 0))
) engine = innodb;

create table MovieGenre(
	mid int not null,
	genre varchar(20) not null,
	foreign key (mid) references Movie(id)
) engine = innodb;

create table MovieDirector(
	mid int not null,
	did int not null,
	foreign key (mid) references Movie(id),
	foreign key (did) references Director(id)
) engine = innodb;

create table MovieActor(
	mid int not null,
	aid int not null,
	role varchar(50),
	foreign key (mid) references Movie(id),
	foreign key (aid) references Actor(id)
) engine = innodb;

create table Review(
	name varchar(20) not null,
	time timestamp not null,
	mid int not null,
	rating int not null,
	comment varchar(500),
	foreign key (mid) references Movie(id),
	-- each person can only write a review for each movie once
	unique (name, mid),
	-- rating between 1 and 5
	check(rating > 0 and rating <= 5)
) engine = innodb;

create table MaxPersonID(
	id int not null,
	primary key (id)
) engine = innodb;

create table MaxMovieID(
	id int not null,
	primary key (id)
) engine = innodb;
