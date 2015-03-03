<?php
	class WadadliInputContainer extends WadadliInputComponent{
		private $container;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$this->container = new WadadliContainer;
		}
		
		public function addContent($content){
			$this->container->addContent($content);
		}
		
		protected function getEntryComponent(){
			return $this->container;
		}
	}
?>