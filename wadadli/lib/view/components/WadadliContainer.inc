<?php
	class WadadliContainer extends WadadliComponent{
		/** @Collection(type=@Object(class='WadadliComponent')) */
		protected $contents;
		
		public function __construct(){
			$arguments = func_get_args();
			
			if(sizeof($arguments) <= 0){
				$arguments[0] = "";
			}
			
			if(sizeof($arguments) <= 1){
				$arguments[1] = array();
			}
			
			if(!is_array($arguments[1])){
				$arguments[1] = array($arguments[1]);
			}
			
			$components = array();
			for($i = 0; $i < sizeof($arguments[1]); $i++){
				if(is_string($arguments[1][$i])){
					$arguments[1][$i] = new WadadliStringComponent($arguments[1][$i]);
				}
				
				$components[] = $arguments[1][$i];
			}
		
			$arguments[1] = $components;
			call_user_func_array(array('parent','__construct'),$arguments);
		}
		
		public function addContent($content){
			if(is_string($content)){
				$content = new WadadliStringComponent($content);
			}
			
			$this->appendToContents($content);
		}
		
		protected function renderComponent(){
			$renderedComponent = "";
			foreach($this->getContents() as $content){
				if($content){
					$renderedComponent .= $content->render(false);
				}
			}
			
			return $renderedComponent;
		}
	}
?>