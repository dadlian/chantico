<?php
	class WadadliMenu extends WadadliComponent{
		/** @Collection(type=@String) */
		protected $actions;
		
		/** @Collection(type=@Collection(type=@String)) */
		protected $arguments;
		
		/** @Boolean(required=true,default=false) */
		protected $defaultSelected;
		
		private $actionEvents;
		private $actionClasses;
		private $actionAttributes;
		private $asynchronised;
		private $asyncTarget;
		private $asyncCallback;
		private $asyncCallbackArg;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$this->actionEvents = array();
			$this->actionClasses = array();
			$this->actionAttributes = array();
			$this->asynchronised = false;
			$this->asyncTarget = "";
			$this->asyncCallback = "";
			$this->asyncCallbackArg = "";
		}
		
		public function addAction($name, $action, $arguments=array()){
			$this->insertToActions($name, $action);
			$this->insertToArguments($name, $arguments);
		}
		
		public function asynchronise($target,$callback="",$callbackArg=""){
			$this->asynchronised = true;
			$this->asyncTarget = $target;
			$this->asyncCallback = $callback;
			$this->asyncCallbackArg = $callbackArg;
		}
		
		public function renderComponent(){
			$renderedComponent = "";
			$index = 0;
			
			$actionLinks = array();
			$activeLink = null;
			foreach($this->getActions() as $name => $action){
				$actionObject = ActionManager::getAction($action);
				
				if($actionObject){
					$actionLink = new WadadliActionLink("{$this->getComponentId()}Action$index",(string)$name,$actionObject->getName());
					
					$actionArguments = ($this->viewFromArguments($name))?$this->viewFromArguments($name):array();
					foreach($actionArguments as $argumentValue){
						$actionLink->addArgument($argumentValue);
					}
				}else{
					$actionLink = new WadadliAnchor("{$this->getComponentId()}Action$index",(string)$name,$action,false);
				}
				
				foreach($this->actionEvents as $event => $parameters){
					$actionLink->$event($parameters[0],$parameters[1]);
				}
				
				foreach($this->actionClasses as $class){
					$actionLink->addClass($class);
				}
				
				foreach($this->actionAttributes as $attribute => $value){
					$actionLink->addAttribute($attribute,$value);
				}
				
				if($this->asynchronised){
					$actionLink->asynchronise($this->asyncTarget,$this->asyncCallback,$this->asyncCallbackArg);
				}
				
				$actionLinks[] = $actionLink;
				
				if(in_array('selected',$actionLink->getClasses())){
					$activeLink = $actionLink;
				}
			}
			
			//Selected the first link by default if no other is selected
			if($this->isDefaultSelected() && !$activeLink && $actionLinks){
				$actionLinks[0]->addClass('selected');
			}
			
			foreach($actionLinks as $actionLink){
				
				$renderedComponent .= $actionLink->render(false);
				
				$index++;
			}
			
			return $renderedComponent;
		}
		
		public function __call($method, $arguments){
			if(preg_match("/^onAction([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,2,array());
				
				$this->actionEvents[$event] = array($arguments[0],$arguments[1]);
			}else{
				return parent::__call($method,$arguments);
			}
		}
		
		public function addActionClass($class){
			$this->actionClasses[] = $class;
		}
		
		public function removeActionClasses(){
			$this->actionClasses = array();
		}
		
		public function addActionAttribute($attribute,$value){
			$this->actionAttributes[$attribute] = $value;
		}
		
		public function removeActionAttributes(){
			$this->actionAttributes = array();
		}
	}
?>