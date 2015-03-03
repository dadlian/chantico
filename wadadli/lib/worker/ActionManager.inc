<?php
	class ActionManager extends Worker{
		protected static $actions;
		private static $actionsGateway;
		
		public static function getAction($name){
			if(!self::$actions){
				self::$actions = array();
			}
			
			if(!self::$actionsGateway){
				self::$actionsGateway = new ActionsGateway();
			}
			
			if(!array_key_exists($name,self::$actions)){
				$action = self::$actionsGateway->findUnique('name',$name);
				
				self::$actions[$name] = "";
				if($action){
					self::$actions[$name] = $action;
				}
			}
		
			return self::$actions[$name];
		}
	}
?>