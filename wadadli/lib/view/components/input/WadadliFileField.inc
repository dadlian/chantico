<?php
	class WadadliFileField extends WadadliInputComponent{
		protected function getEntryComponent(){
			$entryComponent = new WadadliContainer("{$this->getComponentId()}FileField");
			
			$fileInput = new WadadliComponent();
			$fileInput->setTag('input');
			$fileInput->addAttribute('type','file');
			$fileInput->addAttribute('name',camelise($this->getComponentId()));
			$entryComponent->addContent($fileInput);
			
			if($this->getDefaultValue()){
				$filename = new WadadliContainer();
				$filename->setTag('span');
				$filename->addContent(new WadadliStringComponent($this->getDefaultValue()));
				$entryComponent->addContent($filename);
			}
			
			return $entryComponent;
		}
	}
?>