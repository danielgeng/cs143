<?php
	require_once('common.php');
	page_header();
?>

<div class='content'>
<h2>Add a new actor or director</h2>
<hr><br>
<form action='' method='GET'>
	<input type='radio' name='type' value='Actor' checked='true'>Actor
	<input type='radio' name='type' value='Director'>Director<br><br>
	First name: <input type='text' name='first' maxlength='20'><br>
	Last name: <input type='text' name='last' maxlength='20'><br>
	Sex: <input type='radio' name='sex' value='Male'>Male
	<input type='radio' name='sex' value='Female'>Female<br>
	Date of birth: <input type='text' name='dob' placeholder='YYYY-MM-DD' maxlength='20'><br>
	Date of death: <input type='text' name='dod' placeholder='YYYY-MM-DD' maxlength='20'> (Leave blank if still living)<br><br>
	<input type='submit' name='submit' value='Add to database'>
</form>

<?php

function update_max_id(){
	$query = 'update MaxPersonID set id = id+1';
	mysql_query($query);
}

function convert_date($date){
	return str_replace('-', '', $date);
}

if(isset($_GET['submit'])){
	echo '<br><hr>';
	$type = $_GET['type'];
	$dob = $_GET['dob'];
	if(empty($_GET['first']) && empty($_GET['last'])){
		echo 'Error: You must enter a first or last name.';
		exit(1);
	}
	if(empty($_GET['dob'])){
		echo 'Error: You must enter a date of birth.';
		exit(1);
	}
	$first = !empty($_GET['first']) ? append_quotes($_GET['first']) : 'NULL';
	$last = !empty($_GET['last']) ? append_quotes($_GET['last']) : 'NULL';
	$sex = !empty($_GET['sex']) ? append_quotes($_GET['sex']) : 'NULL';
	$dod = !empty($_GET['dod']) ? $_GET['dod'] : 'NULL';

	if($dod != 'NULL'){
		if($dob > $dod){
			echo 'Error: A person cannot die before they are born!';
			exit(1);
		}
		$dod = convert_date($dod);
	}

	$dob = convert_date($dob);

	$db_connection = connect_db();

	$id_query = 'select max(id) from MaxPersonID';

	$rs = mysql_query($id_query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}

	$id = mysql_fetch_row($rs)[0];

	// echo "<br>$id $first $last $sex $dob $dod"; //debug

	if($type == 'Actor'){
		$query = "insert into Actor(id, last, first, sex, dob, dod) values ($id, $last, $first, $sex, $dob, $dod)";
		$rs = mysql_query($query);
		if(!$rs){
			echo 'MySQL Error.';
			exit(1);
		}
	}else{
		$query = "insert into Director(id, last, first, dob, dod) values ($id, $last, $first, $dob, $dod)";
		$rs = mysql_query($query);
		if(!$rs){
			echo 'MySQL Error.';
			exit(1);
		}
	}

	update_max_id();

	echo "New $type was entered successfully to the database!";

	mysql_close();
}

page_footer(); ?>
