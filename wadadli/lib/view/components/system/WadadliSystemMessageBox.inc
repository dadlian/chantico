<?php
	class WadadliSystemMessageBox extends WadadliComponent{
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			
			if(!Postman::hasMessages()){
				$this->addClass('hidden');
			}
		}
		
		protected function renderComponent(){
			$hasErrors = false;
			$hasWarnings = false;
			$hasDebugs = false;
			$hasSuccesses = false;
		
			$debugLevel = SettingsManager::getSetting('Logging','Level');
			$renderedComponent = "";
		
			$errors = Postman::deliverErrors();
			$errorList = new WadadliListComponent("{$this->getComponentId()}ErrorList",array(),false);
			$errorList->addClass('errorList');
			foreach($errors as $error){
				if($error->getText()){
					$hasErrors = true;
					$errorList->addItem($error->getText());
				}
			}
			
			if($hasErrors){
				$renderedComponent .= $errorList->render(false);
			}
			
			$warnings = Postman::deliverWarnings();
			$warningList = new WadadliListComponent("{$this->getComponentId()}WarningList",array(),false);
			$warningList->addClass('warningList');
			foreach($warnings as $warning){
				if($warning->getText()){
					$hasWarnings = true;
					$warningList->addItem($warning->getText());
				}
			}
			
			if($hasWarnings && $debugLevel != "error" ){
				$renderedComponent .= $warningList->render(false);
			}
			
			$debugs = Postman::deliverDebugs();
			$debugList = new WadadliListComponent("{$this->getComponentId()}DebugList",array(),false);
			$debugList->addClass('debugList');
			foreach($debugs as $debug){
				if($debug->getText()){
					$hasDebugs = true;
					$debugList->addItem($debug->getText());
				}
			}
			
			if($hasDebugs && ($debugLevel != "error" || $debugLevel != "warning")){
				$renderedComponent .= $debugList->render(false);
			}
			
			$successes = Postman::deliverSuccesses();
			$successList = new WadadliListComponent("{$this->getComponentId()}SuccessList",array(),false);
			$successList->addClass('successList');
			foreach($successes as $success){
				if($success->getText()){
					$hasSuccesses = true;
					$successList->addItem($success->getText());
				}
			}
			
			if($hasSuccesses){
				$renderedComponent .= $successList->render(false);
			}
			
			return $renderedComponent;
		}
	}
?>