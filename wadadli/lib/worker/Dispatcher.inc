<?php	
	class Dispatcher extends Worker{
		protected static function dispatchRequest(){
			SessionManager::startSession();
			Postman::loadMessages();
			
			//Load Wadadli Settings
			$urlRoot = implode("/",array_compress(preg_split("/\//", SettingsManager::getSetting('Install','Url Root'))));
		
			//Get input URL relative to the url root, stripping off excess backslashes
			$regex = "/http:\/\/(www\.)?".str_replace("/","\/",$urlRoot)."\//";
			$destination = implode("/",array_compress(preg_split("/\//", preg_replace($regex, "", 
							urldecode("http://{$_SERVER['HTTP_HOST']}{$_SERVER['REQUEST_URI']}")))));
			
			//Find the action who's URL pattern matches the given destination
			$actionsGateway = new ActionsGateway();
			$action = $actionsGateway->findUnique('path',$destination);

			if(!$action){
				fatal_error(RESOURCE_UNAVAILABLE_ERROR, "There is no action mapped to '/$destination' in actions.xml.");
				return;
			}
			
			//Confirm valid action configuration
			$actionUrlPattern = implode("/",array_compress(preg_split("/\//", $action->getPath())));
			if(!substr_count($actionUrlPattern, '@') == sizeof($action->getParameters())){
				fatal_error(CONFIGURATION_ERROR,"Parameter count mismatch for action '/$destination' in actions.xml.");
				return;
			}
			
			Registrar::register($action);
			$url = new URLPattern($destination);
			$urlArguments = $url->extractArguments($actionUrlPattern);

			$controllerClass = $action->getController();
			$actionParameters = $action->getParameters();

			//Authorise User
			$allowedRoles = $action->getRoles();
			if($allowedRoles){
				//If the user needs to login change the controller class accordingly
				if(!SessionManager::activeUser()){
					Postman::postSuccess("You must be logged in to perform this action.");
					
					$controllerClass = SettingsManager::getSetting('Authentication','Controller');
					if(!$controllerClass){
						$controllerClass = "Login";
					}
					
					$actionParameters = array();
				}else if(!in_array(SessionManager::activeUser()->getRole(), $allowedRoles)){
					fatal_error("Access denied to http://$urlRoot/$destination. Login or contact your site administrator.",
							SessionManager::activeUser()->getRole()." not authorised to access http://$urlRoot/$destination.");
					return;
				}
			}

			//If user is authorised or action does not need authorisation
			if(class_exists($controllerClass)){
				$controllerArguments = array();
				$i = 0;
				foreach($actionParameters as $actionParameter){
					$controllerArguments[(string)$actionParameter] = preg_replace("/".Controller::DASH_ESCAPE."/","-",
												preg_replace("/-/"," ",urldecode($urlArguments[$i])));
					$i++;
				}

				$controller = new $controllerClass($controllerArguments);
				$controller->initialise();
				$controller->execute();
			}else{
				fatal_error(RESOURCE_UNAVAILABLE_ERROR,"The class '$controllerClass' pointed to by '/$destination' does not exist.");
				return;
			}
			
			Janitor::cleanup(true);	
		}
	}
?>