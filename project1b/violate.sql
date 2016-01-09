-- Primary key constraints

-- 1) Insert 2 actors with the same id
-- ERROR 1062 (23000) at line 6: Duplicate entry '1234' for key 'PRIMARY'
insert into Actor values (1234, 'chu', 'pika', NULL, '1996-02-27', NULL);
insert into Actor values (1234, 'saur', 'bulba', NULL, '1996-02-27', NULL);

-- 2) Insert 2 movies with the same id
-- ERROR 1062 (23000) at line 11: Duplicate entry '12345' for key 'PRIMARY'
insert into Movie values (12345, 'pokemon the 1st movie', 1999, 'amazing', 'nintendo');
insert into Movie values (12345, 'pokemon the 2nd movie', 2000, 'even more amazing', 'nintendo');

-- 3) Insert 2 directors with the same id
-- ERROR 1062 (23000) at line 16: Duplicate entry '4321' for key 'PRIMARY'
insert into Director values (4321, 'trump', 'donald', '1946-06-14', NULL);
insert into Director values (4321, 'obama', 'barack', '1961-08-04', NULL);

-- Referential integrity constraints

-- 1) -1 id does not exist in the Movie table
-- ERROR 1452 (23000) at line 22: Cannot add or update a child row: a foreign key constraint fails (`CS143`.`MovieGenre`, CONSTRAINT `MovieGenre_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
insert into MovieGenre values (-1, "cartoon");

-- 2) -1 id does not exist in the Movie table
-- ERROR 1452 (23000) at line 26: Cannot add or update a child row: a foreign key constraint fails (`CS143`.`MovieDirector`, CONSTRAINT `MovieDirector_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
insert into MovieDirector values (-1, 123);

-- 3) -1 id does not exist in the Director table
-- ERROR 1452 (23000) at line 30: Cannot add or update a child row: a foreign key constraint fails (`CS143`.`MovieDirector`, CONSTRAINT `MovieDirector_ibfk_2` FOREIGN KEY (`did`) REFERENCES `Director` (`id`))
insert into MovieDirector values (3, -1);

-- 4) -1 id does not exist in the Movie table
-- ERROR 1452 (23000) at line 34: Cannot add or update a child row: a foreign key constraint fails (`CS143`.`MovieActor`, CONSTRAINT `MovieActor_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
insert into MovieActor values (-1, 234, NULL);

-- 5) -1 id does not exist in the Actor table
-- ERROR 1452 (23000) at line 38: Cannot add or update a child row: a foreign key constraint fails (`CS143`.`MovieActor`, CONSTRAINT `MovieActor_ibfk_2` FOREIGN KEY (`aid`) REFERENCES `Actor` (`id`))
insert into MovieActor values (3, -1, NULL);

-- 6) -1 id does not exist in the Movie table
-- ERROR 1452 (23000) at line 42: Cannot add or update a child row: a foreign key constraint fails (`CS143`.`Review`, CONSTRAINT `Review_ibfk_1` FOREIGN KEY (`mid`) REFERENCES `Movie` (`id`))
insert into Review values ('chi long qua', 1111111111, -1, 5, 'best movie the world has ever seen');

-- CHECK constraints

-- 1) Insert actor who is not 'Male' or 'Female'
insert into Actor values (9, 'ketchum', 'ash', 'boy', '1996-02-27', NULL);

-- 2) Insert actor with date of death earlier than date of birth
insert into Actor values (8, 'traveler', 'time', 'Male', '2015-10-17', '1994-01-01');

-- 3) Insert review with rating above 5
insert into Review values ('pokemon fanatic', 1111111111, 12345, 6, 'really great movie, 10/10 would watch again');

-- 4) Insert actor with no name
insert into Actor values (7, '', '', 'Male', '2015-10-17', NULL);

-- 5) Insert movie with negative year
insert into Movie values (23456, 'pokemon the -100th movie', -1, 'r', 'samsung');

-- 6) Insert movie with no title
insert into Movie values (58821, '', 1994, 'r', 'apple');
