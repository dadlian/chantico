<?php
	class WadadliAnchor extends WadadliComponent{
		/** @Object(required=true, class='WadadliComponent') */
		protected $clickable;
		
		/** @URL(required=true) */
		protected $link;
		
		/** @Boolean(required=true, default=true) */
		protected $internal;
		
		/** @String(required=true, default='_self') */
		protected $target;
		
		public function __construct(){
			$arguments = func_get_args();
			if(sizeof($arguments) > 1 && is_string($arguments[1])){
				$arguments[1] = new WadadliStringComponent($arguments[1]);
			}
		
			call_user_func_array(array('parent','__construct'),$arguments);
			$this->setTag('a');
			
			$this->setLink($this->getLink());
			$this->addAttribute('target',$this->getTarget());
		}
		
		public function setLink($newLink){
			$urlRoot = $this->getInternal()?SettingsManager::getSetting('Install','Url Root')."/":"";
			$this->link = $newLink;
			
			$currentLink = "http://$_SERVER[HTTP_HOST]$_SERVER[REQUEST_URI]";
			$destinationLink = "";
			if($newLink){
				$destinationLink = "$urlRoot$newLink";
				$destinationLink = (preg_match("/^http/",$destinationLink)?"":"http://").$destinationLink;
			}
			
			if($currentLink == $destinationLink){
				$this->addClass('selected');
			}
			
			if($destinationLink){
				$this->addAttribute('href',$destinationLink);
			}
		}
		
		public function asynchronise($target,$callback="",$callbackArg=""){
			$this->onClick("loadView",array($this->getAttribute("href"),$target,false,$callback,$callbackArg));
			$this->removeAttribute("href");
		}
		
		protected function renderComponent(){
			return $this->getClickable()->render(false);
		}
	}
?>