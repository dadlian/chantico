<?php
	class WadadliListComponent extends WadadliComponent{
		/** @Collection(type=@Object(class='WadadliContainer')) */
		protected $listItems;
		
		/** @Boolean(required=true, default=false) */
		protected $ordered;
		
		public function __construct(){
			$arguments = func_get_args();
			call_user_func_array(array('parent','__construct'),$arguments);
			
			if($this->getOrdered()){
				$this->setTag('ol');
			}else{
				$this->setTag('ul');
			}
			
			if(!$this->getListItems()){
				$this->setListItems(array());
			}
		}
		
		public function addItem($listItem){
			$listItemContainer = new WadadliContainer("{$this->getComponentId()}Item".sizeof($this->getListItems()));
			$listItemContainer->setTag('li');
			
			if(is_string($listItem)){
				$listItem = new WadadliStringComponent($listItem);
			}
			
			$listItemContainer->appendToContents($listItem);
			$this->appendToListItems($listItemContainer);
		}
		
		public function addItems($listItems){
			if(!is_array($listItems)){
				$listItems = array($listItems);
			}
			
			foreach($listItems as $listItem){
				$this->addItem($listItem);
			}
		}
		
		public function __call($method, $arguments){
			if(preg_match("/^onItem([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$listSize = sizeof($this->getListItems());
				$arguments = array_pad($arguments,3,array());
				
				$itemIndicies = $this->padIndicies($arguments[2]);
				
				foreach($itemIndicies as $index){
					if($index >= 0 && $index < $listSize){
						$this->viewFromListItems($index)->$event($arguments[0],$arguments[1]);
					}
				}
			}else{
				return parent::__call($method,$arguments);
			}
		}
		
		public function addItemClass($class,$itemIndicies=array()){
			$listSize = sizeof($this->getListItems());
			$itemIndicies = $this->padIndicies($itemIndicies);
			
			foreach($itemIndicies as $index){
				if($index >= 0 && $index < $listSize){
					$this->viewFromListItems($index)->addClass($class);
				}
			}
		}
		
		public function removeItemClasses($itemIndicies=array()){
			$listSize = sizeof($this->getListItems());
			$itemIndicies = $this->padIndicies($itemIndicies);
			
			foreach($itemIndicies as $index){
				if($index >= 0 && $index < $listSize){
					$this->viewFromListItems($index)->removeClasses();
				}
			}
		}
		
		public function addItemAttribute($name,$attribute,$itemIndicies=array()){
			$listSize = sizeof($this->getListItems());
			$itemIndicies = $this->padIndicies($itemIndicies);
			
			foreach($itemIndicies as $index){
				if($index >= 0 && $index < $listSize){
					$this->viewFromListItems($index)->addAttribute($name,$attribute);
				}
			}
		}
		
		public function removeItemAttributes($itemIndicies=array()){
			$listSize = sizeof($this->getListItems());
			$itemIndicies = $this->padIndicies($itemIndicies);
			
			foreach($itemIndicies as $index){
				if($index >= 0 && $index < $listSize){
					$this->viewFromListItems($index)->removeAttributes();
				}
			}
		}
		
		public function renderComponent(){
			$renderedComponent = "";
			
			foreach($this->getListItems() as $listItem){
				$renderedComponent .= $listItem->render(false);
			}
			
			return $renderedComponent;
		}
		
		private function padIndicies($itemIndicies){
			$listSize = sizeof($this->getListItems());
			
			if(!$itemIndicies){
				$itemIndicies = range(0,$listSize);
			}
			
			$newIndicies = array();
			foreach($itemIndicies as $index){
				if(is_int($index)){
					$newIndicies[] = $index;
				}
			}
			
			return $newIndicies;
		}
	}
?>