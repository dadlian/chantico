<?php
	class WadadliActionLink extends WadadliAnchor{
		/** @Boolean(default=false) */
		protected $alwaysShow;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			
			$actionLink = "";
			$action = ActionManager::getAction($this->getLink());
			
			if($action){
				$allowedRoles = $action->getRoles();
				if($this->getAlwaysShow() || !$allowedRoles || 
					(SessionManager::activeUser() && in_array(SessionManager::activeUser()->getRole(),$allowedRoles))){
					
					if($action->getPath()){
						$this->setLink($action->getPath());
					}else{
						$this->setInternal(false);
						$this->setLink(SettingsManager::getSetting('Install','Url Root'));
					}
				}else{
					$this->setClickable(new WadadliStringComponent());
					$this->setLink("");
					$this->setTarget("");
				}
			}else{
				$this->setLink("");
			}
		}
		
		public function addArgument($argument){
			if(!is_string($argument)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliActionLink addArgument expects string argument value, ".gettype($argument)." given.");
				return;
			}
			
			$this->setLink(preg_replace("/@/",urlencode(preg_replace("/\s/","-",preg_replace("/-/",Controller::DASH_ESCAPE,strtolower($argument)))),$this->getLink(),1));
		}
		
		public function addWildcard($wildcard){
			if(!is_string($wildcard)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliActionLink addWildcard expects string wildcard value, ".gettype($wildcard)." given.");
				return;
			}
			
			$this->setLink(preg_replace("/\*/",$wildcard,$this->getLink(),1));
		}
	}
?>