<?php
	class WadadliObjectPageSelect extends WadadliPageSelect{
		public function __construct($componentId, $objectClass, $pageLength, $action){
			if(DatabaseAdministrator::tableExists($objectClass)){
				$pageResult = DatabaseAdministrator::execute("SELECT COUNT(*) as count FROM $objectClass");
				$pages = (int)ceil($pageResult[0]['count']/$pageLength);
			}else{
				$pages = 1;
			}
			
			
			parent::__construct($componentId, $pages, $action);
		}
	}
?>