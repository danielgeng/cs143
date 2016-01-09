<?php

require_once('common.php');
page_header();

$search = !empty($_GET['search']) ? $_GET['search'] : '';

echo "<div class='content'>";

if($search == ''){ ?>
	<h3>Please enter a non-empty query.</h3>
	<hr><br>
	Search: 
	<form action='' method='GET'>
		<input type='text' name='search'>
		<input type='submit' value='Search'>
	</form>
<?php } 
else{
	echo "<h3>Search results for $search:</h3><hr>";

	$search_list = explode(' ', strtolower($search));

	$actor_query = "select id, concat(coalesce(first,''), ' ', coalesce(last,''), ' (', dob, ')') from Actor where 1";
	$movie_query = "select id, concat(title, ' (', year, ')') from Movie where 1";

	foreach($search_list as $s){
		$actor_query .= " and (lower(first) like '%$s%' or lower(last) like '%$s%')";
		$movie_query .= " and (lower(title) like '%$s%')";
	}

	$db_connection = connect_db();

	$rs = mysql_query($actor_query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}
	echo '<h4>Actors found:</h4>';
	while($row = mysql_fetch_row($rs)){
	    echo "Actor: <a href='showactor.php?id=$row[0]' target='content'>$row[1]</a><br>";
	}

	$rs = mysql_query($movie_query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}
	echo '<br><hr><h4>Movies found:</h4>';
	while($row = mysql_fetch_row($rs)){
	    echo "Movie: <a href='showmovie.php?id=$row[0]' target='content'>$row[1]</a><br>";
	}

	mysql_close();
}

page_footer(); ?>
