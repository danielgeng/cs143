<!doctype html>

<html>
	<head>
		<meta charset="utf-8">
		<title>Project 1A Calculator</title>
	</head>

	<body>
		<h1>Project 1A Calculator</h1>
		<p>
			<form method="GET">
				<input type="text" name="expr">
				<input type="submit" value="Calculate">
			</form>
		</p>

		<p>
		<h2>Result</h2>
		<?php
			function is_valid($expr){
				// check all chars are valid
				if(!preg_match("/^[-+*\/.\d]+$/", $expr)){
					return false;
				}
				// no two + * / . in a row
				elseif(preg_match("/^[+*\/.]{2,}$/", $expr)){
					return false;
				}
				// no three - in a row
				elseif(preg_match("/^[-]{3,}$/", $expr)){
					return false;
				}
				// do not start with + * / .
				elseif(preg_match("/^[+*\/.]/", $expr)){
					return false;
				}
				// do not end with - + * / .
				elseif(preg_match("/[-+*\/.]$/", $expr)){
					return false;
				}
				return true;
			}

			$expr = trim($_GET["expr"]);
			if(is_valid($expr)){
				$expr = str_replace("--", "+", $expr);
				if(preg_match("/\/[0]/", $expr)){  // does NOT handle cases such as 1/0.5
					echo "Division by zero error!";
				}else{
					$invalid = eval("\$ans=$expr;");
					if($invalid){
						echo "Invalid expression!";
					}else{
						echo $expr . " = " . $ans;
					}
				}
			}else{
				echo "Invalid expression!";
			}

		?>
		</p>
	</body>
</html>