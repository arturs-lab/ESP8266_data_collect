<?php 
header("Content-Type: text/plain");
echo "$_GET[id]\n";

//require "mysql_util.php";

$my_file=fopen('/tmp/datacollect.txt','a');
fwrite($my_file,date(DATE_ATOM)." id=$_GET[id],type=$_GET[type],val=$_GET[val]\n");
fclose($my_file);
?>
