<?php
	class WadadliDate extends WadadliClass{
		/** @Integer(required=true) */
		protected $year;
		
		/** @Integer(required=true, min=1, max=12)*/
		protected $month;
		
		/** @Integer(required=true, min=1, max=31)*/
		protected $day;
		
		/** @Integer(min = 0, max= 23)*/
		protected $hours;
		
		/** @Integer(min = 0, max= 59)*/
		protected $minutes;
		
		/** @Integer(min = 0, max= 59)*/
		protected $seconds;
		
		public function getDatetimestamp(){
			return "{$this->getDatestamp()} {$this->getTimestamp()}";
		}
		
		public function getDatestamp(){
			$yearPart = $this->addLeadingZero($this->getYear(),4);
			$monthPart = $this->addLeadingZero($this->getMonth());
			$dayPart = $this->addLeadingZero($this->getDay());
			
			return "$yearPart-$monthPart-$dayPart";
		}
		
		public function getTimestamp(){			
			if($this->getHours()){
				$hourPart = $this->addLeadingZero($this->getHours());
			}else{
				$hourPart = "00";
			}
			
			if($this->getMinutes()){
				$minutePart = $this->addLeadingZero($this->getMinutes());
			}else{
				$minutePart = "00";
			}
			
			if($this->getSeconds()){
				$secondPart = $this->addLeadingZero($this->getSeconds());
			}else{
				$secondPart = "00";
			}
			
			return "$hourPart:$minutePart:$secondPart";
		}
		
		public function compare($otherDate){
			if(!is_object($otherDate) || get_class($otherDate) != get_class($this)){
				if(is_object($otherDate)){
					$type = get_class($otherDate);
				}else{
					$type = gettype($otherDate);
				}
				
				fatal_error(UNEXPECTED_ARGUMENT_WARNING, "WadadliDate can only be compared to other WadadliDates, $type given.");
				return;
			}
			
			if($otherDate->getYear() == $this->getYear() && 
				$otherDate->getMonth() == $this->getMonth() &&
				$otherDate->getDay() == $this->getDay() &&
				$otherDate->getHours() == $this->getHours() &&
				$otherDate->getMinutes() == $this->getMinutes() &&
				$otherDate->getSeconds() == $this->getSeconds()){
				return 0;		
			}
			
			if($otherDate->getYear() < $this->getYear() ||
				($otherDate->getYear() == $this->getYear() && $otherDate->getMonth() < $this->getMonth()) ||
				($otherDate->getYear() == $this->getYear() && $otherDate->getMonth() == $this->getMonth() 
					&& $otherDate->getDay() < $this->getDay()) ||
				($otherDate->getYear() == $this->getYear() && $otherDate->getMonth() == $this->getMonth() 
					&& $otherDate->getDay() == $this->getDay() && $otherDate->getHours() < $this->getHours()) ||
				($otherDate->getYear() == $this->getYear() && $otherDate->getMonth() == $this->getMonth() 
					&& $otherDate->getDay() == $this->getDay() && $otherDate->getHours() == $this->getHours()
					&& $otherDate->getMinutes() < $this->getMinutes()) ||
				($otherDate->getYear() == $this->getYear() && $otherDate->getMonth() == $this->getMonth() 
					&& $otherDate->getDay() == $this->getDay() && $otherDate->getHours() == $this->getHours()
					&& $otherDate->getMinutes() == $this->getMinutes() && $otherDate->getSeconds() < $this->getSeconds())){
				return 1;
			}else{
				return -1;
			}
		}
		
		public static function now(){
			return new WadadliDate((int)date("Y"),(int)date("m"),(int)date("d"),(int)date("H"),(int)date("i"),(int)date("s"));
		}
		
		public function __toString(){
			return $this->getDatetimestamp();
		}
		
		private function addLeadingZero($number,$size=2){
			while(strlen(strval($number)) < $size){
				$number = "0$number";
			}
			
			return $number;
		}
	}
?>
