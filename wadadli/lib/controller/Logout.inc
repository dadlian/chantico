<?php
	class Logout extends Controller{
		public function execute(){
			if(SessionManager::activeUser()){
				$username = SessionManager::activeUser()->getUsername();
				SessionManager::reset();
				Postman::postSuccess("You have successfully signed out of $username");
			}
			
			$redirect = "Location:http://".SettingsManager::getSetting('install','urlRoot');
			if($this->viewFromArguments("onLogout")){
				$onLogout = ActionManager::getAction(decamelise($this->viewFromArguments("onLogout"),true));
				$redirect .= "/{$onLogout->getPath()}";
			}
			
			header($redirect);
		}
	}
?>