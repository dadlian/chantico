<?php
	class WadadliListing extends WadadliComponent{
		/** @String */
		protected $title;
		
		private $listItems;
		private $itemEvents;
		private $itemClasses;
		private $itemAttributes;
		
		public function __construct(){
			$arguments = array_pad(func_get_args(),3,null);
			
			if($arguments[2] && !is_array($arguments[2])){
				$arguments[2] = array($arguments[2]);
			}else if(!$arguments[2]){
				$arguments[2] = array();
			}
			
			call_user_func_array(array('parent','__construct'),array_slice($arguments,0,2));
			$this->listItems = $arguments[2];
			$this->itemEvents = array();
			$this->itemClasses = array();
			$this->itemAttributes = array();
		}
		
		public function addItem($itemName,$listItem){
			$this->listItems[$itemName] = $listItem;
		}
		
		public function __call($method, $arguments){
			if(preg_match("/^onItem([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,2,array());
				
				$this->itemEvents[$event] = array($arguments[0],$arguments[1]);
			}else{
				return parent::__call($method,$arguments);
			}
		}
		
		public function addItemClass($class){
			$this->itemClasses[] = $class;
		}
		
		public function removeItemClasses(){
			$this->itemClasses = array();
		}
		
		public function addItemAttribute($attribute,$value){
			$this->itemAttributes[$attribute] = $value;
		}
		
		public function removeItemAttributes(){
			$this->itemAttributes = array();
		}
		
		public function renderComponent(){
			$renderedComponent = "";
			
			if($this->getTitle()){
				$renderedComponent .= "<div class='title'>{$this->getTitle()}</div>";
			}
			
			foreach($this->listItems as $itemName => $listItem){
				$itemContainer = "<div ";
				
				foreach($this->itemEvents as $itemEvent => $action){
					$itemContainer .= " {$action[0]}='{$action[1]}()'";
				}
				
				$itemContainer .= "class='".implode(",",array_merge(array("listingItem"),$this->itemClasses))."'";
				
				foreach($this->itemAttributes as $attributName => $attributeValue){
					$itemContainer .= " $attributName='$attributeValue'";
				}
				$itemContainer .= ">";
				
				if(!is_int($itemName)){
					$itemContainer .= "<span>$itemName:</span>";
				}
				
				$itemContainer .= "<span>";
				if(is_string($listItem)){
					$itemContainer .= $listItem;
				}else{
					$itemContainer .= $listItem->render(false);
				}
				
				$itemContainer .= "</span>";
				
				$itemContainer .= "</div>";
				$renderedComponent .= $itemContainer;
			}
			
			return $renderedComponent;
		}
		
		protected function padIndicies($itemIndicies){
			if(!$itemIndicies){
				$itemIndicies = array_keys($this->getListItems());
			}
			
			$newIndicies = array();
			foreach($itemIndicies as $index){
				if(is_int($index) || is_string($index)){
					$newIndicies[] = $index;
				}
			}
			
			return $newIndicies;
		}
	}
?>