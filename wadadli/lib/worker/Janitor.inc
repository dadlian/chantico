<?php
	class Janitor extends Worker{		
		protected static function cleanup($success=true){
			//Save undelivered messages
			Postman::saveMessages();
			
			//Clear Object Cache
			QuarterMaster::decommission();
			
			$registeredActions = Registrar::getRegistered('Action');
			foreach($registeredActions as $action){
				Registrar::unregister($action);
			}
			
			//Cleanup all currently active controllers
			$registeredControllers = Registrar::getRegistered('Controller');
			foreach($registeredControllers as $controller){
				$controller->finish();
				Registrar::unregister($controller);
			}
			
			//Rollback and close all active connections
			$registeredConnections = Registrar::getRegistered('DatabaseConnection');
			foreach($registeredConnections as $connection){
				if($success){
					$connection->commit();
				}else{
					$connection->rollback();
				}
				
				$connection->close();
				Registrar::unregister($connection);
			}
		}
	}
?>