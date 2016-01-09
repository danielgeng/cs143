<?php

require_once('common.php');
page_header();

if(empty($_GET['id'])){ header('Location: search.php'); }

echo "<div class='content'>";
$id = $_GET['id'];

$query = "select concat(coalesce(first,''), ' ', coalesce(last,'')) as name, sex, dob, dod from Actor where id = $id";

$db_connection = connect_db();

$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
if(mysql_num_rows($rs) == 0){
	echo 'No results found.';
	exit(1);
}

$row = mysql_fetch_assoc($rs);
echo '<h3>Actor information</h3><hr>';
echo 'Name: ' . $row['name'] . '<br>';
$sex = empty($row['sex']) ? 'N/A' : $row['sex'];
echo 'Sex: ' . $sex . '<br>';
echo 'Date of birth: ' . $row['dob'] . '<br>';
$dod = empty($row['dod']) ? 'Still alive' : $row['dod'];
echo 'Date of death: ' . $dod . '<br>';

echo '<h3>Movies acted in</h3><hr>';

$query = "select * from MovieActor where aid = $id";

$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
if(mysql_num_rows($rs) == 0){
	echo 'No results found.';
}else{
	while($row = mysql_fetch_assoc($rs)){
		$mid = $row['mid'];
		$movie_query = "select title from Movie where id = $mid";
		$mrs = mysql_query($movie_query);
		if(!$mrs){
			echo 'MySQL Error.';
			exit(1);
		}
		$mrow = mysql_fetch_row($mrs);
		$movie_name = $mrow[0];
		echo 'Acted as ' . append_quotes(append_quotes($row['role'])) . " in <a href='showmovie.php?id=$mid' target='content'>$movie_name</a><br>";
	}
}

mysql_close();

page_footer(); ?>
