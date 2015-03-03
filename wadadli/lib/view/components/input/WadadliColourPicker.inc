<?php
	class WadadliColourPicker extends WadadliTextField{
		private $postName;
		
		public function changePostName($newPostName){
			$this->postName = (string)$newPostName;
		}
	
		protected function getEntryComponent(){
			$validLabel = ($this->postName || $this->postName == "0");
			$componentTitle = $validLabel?$this->postName:decapitalise(str_replace(" ","",$this->getTitle()));
			
			$entryComponent = new WadadliContainer();
			$textField = parent::getEntryComponent();
			$textField->addAttribute('name',$componentTitle);
			$textField->addAttribute('value',$this->getDefaultValue());
			$entryComponent->addContent($textField);
						
			$componentId = camelise($textField->getComponentId());
			
			//Enable Colour Dialog
			$this->addPostTagContent("<script type='text/javascript'>addColourInput('$componentId','{$this->getDefaultValue()}')</script>");
			return $entryComponent;
		}
	}
?>