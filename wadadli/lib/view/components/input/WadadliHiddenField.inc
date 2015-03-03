<?php
	class WadadliHiddenField extends WadadliTextField{
		private $fieldName;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			
			$this->fieldName = $this->getTitle();
			$this->setTitle("");
			
			$this->addClass('hidden');
		}
	
		protected function getEntryComponent(){
			$entryComponent = parent::getEntryComponent();
			$entryComponent->addAttribute('type','hidden');
			
			$entryComponent->addAttribute('name',decapitalise(str_replace(" ","",$this->fieldName)));
			
			return $entryComponent;
		}
	}
?>