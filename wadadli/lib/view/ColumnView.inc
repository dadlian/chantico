<?php
	class ColumnView extends View{
		/** @Integer */
		protected $columns;
		
		public function __construct($viewId, $columns=1, $includeHeader=true, $includeFooter=true){
			parent::__construct($viewId,$columns);
			
			if(!is_int($this->getColumns())){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"ColumnView constructor expects integer column count, ".
										gettype($this->getColumns())." given.");
				return;
			}
			
			if($includeHeader){
				$this->addZone("{$viewId}Header","viewHeader");
			}
			
			for($i=0; $i < $this->getColumns(); $i++){
				$this->addZone("{$viewId}Column$i","viewColumn");
			}
			
			if($includeFooter){
				$this->addZone("{$viewId}Footer","viewFooter");
			}
		}
		
		public function addToHeader($component){
			$this->addToZone("{$this->getComponentId()}Header", $component);
		}
		
		public function clearHeader(){
			$this->clearZone("{$this->getComponentId()}Header");
		}
		
		public function addToColumn($columnIndex,$component){
			if(!is_int($columnIndex)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"ColumnView addToColumn expects integer column index, ".gettype($columnIndex)." given.");
				return;
			}
			
			$this->addToZone("{$this->getComponentId()}Column$columnIndex", $component);
		}
		
		public function clearColumn($columnIndex){
			if(!is_int($columnIndex)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"ColumnView addToColumn expects integer column index, ".gettype($columnIndex)." given.");
				return;
			}
			
			$this->clearZone("{$this->getComponentId()}Column$columnIndex");
		}
		
		public function addToFooter($component){
			$this->addToZone("{$this->getComponentId()}Footer", $component);
		}
		
		public function clearFooter(){
			$this->clearZone("{$this->getComponentId()}Footer");
		}
		
		protected function renderComponent(){
			$stretcher = new WadadliComponent();
			$stretcher->addClass('stretcher');
		
			for($column=0; $column < $this->getColumns(); $column++){
				$this->addToColumn($column,$stretcher);
			}
		
			return parent::renderComponent();
		}
	}
?>