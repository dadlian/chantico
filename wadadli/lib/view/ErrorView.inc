<?php
	class ErrorView extends ColumnView{
		public function __construct(){
			parent::__construct("errorView",1,false,false);

			$this->addToColumn(0,new WadadliSystemMessageBox("systemMessageBox"));
		}
	}
?>