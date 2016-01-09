<?php
	require_once('common.php');
	page_header();
?>

<div class='content'>
<h2>Add an actor to a movie</h2>
<hr><br>
<?php

$actor_query = "select id, concat(coalesce(first,''), ' ', coalesce(last,''), ' (', dob, ')') as name from Actor";
$db_connection = connect_db();

$rs = mysql_query($actor_query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
echo "<form action='' method='GET'> Actor: <select name='actor'>";
while($row = mysql_fetch_assoc($rs)){
	$aid = $row['id'];
	$aname = $row['name'];
	echo "<option value='$aid'>$aname</option>";
}
echo '</select><br>';

$movie_query = "select id, concat(title, ' (', year, ')') as title from Movie";
$rs = mysql_query($movie_query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
echo "Movie: <select name='movie'>";
while($row = mysql_fetch_assoc($rs)){
	$mid = $row['id'];
	$title = $row['title'];
	echo "<option value='$mid'>$title</option>";
}
echo '</select><br>';
echo "Role: <input type='text' name='role' maxlength='50'><br><br>";
echo "<input type='submit' name='submit' value='Add relation'></form>";

$aid = (int)$_GET['actor'];
$mid = (int)$_GET['movie'];
$role = !empty($_GET['role']) ? append_quotes($_GET['role']) : 'NULL';

if(isset($_GET['submit'])){
	echo '<br><hr>';
	$query = "insert into MovieActor(mid, aid, role) values ($mid, $aid, $role)";

	$rs = mysql_query($query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}

	echo 'Relation successfully added!';
}

mysql_close();

page_footer(); ?>
