<?php
	class WadadliPageSelect extends WadadliContainer{
		/** @Integer(required=true, default=1) */
		protected $pages;
		
		/** @String(required=true, default="") */
		protected $action;
		
		/** @Collection(type=@String) */
		protected $arguments;
	
		public function __construct($componentId, $pages, $action, $arguments=array(),$alphabetical=false){
			parent::__construct($componentId, array(), $pages, $action, $arguments);
			
			$title = new WadadliContainer();
			$title->setTag('span');
			$title->addContent("Page:");
			$this->addContent($title);
			
			$pageMenu = new WadadliMenu("",array(),array(),true);
			
			if($alphabetical){
				for($i = 65; $i <= 90; $i++){
					$pageMenu->addAction(chr($i), $this->getAction(), array_merge($this->getArguments(), array(chr($i))));
				}
				$pageMenu->addAction("#",$this->getAction(),array_merge($this->getArguments(),array("09")));
				$pageMenu->addAction("All",$this->getAction(),array_merge($this->getArguments(),array("All")));
			}else{
				for($page=1; $page <= $this->getPages(); $page++){
					$pageMenu->addAction((string)$page, $this->getAction(), array_merge($this->getArguments(), array((string)$page)));
				}
			}
			
			$this->addContent($pageMenu);
		}
		
		public function asynchronise($target,$callback){
			$pageMenu = $this->takeFromContents(1);
			$pageMenu->asynchronise($target,$callback);
			$this->addContent($pageMenu);
		}
	}
?>