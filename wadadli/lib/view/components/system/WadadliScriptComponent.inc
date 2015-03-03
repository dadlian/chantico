<?php
	class WadadliScriptComponent extends WadadliComponent{
		/** @URL(required=true)  */
		protected $scriptPath;
		
		/** @String(required=true, default='text/javascript') */
		protected $type;
		
		/** @Boolean(required=true, default=true) */
		protected $internal;
		
		/** @String */
		protected $inlineScript;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),array_merge(array(""),func_get_args()));
			$this->setTag('script');
			$this->removeClasses();
			
			if($this->getScriptPath()){
				$urlRoot = $this->getInternal()?SettingsManager::getSetting('Install','Url Root')."/":"";
				$path = $urlRoot.$this->getScriptPath();
				
				$this->addAttribute('src',(preg_match("/^http/",$path)?"":"http://").$path);
			}
			
			$this->addAttribute('type',$this->getType());
		}
		
		protected function renderComponent(){
			$scriptWrapper = new WadadliStringComponent($this->getInlineScript());
			return $scriptWrapper->render(false);
		}
	}
?>