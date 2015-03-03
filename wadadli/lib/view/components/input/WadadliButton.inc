<?php
	class WadadliButton extends WadadliInputComponent{
		/** @Boolean(required=true, default=false) */
		protected $submit;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			
			$this->setTag('button');
			
			if($this->getSubmit()){
				$this->addAttribute('type','submit');
			}else{
				$this->addAttribute('type','button');
			}
			
			$this->toggleLabel();
		}
		
		public function getEntryComponent(){
			return new WadadliStringComponent($this->getTitle());
		}
	}
?>