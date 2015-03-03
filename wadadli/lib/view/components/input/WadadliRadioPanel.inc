<?php
	class WadadliRadioPanel extends WadadliInputComponent{
		private $options;
		
		private $radioPanelTitle;
		
		protected function setTitle($newTitle){
			$this->radioPanelTitle = $newTitle;
		}
		
		public function addOption($name){
			$this->options[] = $name;
		}
		
		public function clearOptions(){
			$this->options = array();
		}
		
		protected function getEntryComponent(){
			$componentTitle = decapitalise(str_replace(" ","",$this->radioPanelTitle));
			$entryComponent = new WadadliContainer("{$this->getComponentId()}RadioPanel");
			
			$counter = 0;
			foreach($this->options as $optionName){
				$radioButton = new WadadliContainer("{$this->getComponentId()}RadioButton$counter");
				$radioButton->addClass("radioButton");
				
				$button = new WadadliComponent();
				$button->setTag('input');
				$button->addAttribute('type', 'radio');
				$button->addAttribute('name', $componentTitle);
				$button->addAttribute('value', $optionName);
				
				if($this->getDefaultValue() == $optionName || (!$this->getDefaultValue() && $counter==0)){
					$button->addAttribute('checked','checked');
				}
				
				$radioButton->appendToContents($button);
				
				$label = new WadadliContainer();
				$label->setTag('label');
				$label->addAttribute('for',$componentTitle);
				$label->appendToContents(new WadadliStringComponent($optionName));
				$radioButton->appendToContents($label);
				
				$entryComponent->appendToContents($radioButton);
				$counter++;
			}
			
			$stretcher = new WadadliContainer();
			$stretcher->addClass("stretcher");
			$entryComponent->appendToContents($stretcher);
			
			return $entryComponent;
		}
	}
?>