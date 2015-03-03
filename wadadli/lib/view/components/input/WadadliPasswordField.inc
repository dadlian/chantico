<?php
	class WadadliPasswordField extends WadadliTextField{
		/** @Boolean(required=true, default=false) */
		protected $autocomplete;
		
		/** @Boolean(required=true, default=false) */
		protected $disabled;
		
		protected function getEntryComponent(){
			$entryComponent = parent::getEntryComponent();
			$entryComponent->addAttribute('type','password');
			
			if(!$this->isAutocomplete()){
				$entryComponent->addAttribute('autocomplete','off');
			}
			
			if($this->isDisabled()){
				$entryComponent->addAttribute('disabled','disabled');
			}
			
			return $entryComponent;
		}
	}
?>