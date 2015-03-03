<?php
	class WadadliCheckbox extends WadadliInputComponent{
		/** @Boolean(required=true, default=false) */
		protected $checked;
	
		protected function getEntryComponent(){
			$entryComponent = new WadadliComponent("{$this->getComponentId()}Checkbox");
			$entryComponent->setTag('input');
			$entryComponent->addAttribute('type','checkbox');
			if($this->getChecked()){
				$entryComponent->addAttribute('checked','checked');
			}
			
			return $entryComponent;
		}
	}
?>