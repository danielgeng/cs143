<?php

require_once('common.php');
page_header();

if(empty($_GET['id'])){ header('Location: search.php'); }
echo "<div class='content'>";

$id = $_GET['id'];

$query = "select concat(title, ' (', year, ')') as title, rating, company from Movie where id = $id";

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
echo '<h3>Movie information</h3><hr>';
echo 'Title: ' . $row['title'] . '<br>';
$company = empty($row['company']) ? 'N/A' : $row['company'];
echo 'Company: ' . $company . '<br>';
$rating = empty($row['rating']) ? 'N/A' : $row['rating'];
echo 'MPAA Rating: ' . $rating . '<br>';

$query = "select did from MovieDirector where mid = $id";
$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
echo 'Director: ';
if(mysql_num_rows($rs) == 0){
	echo 'N/A<br>';
}else{
	$arr = array();
    while($row = mysql_fetch_assoc($rs)){
    	$did = $row['did'];
    	$dir_query = "select concat(coalesce(first,''), ' ', coalesce(last,''), ' (', dob, ')') from Director where id = $did";
    	$drs = mysql_query($dir_query);
		if(!$drs){
			echo 'MySQL Error.';
			exit(1);
		}
		$drow = mysql_fetch_row($drs);
		array_push($arr, $drow[0]);
    }
    echo implode(', ', $arr);
    echo '<br>';
}

$query = "select genre from MovieGenre where mid = $id";
$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
echo 'Genre: ';
if(mysql_num_rows($rs) == 0){
	echo 'N/A<br>';
}else{
	$arr = array();
    while($row = mysql_fetch_row($rs)){
    	array_push($arr, $row[0]);
    }
    echo implode(', ', $arr);
    echo '<br>';
}

echo '<h3>Actor credits</h3><hr>';
$query = "select aid, role from MovieActor where mid = $id";
$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
if(mysql_num_rows($rs) == 0){
	echo 'No results found.';
}else{
	while($row = mysql_fetch_assoc($rs)){
		$aid = $row['aid'];
		$role = $row['role'];
    	$act_query = "select concat(coalesce(first,''), ' ', coalesce(last,'')) from Actor where id = $aid";
    	$ars = mysql_query($act_query);
		if(!$ars){
			echo 'MySQL Error.';
			exit(1);
		}
		$arow = mysql_fetch_row($ars);
		$aname = $arow[0];
		echo "$role -- <a href='showactor.php?id=$aid' target='content'>$aname</a><br>";
	}
}

echo '<h3>Reviews</h3><hr>';
$query = "select avg(rating) as avg, count(*) as num from Review where mid = $id group by mid";
$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
if(mysql_num_rows($rs) == 0){
	echo "No reviews found. <a href='addreview.php?id=$id' target='content'>Add a review?</a><br>";
}else{
	$row = mysql_fetch_assoc($rs);
	$avg = $row['avg'];
	$num = $row['num'];
	echo "<b>Average rating: $avg/5 from $num review(s). <a href='addreview.php?id=$id' target='content'>Add a review?</a></b><br><br><br>";
	$query = "select name, time, rating, comment from Review where mid = $id";
	$rs = mysql_query($query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}
	while($row = mysql_fetch_assoc($rs)){
		$rname = $row['name'];
		$rtime = $row['time'];
		$rrating = $row['rating'];
		$comment = $row['comment'];
		echo "<u>User: <b>$rname</b>, Time: $rtime, Rating: $rrating/5</u><br>$comment<br><br>";
	}
}

mysql_close();

page_footer(); ?>
