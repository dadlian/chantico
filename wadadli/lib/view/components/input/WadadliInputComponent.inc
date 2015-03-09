<?php
	abstract class WadadliInputComponent extends WadadliComponent{
		/** @String(required=true) */
		protected $title;
		
		/** @String */
		protected $defaultValue;
		
		/** @Boolean(required=true, default=false) */
		protected $required;
		
		private $events;
		private $classes;
		private $attributes;
		private $showLabel;
		private $postName;
		
		abstract protected function getEntryComponent();
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),array_merge(array(""),func_get_args()));
			
			$this->setComponentId(preg_replace("/\./","",decapitalise($this->getTitle())));
			$this->setDefaultValue(str_replace("'","&#39;",$this->getDefaultValue()));
			$this->events = array('label'=>array(),'entry'=>array());
			$this->classes = array('label'=>array(),'entry'=>array());
			$this->attributes = array('label'=>array(),'entry'=>array());
			$this->showLabel = true;
		}
		
		public function toggleLabel(){
			$this->showLabel = !$this->showLabel;
		}
		
		//Make the input component value something different from the title
		public function changePostName($newPostName){
			$this->postName = (string)$newPostName;
		}
		
		protected function renderComponent(){
			$renderedComponent = "";
			
			$validLabel = ($this->postName || $this->postName == "0");
			$componentTitle = $validLabel?$this->postName:decapitalise(preg_replace("/[\.\s]/","",$this->getTitle()));
			
			if(($componentTitle || $validLabel)  && $this->showLabel){
				$label = new WadadliContainer($this->getComponentId()."Label");
				$label->setTag('label');
				$label->addAttribute('for',$componentTitle);
				
				foreach($this->events['label'] as $event => $parameters){
					$label->$event($parameters[0],$parameters[1]);
				}
				
				foreach($this->classes['label'] as $class){
					$label->addClass($class);
				}
				
				foreach($this->attributes['label'] as $attribute => $value){
					$label->addAttribute($attribute,$value);
				}
				
				$label->appendToContents(new WadadliStringComponent($this->getTitle().(($this->getRequired())?" *":"")));
				$renderedComponent .= $label->render(false);
			}
			
			$entryComponent = $this->getEntryComponent();
			if($componentTitle){
				$entryComponent->addAttribute('name',decapitalise($componentTitle));
			}
			
			$entryComponent->addAttribute('value',$this->getDefaultValue());
			
			foreach($this->events['entry'] as $event => $parameters){
				$entryComponent->$event($parameters[0],$parameters[1]);
			}
			
			foreach($this->classes['entry'] as $class){
				$entryComponent->addClass($class);
			}
			
			foreach($this->attributes['entry'] as $attribute => $value){
				$entryComponent->addAttribute($attribute,$value);
			}
			
			$renderedComponent .= $entryComponent->render(false);
			
			return $renderedComponent;
		}
		
		public function __call($method, $arguments){
			if(preg_match("/^onLabel([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,2,array());
				
				$this->events['label'][$event] = array($arguments[0],$arguments[1]);
			}else if(preg_match("/^onEntry([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,2,array());
				
				$this->events['entry'][$event] = array($arguments[0],$arguments[1]);
			}else{
				return parent::__call($method,$arguments);
			}
		}
		
		public function addLabelClass($class){
			$this->classes['label'][] = $class;
		}
		
		public function removeLabelClasses(){
			$this->classes['label'] = array();
		}
		
		public function addLabelAttribute($attribute,$value){
			$this->attributes['label'][$attribute] = $value;
		}
		
		public function removeLabelAttributes(){
			$this->attributes['label'] = array();
		}
		
		public function addEntryClass($class){
			$this->classes['entry'][] = $class;
		}
		
		public function removeEntryClasses(){
			$this->classes['entry'] = array();
		}
		
		public function addEntryAttribute($attribute,$value){
			$this->attributes['entry'][$attribute] = $value;
		}
		
		public function removeEntryAttributes(){
			$this->attributes['entry'] = array();
		}
	}
?>