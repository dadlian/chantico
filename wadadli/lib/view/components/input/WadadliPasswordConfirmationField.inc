<?php
	class WadadliPasswordConfirmationField extends WadadliInputComponent{
		private $innerTitle;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$this->innerTitle = $this->getTitle();
			$this->setTitle("");
		}
		
		protected function getEntryComponent(){
			$entryComponent = new WadadliContainer("{$this->getComponentId()}ConfirmationField");
			
			if($this->getDefaultValue()){
				$toggleLink = new WadadliAnchor("","+ Change Password");
				$toggleLink->onClick('showPasswordFields');
				$entryComponent->appendToContents($toggleLink);
			}
			
			$passwordField = new WadadliPasswordField($this->innerTitle,"",$this->getRequired());
			$confirmPasswordField = new WadadliPasswordField("Confirm {$this->innerTitle}","",$this->getRequired());
			
			if($this->getDefaultValue()){
				$passwordField->addClass('hidden');
				$passwordField->setDisabled(true);
				$confirmPasswordField->addClass('hidden');
				$confirmPasswordField->setDisabled(true);
			}
			
			$entryComponent->appendToContents($passwordField);
			$entryComponent->appendToContents($confirmPasswordField);
			
			return $entryComponent;
		}
	}
?>