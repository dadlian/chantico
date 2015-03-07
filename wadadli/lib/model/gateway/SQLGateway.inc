<?php
	class SQLGateway extends Gateway{		
		//Keep track of classes processed this session so we don't revisit them
		private $classesSeen;
				
		//An array of all list properties seen for table creation
		private $listProperties;
		
		//A map of loaded class object members for delayed loading
		private $objectMembers;
		
		//A map of queued insert queries and their arguments for delayed execution
		private $insertQueries;
		
		public function find($className, $searcher=null, $sorter=null, $records=0, $start=0, $fullLoad=true){
			$objects = array();
			
			//Verify that $className is a string
			if(!is_string($className)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING, "SQLGateway find expects a string search class argument. ".
										gettype($className)." given.");
				return;
			}
			
			//Verify that the argument $className is valid and exists
			if(!class_exists($className)){
				warning(UNEXPECTED_ARGUMENT_WARNING, "SQLGateway can only search for PersistentClass objects. ". $className.
									" is not Persistent.");
				return $objects;
			}
			
			//Verify that the argument $className is a PersistentClass
			$reflectedClass = Mirror::reflectClass($className);
			if(!$reflectedClass->descendsFrom('PersistentClass')){
				warning(UNEXPECTED_ARGUMENT_WARNING, "SQLGateway can only search for PersistentClass objects. ". $className.
									" is not Persistent.");
				return $objects;
			}
			
			if($reflectedClass->isAbstract()){
				warning(UNEXPECTED_ARGUMENT_WARNING, "SQLGateway cannot search for abstract PersistentClasses. $className is abstract.");
				return $objects;
			}
			
			//Initialise an empty searcher if none was specified
			if(is_null($searcher)){
				$searcher = new Searcher(array());
			}
			
			//Test that searcher is actually a searcher object
			if(!is_object($searcher) || get_class($searcher) != "Searcher"){
				if(is_object($searcher)){
					$type = get_class($searcher);
				}else{
					$type = gettype($searcher);
				}
				
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"SQLGateway find expects a Searcher class object argument as a searcher, $type given.");
				return $objects;
			}
			
			//Initialise an empty sorter if none was specified
			if(is_null($sorter)){
				$sorter = new Sorter(array());
			}
			
			//Test that sorter is actually a sorter object
			if(!is_object($sorter) || get_class($sorter) != "Sorter"){
				if(is_object($sorter)){
					$type = get_class($sorter);
				}else{
					$type = gettype($sorter);
				}
				
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"SQLGateway find expects a Sorter class object argument as a sorter, $type given.");
				return $objects;
			}
			
			if(!is_int($records)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"SQLGateway find expects integer limit values, ".gettype($records)." given.");
				return $objects;
			}else if($records < 0){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"SQLGateway find limit values must be greater than or equal to 0.");
				return $objects;
			}
			
			if(!is_int($start)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"SQLGateway find expects integer start values, ".gettype($start)." given.");
				return $objects;
			}else if($start < 0){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"SQLGateway find start values must be greater than or equal to 0.");
				return $objects;
			}
			
			$class = Mirror::reflectClass($className);
			
			//If there are no objects of the searched for type in the database, simply return an empty array
			if(!DatabaseAdministrator::tableExists($className)){
				return $objects;
			}
			
			//Construct a query and search for all objects matching it
			$where = array();
			$whereArguments = array();
			
			$criteria = array();
			if($searcher->getCriteria()){
				$criteria = $searcher->getCriteria();
			}
			
			foreach($criteria as $criterion){
				$condition = $criterion->getCondition();
				
				//We search the object table using its own arguments
				if($class->hasProperty($criterion->getField())){
					$property = Mirror::reflectProperty($className, $criterion->getField());
					$annotation = $property->getAnnotation();
					$values = $criterion->getValues();
					$field = $criterion->getField();
					
					//Optimisation: If finding an object by ID load it from the cache if it exists and return it immediately
					foreach($values as $value){
						if($field == 'id' && $condition == Criterion::EQUAL && QuarterMaster::release($value) && 
							get_class(QuarterMaster::release($value)) == $className){
							$objects[$value] = QuarterMaster::release($value);
						}
					}
					
					if(sizeof($objects) == sizeof($values)){
						return $objects;
					}
					
					//Find Collection Root Field Type
					if($annotation->isCollection()){
						$listTables = array();
						while($annotation->isCollection()){
							if(!$listTables){
								$listTables[] = "$className".capitalise($property->getName());
							}else{
								$listTables[] = $listTables[sizeof($listTables)-1]."List";
							}
							
							$annotation = $annotation->getContainedType();
						}
						
						$field = "value";
					}
					
					//Convert conditions if necessary
					if($annotation->isString() && $condition == Criterion::EQUAL){
						$condition = Criterion::LIKE;
					}else if($annotation->isString() && $condition == Criterion::NOT_EQUAL){
						$condition = Criterion::NOT_LIKE;
					}
					
					//Convert parameters as necessary based on property type
					for($i=0; $i < sizeof($values); $i++){
						if($annotation->isFloat() || $annotation->isMonetary()){
							$decimals = strlen(end(preg_split("/\./",strval($values[$i]))));
							$field = "ROUND($field,$decimals)";
						}
					}
					
					//Create where condition based on property type
					if($property->getAnnotation()->isCollection()){
						if(!Criterion::isListComparator($condition)){
							warning(DATA_ACCESS_ERROR, "SQLGateway cannot find $className objects using non-applicable comparator ".
									"'{$condition}'.");
							continue;
						}
						
						$listCondition = "";
						for($i=sizeof($listTables)-1; $i >= 0; $i--){
							$parentColumn = "parentList";
							if($i == 0){
								$parentColumn = decapitalise($className);
							}
							
							if(sizeof($values) > 1){
								$equality = Criterion::INCLUDES;
								$placeholder = "(".implode(",",array_fill(0,sizeof($values),"?")). ")";
							}else{
								$placeholder = "?";
							
								if(!is_null($value)){
									$equality = Criterion::EQUAL;
								}else{
									$equality = Criterion::IS;
								}
							}
							
							if(!$listCondition){
								$listCondition = "SELECT $parentColumn FROM {$listTables[$i]} WHERE $field $equality $placeholder";
								$whereArguments = array_merge($whereArguments,$values);
							}else{
								$listCondition = "SELECT $parentColumn FROM {$listTables[$i]} WHERE id IN ($listCondition)";
							}
						}
							
						$where[] = "id {$condition} ($listCondition)";
					}else{
						if(($annotation->isObject() && !Criterion::isObjectComparator($condition))
							|| !Criterion::isPrimitiveComparator($condition)){
							warning(DATA_ACCESS_ERROR, "SQLGateway cannot find $className objects using non-applicable comparator ".
									"'{$condition}'.");
							continue;
						}
						
						if(sizeof($values) > 1){
							$condition = Criterion::INCLUDES;
							$placeholder = "(".implode(",",array_fill(0,sizeof($values),"?")). ")";
						}else{
							$placeholder = "?";
						}
						
						$where[] = "$field {$condition} $placeholder";
						$whereArguments = array_merge($whereArguments,$values);
					}
				//If the search is for inclusion in another object we generate a simple query
				}else if(class_exists($criterion->getField())){
					$value = $criterion->viewFromValues(0);
					$table = $criterion->getField();
					
					$ownerClass = Mirror::reflectClass($criterion->getField());
					
					if(!$ownerClass->hasProperty($value)){
						warning(DATA_ACCESS_ERROR, "SQLGateway cannot find $className objects using non-existant owner field ".
										"'{$value}'.");
						continue;
					}else if((string)((int)$condition) !== $condition && !Criterion::isListComparator($condition)){
						warning(DATA_ACCESS_ERROR, "SQLGateway cannot find $className objects using non-applicable comparator ".
								"'{$condition}'.");
						continue;
					}else if(!$ownerClass->descendsFrom('PersistentClass')){
						warning(DATA_ACCESS_ERROR, "SQLGateway cannot find $className objects using non-persistent owner ".
								"'{$criterion->getField()}'.");
						continue;
					}
					
					//If the condition is an ID number we are looking for inclusion in a particular class
					if((string)((int)$condition) == $condition){
						$where[] = "id IN (SELECT value FROM $table".capitalise($value)." WHERE ".strtolower($table)." = '$condition')";
					//else we look for inclusion in ANY class
					}else{
						$property = Mirror::reflectProperty($criterion->getField(), $value);
						$annotation = $property->getAnnotation();
					
						if($annotation->isCollection()){
							$value = "value";
							$table = "";
						
							while($annotation->isCollection()){
								if(!$table){
									$table = $criterion->getField().capitalise($criterion->viewFromValues(0));
								}else{
									$table .= "List";
								}
							
								$annotation = $annotation->getContainedType();
							}						
						}
					
						if(!DatabaseAdministrator::tableExists($ownerClass->getName()) || !$annotation->isObject()){
							continue;
						}
					
						$where[] = "id {$condition} (SELECT $value FROM $table)";
					}
				}else{
					warning(DATA_ACCESS_ERROR, "SQLGateway cannot find $className objects fron non-existant field ".
									"'{$criterion->getField()}'.");
				}
			}
			
			//Create result order based on Sorter
			$sorts = array();
			$criteria = array();
			if($sorter->getCriteria()){
				$criteria = $sorter->getCriteria();
			}
			
			foreach($criteria as $criterion){
				$condition = $criterion->getCondition();
				
				if($condition != Criterion::RANDOM && !$class->hasProperty($criterion->getField())){
					warning(DATA_ACCESS_ERROR, "SQLGateway cannot sort $className results using non-existant field ".
									"'{$criterion->getField()}'.");
					continue;
				}else if(!in_array($condition, array(Criterion::ASCENDING, Criterion::DESCENDING, Criterion::RANDOM))){
					warning(DATA_ACCESS_ERROR, "SQLGateway cannot sort $className results according to invalid order ".
									"'{$condition}'.");
					continue;
				}
				
				if($condition == Criterion::RANDOM){
					$sorts[] = "$condition";
				}else{
					$sortProperty = Mirror::reflectProperty($className, $criterion->getField());
					if($sortProperty->getAnnotation()->isCollection()){
						warning(DATA_ACCESS_ERROR, "SQLGateway cannot sort $className results using collection field ".
										"'{$criterion->getField()}'.");
						continue;
					}
					
					$sorts[] = "a.{$criterion->getField()} {$condition}";
				}
			}
			
			$order = "";
			if($sorts){
				$order = "ORDER BY ".implode(",",$sorts);
			}
			
			//Set result set limits according to parameters
			$limit = "";
			if($records){
				$limit = "LIMIT $start,$records";
			}
			
			//We want to search and select fields from the full class hierarchy
			$persistentClass = Mirror::reflectClass('PersistentClass');
			$classHierarchy = array_diff($reflectedClass->getClassHierarchy(),$persistentClass->getClassHierarchy());
			
			$index = 97;
			$label = chr($index);
			$previousLabel = $label;
			$fromTable = "$className AS $label";
			foreach(array_reverse($classHierarchy) as $class){
				if($className != $class->getName()){
					$index++;
					$label = chr($index);
					$fromTable .= " JOIN {$class->getName()} AS $label ON $previousLabel.id = $label.id";
					$previousLabel = $label;
				}
			}
			
			$query = "SELECT * FROM $fromTable ".($where?preg_replace("/\sid\s/"," a.id ","WHERE ".implode(" AND ",$where)):"")." $order $limit";
			//Replace placeholders with hard coded nulls, if necessary
			if(in_array(null,$whereArguments)){
				$positions = array();
				$pos = -1;
				while (($pos = strpos($query,"?", $pos+1)) !== false) {
					$positions[] = $pos;
				}
				$filteredWhereArguments = array();
				for($i=0; $i<sizeof($whereArguments); $i++){
					$whereArgument = $whereArguments[$i];
					if(is_null($whereArgument)){
						$query = substr_replace($query,"null",$positions[$i],1);
					}else{
						$filteredWhereArguments[] = $whereArgument;
					}
				}
				
				$whereArguments = $filteredWhereArguments;
			}
			
			$results = call_user_func_array(array('DatabaseAdministrator','execute'),array_merge(array($query),$whereArguments));
			$data = array();
			
			foreach($results as $result){
				$data[$result['id']] = $result;
			}
			
			$this->objectMembers = array();
			
			foreach($data as $id => $objectData){
				$objects[$id] = $this->loadObject($reflectedClass, $objectData);
			}
			
			//If objects are to be lazy loaded we return the stubbed objects. Otherwise we load nested object and collections.
			if($fullLoad && $objects){
				$subGateway = new SQLGateway();
				$searcher = new Searcher();
				foreach($reflectedClass->getProperties() as $property){
					$setter = "set".capitalise($property->getName());
					if($property->getAnnotation()->isObject()){
						$containedClass = $property->getAnnotation()->getObjectClass();
						$reflectedClass = Mirror::reflectClass($containedClass);
						
						if(!$reflectedClass->descendsFrom("PersistentClass")){
							continue;
						}
						
						$ids = array();
						if(array_key_exists($containedClass, $this->objectMembers)){
							$ids = array_keys($this->objectMembers[$containedClass]);
						}
						
						$searcher->clearCriteria();
						$searcher->addCriterion('id',Criterion::EQUAL,$ids);
						
						foreach($subGateway->find($containedClass, $searcher) as $memberObject){
							foreach($this->objectMembers[$containedClass][$memberObject->getId()] as $objectId){
								$objects[$objectId]->$setter($memberObject);
							}
						}
					}else if($property->getAnnotation()->isCollection()){
						$collections = $this->loadCollections($className.capitalise($property->getName()),
											$property->getAnnotation()->getContainedType(),
											array_keys($objects),strtolower($className));
						
						foreach($collections as $ownerID => $collection){
							$objects[$ownerID]->$setter($collection);
						}
					}
				}
			}
			
			return $objects;
		}
		
		public function save($saveObjects){
			if(!is_array($saveObjects)){
				$saveObjects = array($saveObjects);
			}
			
			foreach($saveObjects as $saveObject){
				if(!is_object($saveObject) || !$this->checkUpdateParameters($saveObject)){
					fatal_error(DATA_MODIFY_ERROR, "Only objects of PersistentClass can be saved via a Gateway");
					return;
				}
				
				//Re-initialise seen class hierarchy for this object
				$this->classesSeen = array();
				$this->listProperties = array();

				//Manage table hierarchy for object class
				$this->manageTableForClass(Mirror::reflectClass($saveObject));
				
				while(sizeof($this->listProperties) > 0){
					$listProperty = array_shift($this->listProperties);
					$this->manageTableForList($listProperty);
				}
				
				//Save object data to table hierarchy
				$this->writeObject($saveObject);
			}
		}
		
		public function delete($deleteObjects){
			if(!is_array($deleteObjects)){
				$deleteObjects = array($deleteObjects);
			}
			
			$oldObjectMap = array();
			$oldFileList = array();
			
			foreach($deleteObjects as $deleteObject){
				if(!is_object($deleteObject) || !$this->checkUpdateParameters($deleteObject)){
					fatal_error(DATA_MODIFY_ERROR, "A SQLGateway can only delete PersistentClass Objects.");
					return;
				}
				
				$class = Mirror::reflectClass($deleteObject);
				
				if(DatabaseAdministrator::tableExists($class->getName())){
					if(!array_key_exists($class->getName(),$oldObjectMap)){
						$oldObjectMap[$class->getName()] = array();
					}
					
					$oldObjectMap[$class->getName()][] = $deleteObject->getId();
				}
				
				//Enqueue old files for deletion
				foreach($class->getProperties() as $property){
					$annotation = $property->getAnnotation();
					while($annotation->isCollection()){
						$annotation = $annotation->getContainedType();
					}
					
					if($annotation->isFile()){
						$getter = "get".capitalise($property->getName());
						$oldFileList = array_merge($oldFileList, array_flatten($deleteObject->$getter()));
					}
				}
			}
			
			//Delete old objects
			foreach($oldObjectMap as $class => $objects){
				DatabaseAdministrator::execute("DELETE FROM $class WHERE id IN (".implode(",",$objects).")");
			}
			
			//Delete old files
			foreach($oldFileList as $oldFile){
				if($oldFile){
					unlink(SettingsManager::getSetting('Install','Project Path')."/$oldFile");
				}
			}
		}
		
		/*
		 * Ensures that a table representation of an object already exists, and decides how to proceed.
		 */
		private function manageTableForClass($class){
			if(!in_array($class->getName(), $this->classesSeen)){
				$this->classesSeen[] = $class->getName();
				if(DatabaseAdministrator::tableExists($class->getName())){
					//Add any new class fields, or change field types for existing table
					$this->alterTableForClass($class);
				}else{
					$this->createTableForClass($class);
				}
			}
		}
		
		/*
		 * Checks whether a table representation of the list already exists, and decides how to proceed based on the result.
		 */
		private function manageTableForList($property){
			$class = $property->getDeclaringClass();
			
			if(!in_array($class->getName().capitalise($property->getName()), $this->classesSeen)){
				$this->classesSeen[] = $class->getName().capitalise($property->getName());
				if(DatabaseAdministrator::tableExists($class->getName().capitalise($property->getName()))){
					$this->alterTableForList($property);
				}else{
					$this->createTableForList($property);
				}
			}
		}
			 
		/*
		 * Creates a database table corresponding to a certain PersistentClass if it does not already exist
		 */
		private function createTableForClass($class){
			//Every class will need to include an ID field to associate with its parent
			$classProperties = array(Mirror::reflectProperty($class->getName(), "id"),Mirror::reflectProperty($class->getName(), "created"),Mirror::reflectProperty($class->getName(), "modified"));
			$classProperties = array_merge($classProperties, $class->getProperties(false));
			
			//Check that necessary parent table exist. Create them if not
			$classHierarchy = $class->getParentClass()->getClassHierarchy();
			$persistentClass = Mirror::reflectClass("PersistentClass");
			$classHierarchy = array_diff($classHierarchy, $persistentClass->getClassHierarchy());
					
			foreach($classHierarchy as $nextClass){
				$this->manageTableForClass($nextClass);
			}
						
			$createStatement = "CREATE TABLE {$class->getName()}(";
			$idType = "";
			
			foreach($classProperties as $property){
				if($property->getAnnotation()->isCollection()){
					continue;
				}
				
				$columnType = "";
				$propertyName = $property->getName();
				$columnType = $this->getColumnType($property->getAnnotation());
				
				//Create object field table
				if($property->getAnnotation()->isObject()){
					$this->manageTableForClass(Mirror::reflectClass($property->getAnnotation()->getObjectClass()));
				}
				
				if($property->getName() == "id"){
					$idType = $columnType;
					$columnType .= " PRIMARY KEY";
				}				
				
				$createStatement .= "{$property->getName()} $columnType,";
			}
			
			//If class is not hierarchy root, add foreign key to parent object
			if($class->getParentClass()->getName() != "PersistentClass"){
				$parentClassName = $class->getParentClass()->getName();
				$constraintName = "fk_".SettingsManager::getSetting("database","tableprefix")."_".substr(strtolower($class->getName()), 0, 25)."_id";
				$createStatement .= "CONSTRAINT $constraintName FOREIGN KEY (id) 
							REFERENCES $parentClassName (id) ON DELETE CASCADE ON UPDATE CASCADE,";
			}
			
			//Add foreign keys for object references
			foreach($classProperties as $property){
				if($property->getAnnotation()->isObject()){
					$propertyName = $property->getName();
					$objectTable = $property->getAnnotation()->getObjectClass();
					$constraintName = "fk_".SettingsManager::getSetting("database","tableprefix")."_".substr(strtolower($class->getName()),0,25).substr("_$propertyName",0,20);
					$createStatement .= "CONSTRAINT $constraintName FOREIGN KEY ($propertyName) REFERENCES $objectTable (id) 
								ON DELETE SET NULL ON UPDATE CASCADE,";
				}
			}
			
			//Remove trailing comma from $createStatement
			$createStatement = substr($createStatement, 0, strlen($createStatement) - 1);
			$createStatement .= ")ENGINE=INNODB";
			DatabaseAdministrator::execute($createStatement);
			
			//Mark list property for later table creation
			foreach($classProperties as $property){
				if($property->getAnnotation()->isCollection()){
					$this->listProperties[] = $property;
				}
			}
		}
			 
		/*
		 * Creates a database table corresponding to stored object lists when a class has a one-to-many or many-to-many relationship
		 */
		private function createTableForList($property){
			$propertyName = $property->getName();
			$className = $property->getDeclaringClass()->getName();
			$annotation = $property->getAnnotation();
			$listAnnotations = array();
			
			//Get a listing of all the nested types of the Collection property
			while($annotation->isCollection()){
				$annotation = $annotation->getContainedType();
				$listAnnotations[] = $annotation;
			}
			
			//Traverse type list to create tables for each nested list
			for($i = 0; $i < sizeof($listAnnotations); $i++){
				$nextAnnotation = $listAnnotations[$i];
				$tableName = $className.capitalise($propertyName).str_repeat("List", $i);
				if(DatabaseAdministrator::tableExists($tableName)){
					continue;
				}
				
				$createStatement = "CREATE TABLE $tableName(id INT AUTO_INCREMENT PRIMARY KEY,";
				if($i > 0){
					$createStatement .= "parentList INT NOT NULL,";
				}else{
					$createStatement .= strtolower($className)." VARCHAR(20) NOT NULL,";
				}
				
				$createStatement .= "name VARCHAR(128),";
				
				if($nextAnnotation->isObject()){
					$objectClass = Mirror::reflectClass($nextAnnotation->getObjectClass());
					$this->manageTableForClass($objectClass);
					
					$constraintName = "fk_".SettingsManager::getSetting("database","tableprefix")."_".substr(strtolower($tableName),0,25)."_value";
					$createStatement .= "value VARCHAR(20),CONSTRAINT $constraintName
								FOREIGN KEY (value) REFERENCES {$objectClass->getName()} (id) 
								ON UPDATE CASCADE ON DELETE CASCADE,";
				}else if(!$nextAnnotation->isCollection()){
					$columnType = $this->getColumnType($nextAnnotation);
					$createStatement .= "value $columnType,";
				}
				
				$tableAbbr = substr(strtolower($tableName), 0, 15).substr(strtolower($tableName), strlen($tableName)-10,strlen($tableName));
				if($i > 0){
					$constraintName = "fk_".SettingsManager::getSetting("database","tableprefix")."_".$tableAbbr."_parentlist";
					$createStatement .= "CONSTRAINT $constraintName FOREIGN KEY (parentList) REFERENCES ".
							     substr($tableName, 0, strlen($tableName) - 4).
							     " (id) ON UPDATE CASCADE ON DELETE CASCADE)ENGINE=INNODB";
				}else{
					$constraintName = "fk_".SettingsManager::getSetting("database","tableprefix")."_".$tableAbbr."_".substr(strtolower($className),0,20);
					$createStatement .= "CONSTRAINT $constraintName FOREIGN KEY (".strtolower($className).") REFERENCES $className (id)".
							     " ON UPDATE CASCADE ON DELETE CASCADE)ENGINE=INNODB";
				}
				
				DatabaseAdministrator::execute($createStatement);
			}
		}
		
		/*
		 * Change a database table to reflect new class definitions.
		 */
		private function alterTableForClass($class){
			//Get table description
			$tableName = $class->getName();
			$tableDescription = DatabaseAdministrator::execute("DESC $tableName");
			$classProperties = $class->getProperties(false);
			$tableFieldTypeMap = array();
			$tableForeignKeyMap = array();
			
			//Check that necessary parent tables exist. Create them if not
			$classHierarchy = $class->getParentClass()->getClassHierarchy();
			$persistentClass = Mirror::reflectClass("PersistentClass");
			$classHierarchy = array_diff($classHierarchy, $persistentClass->getClassHierarchy());
					
			foreach($classHierarchy as $nextClass){
				$this->manageTableForClass($nextClass);	
			}
			
			foreach($tableDescription as $column){
				$tableFieldTypeMap[$column['Field']] = $column['Type'];
				$tableForeignKeyMap[$column['Field']] = $column['Key'];
			}
			
			foreach($classProperties as $property){
				$changed = true;
				$propertyName = $property->getName();
				
				if(in_array($property->getName(), array_keys($tableFieldTypeMap))){
					$oldColumnType = $tableFieldTypeMap[$property->getName()];
					$oldColumnKey = $tableForeignKeyMap[$property->getName()];
				}else{
					$oldColumnType = "";
					$oldColumnKey = "";
				}
				
				$newColumnType = $this->getColumnType($property->getAnnotation());
				
				//Create object field table
				if($property->getAnnotation()->isObject()){
					$this->manageTableForClass(Mirror::reflectClass($property->getAnnotation()->getObjectClass()));
				}
				
				//Add any missing columns to table
				if(!$oldColumnType && !$property->getAnnotation()->isCollection()){
					$oldColumnType = $newColumnType;
					DatabaseAdministrator::execute("ALTER TABLE $tableName ADD $propertyName $newColumnType");
				
				//Alter table if a field type has changed
				}else if(($property->getAnnotation()->isObject() && (!preg_match("/^varchar/", $oldColumnType) 
						|| $oldColumnKey != "MUL")) ||
					($property->getAnnotation()->isInteger() && (!preg_match("/^int/", $oldColumnType) 
						|| $oldColumnKey == "MUL")) || 
					(($property->getAnnotation()->isFloat() || $property->getAnnotation()->isMonetary()) 
						&& $oldColumnType != "float") || 
					($property->getAnnotation()->isBoolean() && !preg_match("/tinyint/", $oldColumnType)) || 
					($property->getAnnotation()->isDate() && $oldColumnType != "datetime") ||
					($property->getAnnotation()->isText() && $oldColumnType != "text") ||
					($property->getAnnotation()->isString() && !$property->getAnnotation()->isText() && 
						$property->getAnnotation()->getMax() &&
						!preg_match("/^varchar\({$property->getAnnotation()->getMax()}/", $oldColumnType)) ||
					($property->getAnnotation()->isString() && !$property->getAnnotation()->isText() && 
						!$property->getAnnotation()->getMax() && !preg_match("/^varchar\(256/", $oldColumnType))){
					
					//Set all values in changed column to NULL to ensure compatability with new type
					DatabaseAdministrator::execute("UPDATE $tableName SET $propertyName=NULL");
					
					//If column was previously object and no longer is, drop foreign key
					if($oldColumnKey == "MUL" && !$property->getAnnotation()->isObject()){
						$columnKey = "fk_".SettingsManager::getSetting("database","tableprefix")."_".substr(strtolower($tableName),0,25).substr("_$propertyName",0,20);
						DatabaseAdministrator::execute("ALTER TABLE $tableName DROP FOREIGN KEY $columnKey");
						DatabaseAdministrator::execute("ALTER TABLE $tableName DROP KEY $columnKey");
					}
					
					DatabaseAdministrator::execute("ALTER TABLE $tableName CHANGE $propertyName $propertyName $newColumnType");
					
				//Otherwise table has not changed				
				}else{
					$changed = false;
				}
					
				//If column has changed to object add foreign key
				if($changed && $property->getAnnotation()->isObject()){
					$columnKey = "fk_".SettingsManager::getSetting("database","tableprefix")."_".substr(strtolower($tableName),0,25).substr("_$propertyName",0,20);
					$objectTable = $property->getAnnotation()->getObjectClass();
					DatabaseAdministrator::execute("ALTER TABLE $tableName ADD CONSTRAINT $columnKey 
									FOREIGN KEY ($propertyName) REFERENCES $objectTable (id) 
									ON DELETE SET NULL ON UPDATE CASCADE");
				}
			}
			
			//Mark list property for later table creation
			foreach($classProperties as $property){
				if($property->getAnnotation()->isCollection()){
					$this->listProperties[] = $property;
				}
			}
		}
		
		/*
		 * Change a list database table to reflect new class definitions.
		 */
		private function alterTableForList($property){
			$propertyName = $property->getName();
			$className = $property->getDeclaringClass()->getName();
			$annotation = $property->getAnnotation();
			$listAnnotations = array();
			
			//Get a listing of all the nested types of the Collection property
			while($annotation->isCollection()){
				$annotation = $annotation->getContainedType();
				$listAnnotations[] = $annotation;
			}
			
			//Traverse type list in order to alter tables for each nested list
			for($i = 0; $i < sizeof($listAnnotations); $i++){
				$nextAnnotation = $listAnnotations[$i];
				$tableName = $className.capitalise($propertyName).str_repeat("List", $i);
				$columnName = $propertyName;
				
				if(!DatabaseAdministrator::tableExists($tableName)){
					$this->createTableForList($property);
				}else{
					$tableDescription = DatabaseAdministrator::execute("DESC $tableName");
					$dataIndex = sizeof($tableDescription) - 1;
					
					$storedPropertyName = $tableDescription[$dataIndex]['Field'];
					$storedAnnotationType = $tableDescription[$dataIndex]['Type'];
					$storedKey = $tableDescription[$dataIndex]['Key'];
					
					$newColumnType = $this->getColumnType($nextAnnotation);
					if($nextAnnotation->isObject()){
						$this->manageTableForClass(Mirror::reflectClass($nextAnnotation->getObjectClass()));
					}
					
					$typeChange = false;
					if(($nextAnnotation->isObject() && (!preg_match("/^varchar/", $storedAnnotationType) || $storedKey != "MUL"))||
					   ($nextAnnotation->isInteger() && (!preg_match("/^int/", $storedAnnotationType) || $storedKey == "MUL"))||
					   (($nextAnnotation->isFloat() || $nextAnnotation->isMonetary()) && $storedAnnotationType != "float") || 
					   ($nextAnnotation->isBoolean() && !preg_match("/tinyint/", $storedAnnotationType)) || 
					   ($nextAnnotation->isDate() && $storedAnnotationType != "datetime") ||
					   ($nextAnnotation->isText() && $storedAnnotationType != "text") ||
					   ($nextAnnotation->isString() && !$nextAnnotation->isText() &&  $nextAnnotation->getMax() 
						&& !preg_match("/^varchar\({$nextAnnotation->getMax()}/", $storedAnnotationType)) ||
					   ($nextAnnotation->isString() && !$nextAnnotation->isText() && !$nextAnnotation->getMax() 
						&& !preg_match("/^varchar\(256/", $storedAnnotationType))){
						$typeChange = true;
					}
					
					//If property recently changed from collection add element column
					if(sizeof($tableDescription) == 3 && !$nextAnnotation->isCollection()){
						if($nextAnnotation->isObject()){
							$fieldName = strtolower($nextAnnotation->getObjectClass());
						}else{
							$fieldName = "value";
						}
						
						DatabaseAdministrator::execute("ALTER TABLE $tableName ADD COLUMN $fieldName $newColumnType DEFAULT NULL");						
					//If list type has become collection, drop element column
					}else if($nextAnnotation->isCollection() && sizeof($tableDescription) > 3){
						DatabaseAdministrator::execute("ALTER TABLE $tableName DROP COLUMN ".$tableDescription[3]['Field']);
					}
					
					
					$tableAbbr = substr(strtolower($tableName),0,15).substr(strtolower($tableName),strlen($tableName)-10,strlen($tableName));
					if($typeChange){
						//Set all values of old column to NULL to ensure compatability with new type
						DatabaseAdministrator::execute("UPDATE $tableName SET value=NULL");
						
						//If there is a key on the changed column drop it
						if($storedKey == "MUL"){
							$columnKey = "fk_".SettingsManager::getSetting("database","tableprefix")."_".$tableAbbr."_value";
							DatabaseAdministrator::execute("ALTER TABLE $tableName 
												DROP FOREIGN KEY $columnKey");
							DatabaseAdministrator::execute("ALTER TABLE $tableName DROP KEY $columnKey");	
						}
							
						if(sizeof($tableDescription) > 3){
							DatabaseAdministrator::execute("ALTER TABLE $tableName CHANGE value value $newColumnType");
						}
						
						//If new column is object or collection add Foreign Key to class table
						if($nextAnnotation->isObject()){
							$columnKey = "fk_".SettingsManager::getSetting("database","tableprefix")."_{$tableAbbr}_value";
							if($nextAnnotation->isObject()){;
								$this->manageTableForClass(Mirror::reflectClass($nextAnnotation->getObjectClass()));
								$foreignKeyTable = $nextAnnotation->getObjectClass();
							}
							
							DatabaseAdministrator::execute("ALTER TABLE $tableName ADD CONSTRAINT $columnKey 
												FOREIGN KEY(value)
												REFERENCES $foreignKeyTable(id)
												ON DELETE CASCADE ON UPDATE CASCADE");
						}
					}
				}
			}
		}
		
		/*
		 * Performs the SQL Query to either insert or update an object to the database
		 */
		 public function writeObject($saveObject){
			$saveObjectClass = Mirror::reflectClass($saveObject);
			$classHierarchy = $saveObjectClass->getClassHierarchy();
		
			//Remove parents of PersistentClass from hierarchy
			$persistentClass = Mirror::reflectClass("PersistentClass");
			$classHierarchy = array_diff($classHierarchy, $persistentClass->getClassHierarchy());
		
			//Load Class to Property Map
			$propertyMap = array();
			
			foreach($classHierarchy as $class){
				//Every class should include an ID field for linking to parent
				$propertyMap[$class->getName()] = array_merge(array(Mirror::reflectProperty($class->getName(), "id"),
											Mirror::reflectProperty($class->getName(), "created"),
											Mirror::reflectProperty($class->getName(), "modified")),
										$class->getProperties(false));		
			}
				
			//Check whether object already exists;
			$objectArray = DatabaseAdministrator::execute("SELECT id FROM ".get_class($saveObject)." WHERE id=".$saveObject->getId());
			$objectExists = (sizeof($objectArray) > 0);
							
			//Write each class component of the saveObject's hierarchy to the corresponding database table
			foreach($propertyMap as $class => $propertyList){
				$propertyValueMap = $this->buildPropertyValueMap($propertyList, $saveObject);
				$saveValues = array();
				
				if($objectExists){
					$saveQuery = "UPDATE $class SET ";
					foreach($propertyValueMap as $property => $value){
						$saveQuery .= "$property=?,";
						$saveValues[] = $value;
					}
					$saveQuery = substr($saveQuery, 0, strlen($saveQuery) - 1);
					$saveQuery .= " WHERE id=?";
					$saveValues[] = $saveObject->getId();
				}else{
					//Check if $saveObject has owning class, and whether owner has been processed
					if($saveObjectClass->hasOwningClass() && 
						!in_array($saveObjectClass->getOwningClass()->getName(), $this->classesSeen)){
						fatal_error(DATA_MODIFY_ERROR, "Object of class ".$saveObjectClass->getName(). 
								" cannot be saved independently. It must belong to a ".
								$saveObjectClass->getOwningClass()->getName() . " object.");
						return;
					}
					
					$saveQueryProperties = "";
					foreach(array_keys($propertyValueMap) as $property){
						$saveQueryProperties .= "$property,";
					}
					$saveQueryProperties = substr($saveQueryProperties, 0, strlen($saveQueryProperties) - 1);
					
					$saveQueryParameters = "";
					foreach($propertyValueMap as $value){
						$saveQueryParameters .= "?,";
						$saveValues[] = $value;
					}
					$saveQueryParameters = substr($saveQueryParameters, 0, strlen($saveQueryParameters) - 1);
					
					$saveQuery = "INSERT INTO $class($saveQueryProperties) VALUES($saveQueryParameters)";
				}
				
				//Execute the save query and pass in the extracted parameters
				call_user_func_array(array("DatabaseAdministrator","execute"), array_merge(array($saveQuery), $saveValues));
				
				//Save the object's collection properties
				foreach($propertyList as $property){
					if($property->getAnnotation()->isCollection()){
						$getterName = "get".capitalise($property->getName());
						$this->writeList($saveObject, $property, $saveObject->$getterName());
					}
				}
			}
		}
		
		/*
		 * Given an object and a specified list property, saves the value of the property to the database
		 */
		private function writeList($saveObject, $saveProperty, $propertyValue, $listDepth = 0, $parentListId = 0){
			if(!$propertyValue){
				$propertyValue = array();
			}
			
			$saveObjectClass = get_class($saveObject);
			$propertyName = $saveProperty->getName();
			$tableName = $saveObjectClass.capitalise($propertyName);
			$listAnnotation = $saveProperty->getAnnotation()->getContainedType();	
			
			//Load properties based on list depth
			for($i = 0; $i < $listDepth; $i++){
				$tableName .= "List";
				$listAnnotation = $listAnnotation->getContainedType();
			}		
			
			//Remove old elements from persisted list
			$listElementQuery = "SELECT * FROM $tableName WHERE ";
			if($listDepth == 0){
				$listElementQuery .= strtolower($saveObjectClass)."={$saveObject->getId()}";
			}else{
				$listElementQuery .= "parentList=$parentListId";
			}
			
			$listElementResultSet = DatabaseAdministrator::execute($listElementQuery);
			$oldElementIds = array();
			$storedListKeys = array();
			
			$listKeys = array();
			foreach(array_keys($propertyValue) as $listKey){
				$listKeys[] = strval($listKey);
			}
			
			foreach($listElementResultSet as $listElement){
				if(!in_array($listElement['name'], $listKeys, true)){
					$oldElementIds[] = $listElement['id'];
				}else{
					$storedListKeys[] = $listElement['name'];
				}
			}
			
			if(sizeof($oldElementIds) > 0){
				$storedElementDeleteQuery = "DELETE FROM $tableName WHERE id IN (";
				foreach($oldElementIds as $oldElementId){
					$storedElementDeleteQuery .= "?,";
				}
				$storedElementDeleteQuery = substr($storedElementDeleteQuery, 0, strlen($storedElementDeleteQuery) - 1);
				$storedElementDeleteQuery .= ")";
					
				call_user_func_array(array("DatabaseAdministrator", "execute"), 
							array_merge(array($storedElementDeleteQuery), $oldElementIds));
			}
			
			//Add new elements to persisted list
			$insertQuery = "INSERT INTO $tableName(name,";
			$updateQuery = "UPDATE $tableName SET ";
			
			if($listDepth == 0){
				$insertQuery .= strtolower($saveObjectClass);
				$parentKey = $saveObject->getId();
			}else{
				$insertQuery .= "parentList";
				$parentKey = $parentListId;
			}
			
			if($listAnnotation->isCollection()){
				$insertQuery .= ") VALUES(?,?)";
			}else{
				$insertQuery .= ",value) VALUES";
				$updateQuery .= "value=? ";
			}
						
			if($listDepth == 0){
				$updateQuery .= "WHERE name=? AND ".strtolower($saveObjectClass)."=?";
			}else{
				$updateQuery .= "WHERE name=? AND parentList=?";
			}
			
			$insertValues = array();
			
			foreach($propertyValue as $key => $element){
				$key = (string)$key;
				if($listAnnotation->isCollection()){
					if(!in_array($key, $storedListKeys)){
						DatabaseAdministrator::execute($insertQuery, $key, $parentKey);
						$lastInsertId = DatabaseAdministrator::getLastInsertId();
					}else{
						if($listDepth == 0){
							$idQuery = "SELECT id FROM $tableName WHERE name=? AND ".strtolower($saveObjectClass)."=?";
						}else{
							$idQuery = "SELECT id FROM $tableName WHERE name=? AND parentList=?";
						}
						
						$currentEntry = DatabaseAdministrator::execute($idQuery, $key, $parentKey);
						$lastInsertId = $currentEntry[0]['id'];
					}
					
					$this->writeList($saveObject, $saveProperty, $element, $listDepth + 1, $lastInsertId);
				}else{					
					if($listAnnotation->isObject()){
						if($element){
							$this->writeObject($element);
							$elementValue = $element->getId();
						}else{
							$elementValue = null;
						}
					}else if($listAnnotation->isDate()){
						$elementValue = "{$element->getDatestamp()} {$element->getTimestamp()}";
					}else{
						$elementValue = $element;
					}
					
					if(in_array($key, $storedListKeys)){
						DatabaseAdministrator::execute($updateQuery, $elementValue, $key, $parentKey);
					}else{
						$insertQuery .= "(?,?,?),";
						$insertValues = array_merge($insertValues, array($key, $parentKey, $elementValue));
					}
				}
			}
			
			if(sizeof($insertValues) > 0){
				$insertQuery = substr($insertQuery, 0, strlen($insertQuery) - 1);
				$insertArray = array_merge(array($insertQuery), $insertValues);
				call_user_func_array(array("DatabaseAdministrator","execute"), $insertArray);
			}
		}
		
		/*
		 * Returns a map of object properties to their values in the object
		 */
		private function buildPropertyValueMap($propertyList, $object){
			$propertyValueMap = array();
			foreach($propertyList as $property){
				$propertyName = $property->getName();
				$getterName = "get".capitalise($propertyName);
				$propertyValue = $object->$getterName();
				
				if($property->getAnnotation()->isCollection() || 
					(!$propertyValue && !$property->getAnnotation()->isBoolean() &&
						$property->getAnnotation()->isRequired())){
					continue;
				}
				
				if($propertyValue){
					if($property->getAnnotation()->isObject()){
						$this->writeObject($propertyValue);
						$propertyValueMap[$propertyName] = $propertyValue->getId();
					}else if($property->getAnnotation()->isDate()){
						$propertyValueMap[$propertyName] = "{$propertyValue->getDatestamp()} {$propertyValue->getTimestamp()}";
					}else if($property->getAnnotation()->isString()){
						$propertyValueMap[$propertyName] = (string)$propertyValue;
					}else{
						$propertyValueMap[$propertyName] = $propertyValue;
					}
				}else{
					$propertyValueMap[$propertyName] = null;
				}
			}
			
			return $propertyValueMap;
		}
		
		/*
		 * Determine the MySQL column type for each possible WadadliClass property type
		 */
		private function getColumnType($annotation){
			if($annotation->isInteger()){
				$columnType = "INT";
			}else if($annotation->isFloat() || $annotation->isMonetary()){
				$columnType = "FLOAT";
			}else if($annotation->isBoolean()){
				$columnType = "BOOLEAN";
			}else if($annotation->isDate()){
				$columnType = "DATETIME";
			}else if($annotation->isObject()){
				$columnType = "VARCHAR(20)";
			}else if($annotation->isText()){
				$columnType = "TEXT";
			}else if($annotation->isString()){
				$limit = 256;
				if($annotation->getMax()){
					$limit = $annotation->getMax();
				}
				
				$columnType = "VARCHAR($limit)";
			}else if($annotation->isCollection()){
				$columnType = "";
			}
			
			return $columnType;
		}
		
		/*
		 * Given raw data construct PersistentClass object
		 */
		 private function loadObject($class, $data){
			if(array_key_exists('id', $data) && QuarterMaster::release($data['id']) && 
				get_class(QuarterMaster::release($data['id'])) == $class->getName()){
					return QuarterMaster::release($data['id']);
			}
			
			$arguments = array();
			foreach(array_diff($class->getProperties(), Mirror::reflectClass('PersistentClass')->getProperties()) as $property){
				$field = $property->getName();
				
				if(!array_key_exists($field, $data)){
					$data[$field] = null;
				}
				
				if($property->getAnnotation()->isCollection()){
					$arguments[] = array();
				}else if($property->getAnnotation()->isDate()){
					$datePieces = preg_split("/[-: ]/", $data[$field]);
					$newDate = new WadadliDate();
					
					if(sizeof($datePieces) > 0){
						$newDate->setYear(intval($datePieces[0]));
					}
					
					if(sizeof($datePieces) > 1){
						$newDate->setMonth(intval($datePieces[1]));
					}
					
					if(sizeof($datePieces) > 2){
						$newDate->setDay(intval($datePieces[2]));
					}
					
					if(sizeof($datePieces) > 3){
						$newDate->setHours(intval($datePieces[3]));
					}
					
					if(sizeof($datePieces) > 4){
						$newDate->setMinutes(intval($datePieces[4]));
					}
					
					if(sizeof($datePieces) > 5){
						$newDate->setSeconds(intval($datePieces[5]));
					}
					
					$arguments[] = $newDate;
				}else if($property->getAnnotation()->isObject()){
					$containedClass = $property->getAnnotation()->getObjectClass();
					if(!array_key_exists($containedClass, $this->objectMembers)){
						$this->objectMembers[$containedClass] = array();
					}
					
					$arguments[] = null;
					if($data[$field]){
						if(!array_key_exists($data[$field],$this->objectMembers[$containedClass])){
							$this->objectMembers[$containedClass][$data[$field]] = array();
						}
						
						//Make note of contained object for delayed loading
						$this->objectMembers[$containedClass][$data[$field]][] = $data['id'];
					}
				}else if($property->getAnnotation()->isBoolean()){
					$arguments[] = (bool)$data[$field];
				}else if($property->getAnnotation()->isString()){
					$arguments[] = strval($data[$field]);
				}else{
					$arguments[] = $data[$field];
				}
			}
			
			$object = $class->newInstanceArgs($arguments);
			$object->setId($data['id']);
			$object->setModified($data['created']);
			$object->setModified($data['modified']);
			
			//Cache object before returning it
			QuarterMaster::store($object);
			return $object;
		 }
		 
		 /*
		  * Delayed loading of object collections to ensure a single query to each table
		  */
		 private function loadCollections($listTable, $listAnnotation, $parentIds, $parentColumn = "parentList"){
			$collections = array();
			
			$fields = "id,$parentColumn,name";
			if(!$listAnnotation->isCollection()){
				$fields .= ",value";
			}
			
			$results = array();
			if(DatabaseAdministrator::tableExists($listTable) && $parentIds){
				$results = DatabaseAdministrator::execute("SELECT $fields FROM $listTable WHERE $parentColumn IN (".implode(",",$parentIds).")");
			}
			
			if($listAnnotation->isObject()){
				$subGateway = new SQLGateway();
				$values = array();
				
				foreach($results as $result){
					$values[] = $result['value'];
				}
				
				$searcher = new Searcher();
				$searcher->addCriterion('id',Criterion::EQUAL,$values);
				$objects = $subGateway->find($listAnnotation->getObjectClass(), $searcher);
			}else if($listAnnotation->isCollection()){
				$ids = array();
				
				foreach($results as $result){
					$ids[] = $result['id'];
				}
				
				$nestedCollections = $this->loadCollections($listTable."List", $listAnnotation->getContainedType(), $ids);
			}
			
			foreach($results as $result){
				if(!array_key_exists($result[$parentColumn], $collections)){
					$collections[$result[$parentColumn]] = array();
				}
				
				if($listAnnotation->isDate()){
					$datePieces = preg_split("/[-: ]/", $result['value']);
					$value = new WadadliDate(intval($datePieces[0]),intval($datePieces[1]),intval($datePieces[2]),
										intval($datePieces[3]),intval($datePieces[4]),intval($datePieces[5]));
					
					$collections[$result[$parentColumn]][$result['name']] = $value;
				}else if($listAnnotation->isObject()){
					if($result['value']){
						$collections[$result[$parentColumn]][$result['name']] = $objects[$result['value']];
					}else{
						$objectClass = $listAnnotation->getObjectClass();
						$collections[$result[$parentColumn]][$result['name']] = null;
					}
				}else if($listAnnotation->isCollection()){
					if(array_key_exists($result['id'], $nestedCollections)){
						$collections[$result[$parentColumn]][$result['name']] = $nestedCollections[$result['id']];
					}else{
						$collections[$result[$parentColumn]][$result['name']] = array();
					}
				}else{
					$value = $result['value'];
					if($listAnnotation->isBoolean()){
						$value = boolval($value);
					}
					
					$collections[$result[$parentColumn]][$result['name']] = $value;
				}
			}
			
			return $collections;
		 }
	}
?>