<?php
	class ActionsGateway extends XMLGateway{
		public function __construct(){
			parent::__construct("/controller/actions.xml");
		}
		
		public function find(){
			$arguments = array_merge(array('Action'),func_get_args());
			return call_user_func_array(array('parent','find'), $arguments);
		}
	}
?>