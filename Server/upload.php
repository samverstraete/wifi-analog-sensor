<?

error_reporting(E_ERROR | E_WARNING | E_PARSE);

class DB { static $link; }
DB::$link = mysqli_connect("127.0.0.1", "samver_test", "testje", "samver_test");
if (!DB::$link) die("Error: Unable to connect to MySQL.");


upload();

mysqli_close(DB::$link);


function upload(){
	$postdata = file_get_contents('php://input');
	file_put_contents("test.txt",$postdata);
	$json = json_decode($postdata);
	if ($json === NULL) {
		echo "JSON error";
	} else {
		$device = $_REQUEST['device'];
		//check device is number
		if (is_numeric($device) && $device > 0) {
			$msgtime = intval($json->frame->time);
			$nowtime = intval($json->millis);
			$min = floatval($json->frame->min);
			$max = floatval($json->frame->max);
			$mean = floatval($json->frame->mean);
			//calc log time
			$stime = time() - ($nowtime - $msgtime) / 1000;
			$sdate = date("Y-m-d H:i:s", $stime);
			$sql = "INSERT INTO `sensors` (`device`, `time`, `min`, `max`, `mean`) 
				VALUES ($device, '$sdate', '$min', '$max', '$mean')";
			if(mysqli_query(DB::$link, $sql)) {
				echo "ok";
			} else {
				echo mysqli_error(DB::$link);
				file_put_contents("sql.txt", mysqli_error(DB::$link));
			}
		} else {
			echo "Wrong device";
		}
	}
}

?>	