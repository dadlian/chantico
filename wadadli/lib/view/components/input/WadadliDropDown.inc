<?php
	class WadadliDropDown extends WadadliInputComponent{
		/** @Collection(type=@String) */
		protected $options;
		
		public function addOption($name,$value=""){
			if(!$value){
				$value = $name;
			}
			
			$this->insertToOptions($value,(string)$name);
		}
		
		public function clearOptions(){
			$this->setOptions(array());
		}
		
		protected function getEntryComponent(){
			$entryComponent = new WadadliContainer("{$this->getComponentId()}DropDown");
			$entryComponent->setTag('select');
			
			$options = "";
			foreach($this->getOptions() as $optionValue => $optionName){
				$options .= "<option value='$optionValue'";
				if($this->getDefaultValue() == $optionName){
					$options .= " selected='selected'";
				}
				$options .= ">$optionName</option>";
			}
				
			$entryComponent->appendToContents(new WadadliStringComponent($options));
			return $entryComponent;
		}
	}
?>