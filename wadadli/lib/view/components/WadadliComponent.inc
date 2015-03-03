<?php
	class WadadliComponent extends PersistentClass{
		/** @String */
		protected $componentId;
		
		private $eventHandlers;
		private $classes;
		private $attributes;
		private $tag;
		private $postTagContent;
		private $selfClosing;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			
			$this->classes = array();
			$this->eventHandlers = array();
			$this->tag = "div";
			
			$this->eventHandlers = array();
			$this->classes = array();
			$this->attributes = array();
			$this->postTagContent = "";
			$this->selfClosing = false;
			
			$visibleClass = false;
			foreach(Mirror::reflectClass($this)->getClassHierarchy() as $class){
				if($visibleClass){
					$this->addClass(decapitalise($class->getName()));
				}
				
				if($class->getName() == "WadadliComponent"){
					$visibleClass = true;
				}
			}
		}
		
		/*
		 * Method is invoked when an undefined method is called on this class
		 */
		public function __call($method, $arguments){
			if(preg_match("/^on[A-Z]\w*/", $method, $matches)){
				//Create a dummy argument if none was provided
				$arguments = array_pad($arguments,2,null);
				
				if(!is_string($arguments[0])){
					fatal_error(UNEXPECTED_ARGUMENT_WARNING,
							"WadadliComponent $method expects string action, ".gettype($arguments[0])." given.");
					return;
				}
				
				if(is_null($arguments[1])){
					$arguments[1] = array();
				}else if(!is_array($arguments[1])){
					fatal_error(UNEXPECTED_ARGUMENT_WARNING,
							"WadadliComponent $method expects argument array, ".gettype($arguments[1])." given.");
					return;
				}
				
				//Initialise parameters for method call
				$event = strtolower($matches[0]);
				
				if(!array_key_exists($event,$this->eventHandlers)){
					$this->eventHandlers[$event] = array();
				}
				
				$this->eventHandlers[$event][] = array($arguments[0],$arguments[1]);
			}else{
				return parent::__call($method,$arguments);
			}
		}
		
		public function setTag($newTag){
			if(!is_string($newTag)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliComponent addTag expects string argument, ".gettype($newTag)." given");
				return;
			}
			
			$this->tag = $newTag;
		}
		
		public function addClass($class){
			if(!is_string($class)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliComponent addClass expects string argument, ".gettype($class)." given.");
				return;
			}
			
			$this->classes[$class] = $class;
		}
		
		public function removeClasses(){
			$this->classes = array();
		}
		
		public function getClasses(){
			return $this->classes;
		}
		
		public function addAttribute($attribute, $value){
			if(!is_string($attribute)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliComponent addAttribute expects string attribute name, ".gettype($attribute)." given.");
				return;
			}
			
			if(!is_string($value)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliComponent addAttribute expects string attribute value, ".gettype($value)." given.");
				return;
			}
			
			$this->attributes[$attribute] = $value;
		}
		
		public function removeAttribute($attribute){
			unset($this->attributes[$attribute]);
		}
		
		public function removeAttributes(){
			$this->attributes = array();
		}
		
		public function getAttribute($attributeName){
			if(array_key_exists($attributeName, $this->attributes)){
				return $this->attributes[$attributeName];
			}
		}
		
		public function loadView($event,$action,$target="",$arguments=array(),$isDynamic=false,$callback="",$callbackArg=""){
			if(!$target){
				$target = $this->getComponentId();
			}
			
			if(!is_string($event)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliComponent loadView expects string event, ".gettype($event)." given.");
				return;
			}
			
			if(!is_string($action)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliComponent laodView expects string action, ".gettype($action)." given.");
				return;
			}
			
			if(!is_string($target)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,
						"WadadliComponent laodView expects string target name, ".gettype($target)." given.");
				return;
			}
			
			if($arguments && !is_array($arguments)){
				$arguments = array($arguments);
			}
			
			foreach($arguments as $argument){
				if(!is_string($argument)){
					fatal_error(UNEXPECTED_ARGUMENT_WARNING,
							"WadadliComponent laodView expects string action arguments, ".gettype($argument)." given.");
					return;
				}
			}
		
			$action = ActionManager::getAction($action);
			if(!$action){
				return;
			}
			
			$eventSetter = "on".camelise($event,true);
			$actionUrl = SettingsManager::getSetting('Install','Url Root')."/".$action->getPath();
			$actionUrl = (preg_match("/^http/",$actionUrl)?"":"http://").$actionUrl;
			
			$this->$eventSetter('loadView',array_merge(array($actionUrl,$target,$isDynamic,$callback,$callbackArg),$arguments));
		}
		
		public function addPostTagContent($content){
			if(is_string($content)){
				$this->postTagContent .= $content;
			}
		}
		
		public function setSelfClosing(){
			$this->selfClosing = true;
		}
		
		public function render($output=true){
			$idString = "";
			if($this->getComponentId()){
				$idString = " id='".str_replace(" ","",$this->getComponentId())."'";
			}
			
			$classString = "";
			if($this->classes){
				$classString = " class='".implode(" ",$this->classes)."'";
			}
			
			$eventHandlerStringParts = array();
			if($this->eventHandlers){
				foreach($this->eventHandlers as $event => $handlers){
					$eventHandlerString = "$event='";
					foreach($handlers as $handler){
						$function = $handler[0];
						
						$arguments = array();
						foreach($handler[1] as $argument){
							$arguments[] = preg_replace("/['\"]/","\'",$argument);
						}
						$arguments = implode("\",\"",$arguments);
						
						if($arguments != ""){
							$arguments = "\"$arguments\",this";
						}else{
							$arguments = "this";
						}
						
						$eventHandlerString .= "$function($arguments);";
					}
					$eventHandlerString .= "'";
					
					$eventHandlerStringParts[] = $eventHandlerString;
				}
			}
			$eventHandlerString = implode(" ",$eventHandlerStringParts);
			
			$attributeStringParts = array();
			
			if($this->attributes){
				foreach($this->attributes as $key => $value){
					$attributeStringParts[] = "$key='$value'";
				}
			}
			$attributeString = implode(" ",$attributeStringParts);
		
			$renderedComponent = "";
			
			if($this->tag){
				$renderedComponent .= "<{$this->tag}{$idString}{$classString} $eventHandlerString $attributeString>";
			}
			$renderedComponent .= $this->renderComponent();
			if($this->tag && !$this->selfClosing){
				$renderedComponent .= "</{$this->tag}>";
			}
			$renderedComponent .= $this->postTagContent;
			
			$formattedComponent = str_replace(Calligrapher::$visualNewline,"&#10",Calligrapher::formatMarkup($renderedComponent));
			if($output){
				echo $formattedComponent;
			}else{
				return $formattedComponent;
			}
		}
		
		protected function renderComponent(){
			return "";
		}
	}
?>