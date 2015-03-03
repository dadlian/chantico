<?php
	class WadadliTable extends WadadliComponent{
		/** @Integer(required=true) */
		protected $rows;
		
		/** @Integer(required=true) */
		protected $columns;
		
		/** @Collection(type=@String) */
		protected $headings;
		
		private $cells;
		private $events;
		private $classes;
		private $attributes;
		
		public function __construct(){
			$arguments = func_get_args();
			$headings = array();
			if(sizeof($arguments) > 3 && is_array($arguments[3])){
				foreach($arguments[3] as $heading){
					$headings[] = $heading;
				}
			}
			$arguments[3] = array_pad($headings,intval($arguments[2]),"");
			
			call_user_func_array(array('parent','__construct'),$arguments);
			$this->setTag('table');
			
			$this->cells = array();
			for($row=0; $row<$this->getRows(); $row++){
				$this->cells[$row] = array();
			
				for($column=0; $column<$this->getColumns(); $column++){
					$this->cells[$row][$column] = array();
				}
			}
			
			$this->events = array('heading'=>array(),'row'=>array(),'column'=>array());
			$this->classes = array('heading'=>array(),'row'=>array(),'column'=>array());
			$this->attributes = array('heading'=>array(),'row'=>array(),'column'=>array());
		}
		
		public function renderComponent(){
			$renderedContent = "";
			
			$headings = $this->getHeadings();
			if($headings){
				$renderedContent .= "<thead><tr>";
				for($column=0; $column < $this->getColumns(); $column++){
					$renderedContent .= "<th";
			
					if(array_key_exists($column,$this->events['heading'])){
						foreach($this->events['heading'][$column] as $event => $parameters){
							$renderedContent .= " $event='{$parameters[0][0]}(";
							
							foreach($parameters[0][1] as $argument){
								$renderedContent .= "\"$argument\",";
							}
							
							$renderedContent .= "this)'";
						}
					}
					
					$renderedContent .= " class='";
					if(array_key_exists($column,$this->classes['heading'])){
						foreach($this->classes['heading'][$column] as $attribute => $class){
							$renderedContent .= " {$class[0]}";
						}
					}
					$renderedContent .= "'";
					
					if(array_key_exists($column,$this->attributes['heading'])){
						foreach($this->attributes['heading'][$column] as $attribute => $value){
							$renderedContent .= " $attribute='$value[0]'";
						}
					}
					
					$renderedContent .= ">{$headings[$column]}</th>";
				}
				$renderedContent .= "</tr></thead>";
			}
			
			$renderedContent .= "<tbody>";
			for($row=0; $row < $this->getRows(); $row++){
				$rowContainer = "<tr";
				
				if(array_key_exists($row,$this->events['row'])){
					foreach($this->events['row'][$row] as $event => $parameters){
						$rowContainer .= " $event='{$parameters[0][0]}(";
						
						foreach($parameters[0][1] as $argument){
							$rowContainer .= "\"$argument\",";
						}
						
						$rowContainer .= "this)'";
					}
				}
				
				if(array_key_exists($row,$this->classes['row'])){
					$rowClasses = array();
					foreach($this->classes['row'][$row] as $value){
						$rowClasses[] = $value[0];
					}
					$rowContainer .= " class='".implode(" ",$rowClasses)."'";
				}
				
				if(array_key_exists($row,$this->attributes['row'])){
					foreach($this->attributes['row'][$row] as $attribute => $value){
						$rowContainer .= " $attribute='$value[0]'";
					}
				}
				
				$rowContainer .= ">";
				
				for($column=0; $column < $this->getColumns(); $column++){
					if(!$this->cells[$row][$column]){
						continue;
					}
				
					$cell = "<td id='{$this->getComponentId()}Cell{$row}{$column}'";
			
					if(array_key_exists($column,$this->events['column'])){
						foreach($this->events['column'][$column] as $event => $parameters){
							if(!in_array($row,$parameters[1])){
								$cell .= " $event='{$parameters[0][0]}(";
								
								foreach($parameters[0][1] as $argument){
									$cell .= "\"$argument\",";
								}
								
								$cell .= "this)'";
							}
						}
					}
				
					$cell .= "class='";
					if(array_key_exists($column,$this->classes['column'])){
						foreach($this->classes['column'][$column] as $attribute => $class){
							if(!in_array($row,$class[1])){
								$cell .= " {$class[0]}";
							}
						}
					}
					$cell .= "'";
					
					if(array_key_exists($column,$this->attributes['column'])){
						foreach($this->attributes['column'][$column] as $attribute => $value){
							if(!in_array($row,$value[1])){
								$cell .= " $attribute='$value[0]'";
							}
						}
					}
					
					$cell .= ">";
					
					foreach($this->cells[$row][$column] as $cellContent){
						if(is_string($cellContent)){
							$cell .= $cellContent;
						}else{
							$cell .= $cellContent->render(false);
						}
					}
					
					$cell .= "</td>";
					$rowContainer .= $cell;
				}
				
				$rowContainer .= "</tr>";
				$renderedContent .= $rowContainer;
			}
			$renderedContent .= "</tbody>";
			
			return $renderedContent;
		}
		
		public function setColumnHeading($column, $heading){
			if(!is_int($column)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable setColumnHeading expects integer column index, "
						.gettype($column)." given");
				return;
			}else if($column < 0 || $column >= $this->getColumns()){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable column index, $column, is out of range [0,{$this->getColumns()}]");
				return;
			}
			
			$this->insertToHeadings($column, $heading);
		}
		
		public function addContent($row,$column,$content){
			if(!is_int($row)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable addContent expects integer row index, ".gettype($column)." given");
				return;
			}else if($row < 0 || $row >= $this->getRows()){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable row index, $row, is out of range [0,{$this->getRows()}]");
				return;
			}
			
			if(!is_int($column)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable addContent expects integer column index, ".gettype($column)." given");
				return;
			}else if($column < 0 || $column >= $this->getColumns()){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable column index, $column, is out of range [0,{$this->getColumns()}]");
				return;
			}
			
			$this->cells[$row][$column][] = $content;
		}
		
		public function clearContent($row,$column){
			if(!is_int($row)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable clearContent expects integer row index, ".gettype($column)." given");
				return;
			}else if($row < 0 || $row >= $this->getRows()){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable row index, $row, is out of range [0,{$this->getRows()}]");
				return;
			}
			
			if(!is_int($column)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable clearContent expects integer column index, ".gettype($column)." given");
				return;
			}else if($column < 0 || $column >= $this->getColumns()){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"WadadliTable column index, $column, is out of range [0,{$this->getColumns()}]");
				return;
			}
			
			$this->cells[$row][$column] = array();
		}
		
		public function clearTable(){
			for($row=0; $row<$this->getRows(); $row++){
				for($column=0; $column<$this->getColumns(); $column++){
					$this->clearContent($row,$column);
				}
			}
		}
		
		public function __call($method, $arguments){
			if(preg_match("/^onRow([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,4,array());
				
				$this->addTableProperty('row','events',$event,array($arguments[0],$arguments[1]),$arguments[2]);
			}else if(preg_match("/^onColumn([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,4,array());
				
				$this->addTableProperty('column','events',$event,array($arguments[0],$arguments[1]),$arguments[2],$arguments[3]);
			}else if(preg_match("/^onHeading([A-Z]\w*)/", $method, $matches)){
				$event = "on{$matches[1]}";
				$arguments = array_pad($arguments,4,array());
				
				$this->addTableProperty('heading','events',$event,array($arguments[0],$arguments[1]),$arguments[2],$arguments[3]);
			}else{
				return parent::__call($method,$arguments);
			}
		}
		
		public function addHeadingClass($class,$column){
			$this->addTableProperty('heading','classes',"",$class,array($column));
		}
		
		public function removeHeadingClasses($column){
			$this->removeTableProperty('heading','classes',array($column));
		}
		
		public function addHeadingAttribute($attribute,$value,$column){
			$this->addTableProperty('heading','attributes',$attribute,$value,array($column));
		}
		
		public function removeHeadingAttributes($column){
			$this->removeTableProperty('heading','attributes',array($column));
		}
		
		public function addRowClass($class,$rows=array()){
			$this->addTableProperty('row','classes',"",$class,$rows);
		}
		
		public function removeRowClasses($rows=array()){
			$this->removeTableProperty('row','classes',$rows);
		}
		
		public function addRowAttribute($attribute,$value,$rows=array()){
			$this->addTableProperty('row','attributes',$attribute,$value,$rows);
		}
		
		public function removeRowAttributes($rows=array()){
			$this->removeTableProperty('row','attributes',$rows);
		}
		
		public function addColumnClass($class,$columns=array(),$excludeRows=array()){
			$this->addTableProperty('column','classes',"",$class,$columns,$excludeRows);
		}
		
		public function removeColumnClasses($columns=array()){
			$this->removeTableProperty('column','classes',$columns);
		}
		
		public function addColumnAttribute($attribute,$value,$columns=array(),$excludeRows=array()){
			$this->addTableProperty('column','attributes',$attribute,$value,$columns,$excludeRows);
		}
		
		public function removeColumnAttributes($columns=array()){
			$this->removeTableProperty('column','attributes',$columns);
		}
		
		private function addTableProperty($target,$property,$key,$value,$targetIndicies,$excludeRows=array()){
			if(!$targetIndicies){
				$getter = "get".capitalise($target)."s";
				$targetIndicies = range(0,$this->$getter());
			}
			
			$classProperty = $this->$property;
			foreach($targetIndicies as $targetIndex){
				if(!array_key_exists($targetIndex, $classProperty[$target])){
					$classProperty[$target][$targetIndex] = array();
				}
				
				if($key){
					$classProperty[$target][$targetIndex][$key] = array($value,$excludeRows);
				}else{
					$classProperty[$target][$targetIndex][] = array($value,$excludeRows);
				}
			}
			$this->$property = $classProperty;
		}
		
		private function removeTableProperty($target,$property,$targetIndicies){
			if(!$targetIndicies){
				$targetIndicies = range(0,$this->getRows());
			
				$classProperty = $this->$property;
				foreach($targetIndicies as $targetIndex){
					if(array_key_exists($targetIndex,$classProperty[$target])){
						$classProperty[$target][$row] = array();
					}
				}
				$this->$property = $classProperty;
			}
		}
	}
?>