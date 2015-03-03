<?php
	class APIError extends Controller{
		public function execute(){
			$message = array();
			if(SettingsManager::getSetting("logging","level") == "debug"){
				foreach(Postman::deliverDebugs() as $debug){
					$messages[] = $debug->getText();
				}
			}else{
				foreach(Postman::deliverErrors() as $error){
					$messages[] = $error->getText();
				}
			}
			
			ResponseHandler::error($messages);
		}
	}
?>