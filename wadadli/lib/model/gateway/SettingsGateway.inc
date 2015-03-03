<?php
	class SettingsGateway extends XMLGateway{
		public function __construct($source="wadadli/conf/settings.xml"){
			parent::__construct($source);
		}
		
		public function find(){
			$arguments = array_merge(array('Settings'),func_get_args());
			return call_user_func_array(array('parent','find'), $arguments);
		}
	}
?>