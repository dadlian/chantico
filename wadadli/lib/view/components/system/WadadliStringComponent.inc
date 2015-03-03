<?php
	class WadadliStringComponent extends WadadliComponent{
		/** @Text */
		protected $content;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),array_merge(array(""),func_get_args()));
			$this->setTag('');
		}
		
		protected function renderComponent(){
			return $this->getContent();
		}
	}
?>