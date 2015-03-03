<?php
	class View extends WadadliComponent{
		private $zones = array();
		private $zoneLoadActions = array();
		private $zoneLoadActionArguments = array();
		
		public function addZone($zoneName, $class=""){
			if(!is_string($zoneName)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View addZone expects string zone name, ".gettype($zoneName)." given.");
				return;
			}
			
			if(!array_key_exists($zoneName, $this->zones)){
				$zone = new WadadliContainer($zoneName);
				if($class){
					$zone->addClass($class);
				}
				$this->zones[$zoneName] = $zone;
			}
		}
		
		public function removeZone($zoneName){
			if(!is_string($zoneName)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View removeZone expects string zone name, ".gettype($zoneName)." given.");
				return;
			}
			
			if(array_key_exists($zoneName, $this->zones)){
				unset($this->zones[$zoneName]);
			}
		}
		
		public function addToZone($zoneName, $component){
			if(is_string($component)){
				$component = new WadadliStringComponent($component);
			}
		
			$this->addZone($zoneName);
			$this->zones[$zoneName]->appendToContents($component);
		}
		
		public function addToZoneOnLoad($zoneName, $action, $actionArguments=array()){
			if(!is_string($zoneName)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View addToZoneOnLoad expects string zone name, ".gettype($zoneName)." given.");
				return;
			}
			
			if(!array_key_exists($zoneName, $this->getZones())){
				$this->addZone($zoneName);
			}
			
			if(!is_string($action)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View addToZoneOnLoad expects string action, ".gettype($action)." given.");
				return;
			}
			
			if($actionArguments && !is_array($actionArguments)){
				$actionArguments = array($actionArguments);
			}
			
			foreach($actionArguments as $actionArgument){
				if(!is_string($actionArgument)){
					fatal_error(UNEXPECTED_ARGUMENT_WARNING,
							"WadadliComponent laodView expects string action arguments, ".gettype($actionArgument)." given.");
					return;
				}
			}
			
			$this->zoneLoadActions[$zoneName] = $action;
			$this->zoneLoadActionArguments[$zoneName] = $actionArguments;
		}
		
		public function clearZone($zoneName){
			if(!is_string($zoneName)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View clearZone expects string zone name, ".gettype($zoneName)." given.");
				return;
			}
			
			if(array_key_exists($zoneName, $this->zones)){
				$this->zones[$zoneName]->setContents(array());
				unset($this->zoneLoadActions[$zoneName]);
				unset($this->zoneLoadActionArguments[$zoneName]);
			}
		}
		
		public function getZones(){
			return array_keys($this->zones);
		}
		
		public function getZoneLoadAction($zoneName){
			if(!is_string($zoneName)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View getZoneLoadAction expects string zone name, ".gettype($zoneName)." given.");
				return;
			}
			
			$zoneLoadAction = "";
			if(array_key_exists($zoneName, $this->zoneLoadActions)){
				$zoneLoadAction = $this->zoneLoadActions[$zoneName];
			}
			
			return $zoneLoadAction;
		}
		
		public function getZoneLoadActionArguments($zoneName){
			if(!is_string($zoneName)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"View getZoneLoadActionArguments expects string zone name, ".gettype($zoneName)." given.");
				return;
			}
			
			$zoneLoadActionArguments = "";
			if(array_key_exists($zoneName, $this->zoneLoadActionArguments)){
				$zoneLoadActionArguments = $this->zoneLoadActionArguments[$zoneName];
			}
			
			return $zoneLoadActionArguments;
		}
		
		protected function renderComponent(){
			$renderedComponent = "";
			foreach($this->zones as $zone){
				$renderedComponent .= $zone->render(false);
			}
			
			$stretcher = new WadadliComponent();
			$stretcher->addClass('stretcher');
			$renderedComponent .= $stretcher->render(false);
			
			return Calligrapher::formatMarkup($renderedComponent);
		}
	}
?>