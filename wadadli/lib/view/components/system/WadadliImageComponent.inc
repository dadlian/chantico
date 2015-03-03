<?php
	class WadadliImageComponent extends WadadliComponent{
		/** @File(required=true)  */
		protected $imagePath;
		
		/** @String */
		protected $imageText;
		
		/** @Boolean(default=true) */
		protected $internal;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$this->setTag('img');
			$imagePath = ($this->getInternal()?SettingsManager::getSetting('Install','Url Root'):"")."/{$this->getImagePath()}";
			$imagePath = (preg_match("/^http/",$imagePath)?"":"http://").$imagePath;
			
			$this->addAttribute('src',$imagePath);
			$this->addAttribute('alt',$this->getImageText());
		}
	}
?>