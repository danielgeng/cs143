<?php
	require_once('common.php');
	page_header();
?>

<div class='content'>
<h2>Add a director to a movie</h2>
<hr><br>
<?php

$dir_query = "select id, concat(coalesce(first,''), ' ', coalesce(last,''), ' (', dob, ')') as name from Director";
$db_connection = connect_db();

$rs = mysql_query($dir_query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
echo "<form action='' method='GET'> Director: <select name='dir'>";
while($row = mysql_fetch_assoc($rs)){
	$did = $row['id'];
	$name = $row['name'];
	echo "<option value='$did'>$name</option>";
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
echo '</select><br><br>';
echo "<input type='submit' name='submit' value='Add relation'></form>";

$did = (int)$_GET['dir'];
$mid = (int)$_GET['movie'];

if(isset($_GET['submit'])){
	echo '<br><hr>';
	$query = "insert into MovieDirector(mid, did) values ($mid, $did)";

	$rs = mysql_query($query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}

	echo 'Relation successfully added!';
}

mysql_close();

page_footer(); ?>
