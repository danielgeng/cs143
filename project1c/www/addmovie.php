<?php
	require_once('common.php');
	page_header();
?>

<div class='content'>
<h2>Add a new movie</h2>
<hr><br>
<form action='' method='GET'>
	Title: <input type='text' name='title' maxlength='100' placeholder='Required'><br>
	Year: <input type='text' name='year' placeholder='YYYY' maxlength='4'><br>
	Company: <input type='text' name='company' maxlength='40'><br>
	MPAA Rating: <select name='rating'>
					<option selected=''></option>
					<option value='G'>G</option>
					<option value='PG'>PG</option>
					<option value='PG-13'>PG-13</option>
					<option value='R'>R</option>
					<option value='NC-17'>NC-17</option>
					<option value='surrendere'>surrendere</option>
				</select>
				<br>
	Genre: <input type='checkbox' name='genre[Action]' value='Action'>Action</input>
			<input type='checkbox' name='genre[Adult]' value='Adult'>Adult</input>
			<input type='checkbox' name='genre[Adventure]' value='Adventure'>Adventure</input>
			<input type='checkbox' name='genre[Animation]' value='Animation'>Animation</input>
			<input type='checkbox' name='genre[Comedy]' value='Comedy'>Comedy</input>
			<input type='checkbox' name='genre[Crime]' value='Crime'>Crime</input>
			<input type='checkbox' name='genre[Documentary]' value='Documentary'>Documentary</input>
			<input type='checkbox' name='genre[Drama]' value='Drama'>Drama</input>
			<input type='checkbox' name='genre[Family]' value='Family'>Family</input>
			<input type='checkbox' name='genre[Fantasy]' value='Fantasy'>Fantasy</input>
			<input type='checkbox' name='genre[Horror]' value='Horror'>Horror</input>
			<input type='checkbox' name='genre[Musical]' value='Musical'>Musical</input>
			<input type='checkbox' name='genre[Mystery]' value='Mystery'>Mystery</input>
			<input type='checkbox' name='genre[Romance]' value='Romance'>Romance</input>
			<input type='checkbox' name='genre[Sci-Fi]' value='Sci-Fi'>Sci-Fi</input>
			<input type='checkbox' name='genre[Short]' value='Short'>Short</input>
			<input type='checkbox' name='genre[Thriller]' value='Thriller'>Thriller</input>
			<input type='checkbox' name='genre[War]' value='War'>War</input>
			<input type='checkbox' name='genre[Western]' value='Western'>Western</input>
		<br><br>
	<input type='submit' name='submit' value='Add to database'>
</form>

<?php

function update_max_id(){
	$query = 'update MaxMovieID set id = id+1';
	mysql_query($query);
}

if(isset($_GET['submit'])){
	echo '<br><hr>';

	if(empty($_GET['title'])){
		echo 'Error: You must enter a title.';
		exit(1);
	}

	$title = !empty($_GET['title']) ? append_quotes($_GET['title']) : 'NULL';
	$year = !empty($_GET['year']) ? (int)$_GET['year'] : 'NULL';
	$company = !empty($_GET['company']) ? append_quotes($_GET['company']) : 'NULL';
	$rating = !empty($_GET['rating']) ? append_quotes($_GET['rating']) : 'NULL';

	if(is_int($year)){
		if($year < 0){
			echo 'Error: Movies released before 0 AD are not allowed!';
			exit(1);
		}
	}

	$db_connection = connect_db();

	$id_query = 'select max(id) from MaxMovieID';

	$rs = mysql_query($id_query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}

	$id = (int)mysql_fetch_row($rs)[0];

	// echo "<br> $id $title $year $rating $company"; //debug

	$query = "insert into Movie(id, title, year, rating, company) values ($id, $title, $year, $rating, $company)";
	$rs = mysql_query($query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}

	$genres = $_GET['genre'];
	foreach($genres as $genre){
		$genre = append_quotes($genre);
		$query = "insert into MovieGenre(mid, genre) values ($id, $genre)";
		$rs = mysql_query($query);
		if(!$rs){
			echo 'MySQL Error.';
			exit(1);
		}	
	}

	update_max_id();

	echo "New movie was entered successfully to the database!";

	mysql_close();
}

page_footer(); ?>
