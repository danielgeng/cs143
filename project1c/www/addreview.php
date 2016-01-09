<?php

require_once('common.php');
page_header();

$id = !empty($_GET['id']) ? $_GET['id'] : '';
if($id == ''){ header('Location: search.php'); }

$query = "select title from Movie where id = $id";
$db_connection = connect_db();
$rs = mysql_query($query);
if(!$rs){
	echo 'MySQL Error.';
	exit(1);
}
$row = mysql_fetch_row($rs);
$movie_name = $row[0];

?>

<div class='content'>
<h2>Add a review</h2>
<hr><br>
<form action='addreview.php?id=<?php echo $id?>' method='GET'>
	Name: <input type='text' name='name' maxlength='20'><br>
	Movie Name: <select name='movie'>
					<option selected><?php echo $movie_name; ?></option>
				</select>
				<br>
	Rating: <select name='rating'>
				<option selected='5'>5</option>
				<option value='4'>4</option>
				<option value='3'>3</option>
				<option value='2'>2</option>
				<option value='1'>1</option>
			</select>
			<br>
	Comment: <br>
	<textarea name='comment' rows='8' cols='60' maxlength='500'></textarea><br>
	<input type='submit' name='submit' value='Add your review!'>
	<input type='hidden' value='<?php echo $id ?>' name='id'>
</form>

<?php

if(isset($_GET['submit'])){
	echo '<br><hr>';

	if(empty($_GET['name'])){
		echo 'Error: You must enter a name.';
		exit(1);
	}

	$name = append_quotes($_GET['name']);
	$rating = (int)$_GET['rating'];
	$comment = !empty($_GET['comment']) ? append_quotes($_GET['comment']) : 'NULL';

	$query = "insert into Review(name, mid, rating, comment) values ($name, $id, $rating, $comment)";
	$rs = mysql_query($query);
	if(!$rs){
		echo 'MySQL Error.';
		exit(1);
	}

	echo "New review successfully added for <a href='showmovie.php?id=$id' target='content'>$movie_name</a>!";

	mysql_close();
}
page_footer(); ?>
