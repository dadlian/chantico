<?php
	class Timer{
		static protected $times = array();
		
		public static function addTime($item, $startTime, $endTime){
			$startTime = explode(" ",$startTime);
			$startTime = $startTime[1] + $startTime[0];
			$endTime = explode(" ",$endTime);
			$endTime = $endTime[1] + $endTime[0];
			
			if(!array_key_exists($item,self::$times)){
				self::$times[$item] = 0;
			}
			
			self::$times[$item] = self::$times[$item] + (floatval($endTime)-floatval($startTime));
		}
		
		public static function printTimes(){
			foreach(self::$times as $item => $time){
				echo "$item: $time seconds<BR>\n";
			}
		}
	}
?>