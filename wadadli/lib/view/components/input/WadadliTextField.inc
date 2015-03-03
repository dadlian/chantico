<?php
	class WadadliTextField extends WadadliInputComponent{
		/** @String */
		protected $placeholder;
	
		protected function getEntryComponent(){
			$entryComponent = new WadadliComponent("{$this->getComponentId()}TextField");
			$entryComponent->setTag('input');
			$entryComponent->addAttribute('type','text');
			$entryComponent->addAttribute('placeholder',$this->getPlaceholder());
			
			return $entryComponent;
		}
	}
?>