<!doctype html>

<html>

<head>
	<meta charset="utf-8">
	<title>CS143 Project 1B</title>
</head>

<body>
	<p>
		Enter a query:
		<form action="" method="GET">
			<textarea name="query" rows="8" cols="60"></textarea>
			<input type="submit" value="Submit">
		</form>
	</p>
	<br>
	<?php
		if(!empty($_GET['query'])){
			$db_connection = mysql_connect("localhost", "cs143", "");

			if(!$db_connection){
				$errmsg = mysql_error($db_connection);
				echo "Connection failed: $errmsg <br>";
				exit(1);
			}

			mysql_select_db("CS143", $db_connection);

			$query = $_GET['query'];
			$sanitized_name = mysql_real_escape_string($query);
			$query_to_issue = sprintf($query, $sanitized_name);
			$rs = mysql_query($query_to_issue, $db_connection);
			
			$schema = mysql_fetch_assoc($rs);

			echo "Query result:<br><br>";
			echo "<table border='1' style='border-collapse: collapse'><tr>";
			foreach(array_keys($schema) as $attr){
				echo "<td align='center' valign='middle'><b>" . $attr . "</b></td>";
			}
			echo "</tr>";
			while($row = mysql_fetch_row($rs)){
			    echo "<tr>";
			    $sz = sizeof($row);
			    for($i = 0; $i < $sz; $i++){
			    	echo "<td align='center' valign='middle'>" . $row[$i] . "</td>";
			    }
			    echo "</tr>";
			}
			echo "</table>";

			mysql_close($db_connection);
		}
	?>

</body>

</html>
