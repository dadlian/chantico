<?php
	class WadadliTextBox extends WadadliInputComponent{
		/** @String */
		protected $placeholder;
		
		/** @Integer */
		protected $maxLength;
		
		/** @Boolean(default=false) */
		protected $raw;
		
		private $textareaContent;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$this->textareaContent = preg_replace("/\n\s*\n/",Calligrapher::$visualNewline,$this->getDefaultValue());
			$this->setDefaultValue("");
		}
	
		protected function getEntryComponent(){
			$entryComponent = new WadadliContainer("{$this->getComponentId()}TextBox");
			$entryComponent->setTag('textarea');
			$entryComponent->addAttribute('placeholder',$this->getPlaceholder());
			if($this->getMaxLength()){
				$entryComponent->addAttribute('maxlength',strval($this->getMaxLength()));
			}
			
			$entryComponent->appendToContents(new WadadliStringComponent($this->textareaContent));
			
			if(!$this->isRaw()){
				//Enable Tiny Editor
				$this->addPostTagContent("<script type='text/javascript'>addTextEditor('{$entryComponent->getComponentId()}')</script>");
			}
			
			return $entryComponent;
		}
	}
?>