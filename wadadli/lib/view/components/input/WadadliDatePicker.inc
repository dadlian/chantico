<?php
	class WadadliDatePicker extends WadadliTextField{
		/** @Date */
		protected $minDatetime;
		
		/** @Date */
		protected $maxDatetime;
		
		/** @Boolean(default=true) */
		protected $timeIncluded;
		
		/** @Boolean(default=true) */
		protected $dateIncluded;
		
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
			
			$defaultValue = $this->getDefaultValue()?"{$this->getDefaultValue()}":"null";
			$minDate = ($this->getMinDatetime()->getDatestamp() > '0000-01-01 00:00:00')?"'{$this->getMinDatetime()->getDatestamp()}'":"false";
			$maxDate = ($this->getMaxDatetime()->getDatestamp() > '0000-01-01 00:00:00')?"'{$this->getMaxDatetime()->getDatestamp()}'":"false";
			
			$formatParts = array();
			if($this->isDateIncluded()){
				$formatParts[] = "Y-m-d";
			}
			
			if($this->isTimeIncluded()){
				$formatParts[] = "H:i";
			}
			
			$format = implode(" ",$formatParts);
			$timepicker = $this->isTimeIncluded()?"true":"false";
			$datepicker = $this->isDateIncluded()?"true":"false";
			
			$componentId = camelise($textField->getComponentId());
			
			//Enable DatePicker Dialog
			$this->addPostTagContent("<script type='text/javascript'>addDateInput('$componentId','$defaultValue',$minDate,$maxDate,'$format',$timepicker,$datepicker)</script>");
			
			return $entryComponent;
		}
	}
?>