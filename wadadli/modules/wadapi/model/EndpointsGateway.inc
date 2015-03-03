<?php
	class EndpointsGateway extends XMLGateway{
		public function __construct(){
			parent::__construct("endpoints.xml");
		}
		
		public function find(){
			$arguments = array_merge(array('Endpoint'),func_get_args());
			return call_user_func_array(array('parent','find'), $arguments);
		}
	}
?>