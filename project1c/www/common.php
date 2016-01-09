<?php function page_header(){ ?>
<!doctype html>
<html>

<head>
	<meta charset='utf-8'>
	<title>CS143 Project 1C</title>
	<link rel='stylesheet' type='text/css' href='style.css'>
</head>

<body>
<?php } 

function page_footer(){ ?>
</div>
</body>
</html>
<?php } ?>

<?php

function connect_db(){
	$db_connection = mysql_connect('localhost', 'cs143', '');

	if(!$db_connection){
		$errmsg = mysql_error($db_connection);
		echo "Connection failed: $errmsg <br>";
		exit(1);
	}

	mysql_select_db('CS143', $db_connection);

	return $db_connection;
}

function append_quotes($str){
	$retval = '\''. $str .'\'';
	return $retval;
}

?>
