<?php
	class WadadliHeading extends WadadliContainer{
		/** @Integer(min=1, max=6, required=true, default=1) */
		protected $level;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			
			$this->setTag("h{$this->getLevel()}");
		}
	}
?>