<?php
	abstract class ObjectController extends Controller{
		/** @String(required=true,default='PersistentClass') */
		protected $objectClass;
		
		/** @Integer(required=true,default=1000) */
		protected $pageLength;
		
		/** @Collection(type=@Object(class='Criterion')) */
		protected $sortCriteria;
		
		/** @Collection(type=@Object(class='Criterion')) */
		protected $searchCriteria;
		
		private $sqlGateway;
		
		public function __construct(){
			call_user_func_array(array('parent','__construct'),func_get_args());
			$this->initialiseObjectClass();
			
			$this->sqlGateway = new SQLGateway();
		}
		
		public function execute(){
			$action = "search";
			if(array_key_exists("action",$this->getArguments())){
				$action = $this->viewFromArguments("action");
			}
			
			if(!in_array($action,array("search","retrieve","modify","delete"))){
				$action = "search";
			}
			
			$this->$action();
		}
		
		protected function search(){
			$searcher = new Searcher();
			foreach($this->getSearchCriteria() as $criterion){
				$searcher->appendToCriteria($criterion);
			}
			
			$sorter = new Sorter();
			foreach($this->getSortCriteria() as $criterion){
				$sorter->appendToCriteria($criterion);
			}
			
			$page = 0;
			if(array_key_exists("page",$this->getArguments())){
				$page = $this->viewFromArguments("page") - 1;
			}
			
			$objects = $this->sqlGateway->find($this->getObjectClass(),$searcher,$sorter,$this->getPageLength(),$page*$this->getPageLength());
			$this->onSearch($objects);
		}
		
		protected function retrieve(){
			$searcher = new Searcher();
			if($_POST && array_key_exists(strtolower($this->getObjectClass()),$_POST)){
				$searcher->addCriterion('id',Criterion::EQUAL,$_POST[strtolower($this->getObjectClass())]);
			}
			
			$object = $this->sqlGateway->findUnique($this->getObjectClass(),$searcher);
			$this->onRetrieve($object);
		}
		
		protected function modify(){
			$searcher = new Searcher();
			if($_POST && array_key_exists(strtolower($this->getObjectClass()),$_POST)){
				$searcher->addCriterion('id',Criterion::EQUAL,$_POST[strtolower($this->getObjectClass())]);
				$object = $this->sqlGateway->findUnique($this->getObjectClass(),$searcher);
			}else if($this->viewFromArguments(strtolower($this->getObjectClass()))){
				$searcher->addCriterion('id',Criterion::EQUAL,$this->viewFromArguments(strtolower($this->getObjectClass())));
				$object = $this->sqlGateway->findUnique($this->getObjectClass(),$searcher);
			}else{
				$object = null;
			}
			
			$newObject = ($object)?false:true;
			
			if($this->buildObject($object)){
				$this->sqlGateway->save($object);
				Postman::postSuccess(decamelise($this->getObjectClass(),true)." '$object' was successfully saved.");
			
				$this->onModify($object,true,$newObject);
			}else{
				$this->onModify($object,false,$newObject);
			}
		}
		
		protected function delete(){
			$objectID = $this->viewFromArguments(strtolower($this->getObjectClass()));
			if($objectID){
				$searcher = new Searcher();
				$searcher->addCriterion('id',Criterion::EQUAL,$objectID);
				
				$sqlGateway = new SQLGateway();
				$object = $this->sqlGateway->findUnique($this->getObjectClass(),$searcher);
				$sqlGateway->delete($object);
				
				Postman::postSuccess(decamelise($this->getObjectClass(),true)." '$object' was deleted successfully.");
				$this->onDelete($object,true);
			}else{
				Postman::postWarning(decamelise($this->getObjectClass(),true)." $objectID was not found.");
				$this->onDelete($object,false);
			}
		}
		
		protected function buildObject(&$object){
			$objectClass = Mirror::reflectClass($this->getObjectClass());
			
			if(!$object){
				$object = $objectClass->newInstance();
			}
			
			if(!$_POST){
				return false;
			}
			
			$success = true;
			$fileFields = array();
			$objectProperties = array_diff($objectClass->getProperties(),Mirror::reflectClass('PersistentClass')->getProperties());
			foreach($objectProperties as $property){
				$fieldName = decapitalise($property->getName());
				$annotation = $property->getAnnotation();
				
				$realAnnotation = $property->getAnnotation();
				while($realAnnotation->isCollection()){
					$realAnnotation = $realAnnotation->getContainedType();
				}
				
				if(array_key_exists($fieldName, $_POST)){
					$postValue = $_POST[$fieldName];
				}else if(array_key_exists($fieldName, $_FILES)){
					$postValue = $_FILES[$fieldName];
				}else if($annotation->isBoolean()){
					$postValue = false;
				}else if($annotation->isCollection() && !$realAnnotation->isFile()){
					$getter = "get".capitalise($property->getName());
					$postValue = $object->$getter()?$object->$getter():array();
				}else{
					continue;
				}
				
				if($realAnnotation->isFile()){
					$fileFields[$property->getName()] = $annotation;
				}else{
					$fieldValue = $this->validateValue($annotation,$postValue,$property->getName());
					$fieldSuccess = false;
				
					$setter = "set".capitalise($property->getName());
					if(!is_null($fieldValue) || $property->getAnnotation()->isObject()){
						if(!$annotation->isCollection() && $realAnnotation->isInteger()){
							$object->$setter((int)$fieldValue);
						}else{
							$object->$setter($fieldValue);
						}
						
						$fieldSuccess = true;
					}else if($property->getAnnotation()->isRequired()){
						warning("Please specify the {$objectClass->getName()}'s required ".
								strtolower(decamelise($property->getName())).".","");
					}else{
						warning("The entered ".strtolower(decamelise($property->getName())).
								" is invalid. Verify the value and try again.","");
					}
					
					if($fieldSuccess && $realAnnotation->isPassword()){
						$confirmFieldname = "confirm".capitalise($fieldName);
						if(!array_key_exists($confirmFieldname, $_POST) || $postValue != $_POST[$confirmFieldname]){
							warning("The entered ".strtolower(decamelise($property->getName())).
								"s do not match. Please enter them again.","");
							$fieldSuccess = false;
						}
					}
					
					$success = $success && $fieldSuccess;
				}
			}
			
			//Upload Object Files
			if($success){
				$filesUploaded = array();
				$newImages = array();
				foreach($fileFields as $fieldName => $fieldAnnotation){
					$fileData = array();
					$fileDelete = array();
					if(!is_array($_FILES[camelise($fieldName)]['name'])){
						$fileData = array($_FILES[camelise($fieldName)]);
						
						if($fieldAnnotation->isImage()){
							$fileDelete = array($_POST[camelise($fieldName)."Delete"]);
						}else{
							$fileDelete = array_pad($fileDelete,sizeof($fileData),"");
						}
					}else{
						foreach($_FILES[camelise($fieldName)] as $fileField => $values){
							for($i=0; $i < sizeof($values); $i++){
								if(sizeof($fileData) <= $i){
									$fileData[$i] = array();
								}
								
								$fileData[$i][$fileField] = $values[$i];
							}
						}
					}
					
					$setter = "set".capitalise($fieldName);
					$appender = "appendTo".capitalise($fieldName);
					$getter = "get".capitalise($fieldName);
					
					$currentImages = $object->$getter();
					if(!is_array($currentImages)){
						$currentImages = array($currentImages);
					}
					
					for($i=0; $i<sizeof($fileData); $i++){
						$uploadedFile = null;
						if($fileDelete[$i] && sizeof($currentImages) > $i){
							$uploadedFile = "";
							
							if($currentImages[$i]){
								unlink(SettingsManager::getSetting('install','projectpath')."/{$currentImages[$i]}");
								$currentImages[$i] = null;
							}
						}else if($fileData[$i]['name']){
							if($uploadedFile = $this->uploadFile($fileData[$i])){
								$newImages[] = $uploadedFile;
							}else{
								$success = false;
								break;
							}
						}else if(!$currentImages[$i] && $fieldAnnotation->isRequired()){
							warning("Please specify the {$objectClass->getName()}'s required ".
									strtolower(decamelise($fieldName)).".","");
							$success = false;
						}
						
						if(!is_null($uploadedFile)){
							if($fieldAnnotation->isImage() && $uploadedFile){
								$dimensions = getimagesize(SettingsManager::getSetting('install','projectpath')."/$uploadedFile");
								if(($fieldAnnotation->getWidth() && $fieldAnnotation->getWidth() != $dimensions[0]) ||
									($fieldAnnotation->getHeight() && $fieldAnnotation->getHeight() != $dimensions[1])){
										warning("The ".$objectClass->getName()." ".decamelise($fieldName,true)." ".
												"must be {$fieldAnnotation->getWidth()}px wide by ".
												"{$fieldAnnotation->getHeight()}px high.","");
										$success = false;
								}
							}
							
							if($success){
								$filesUploaded[$fieldName] = $uploadedFile;
							}
							
							if($currentImages[$i] && $success){
								unlink(SettingsManager::getSetting('install','projectpath')."/{$currentImages[$i]}");
							}
						}
					}
				}
				
				if($success){
					foreach($filesUploaded as $fieldName => $uploadedFile){
						$setter = "set".capitalise($fieldName);
						$appender = "appendTo".capitalise($fieldName);
						
						if($fieldAnnotation->isFile()){
							$object->$setter($uploadedFile);
						}else{
							$object->$appender($uploadedFile);
						}
					}
				}else{
					foreach($newImages as $newImage){
						unlink(SettingsManager::getSetting("Install","Project Path")."/$newImage");
					}
				}
			}
			
			return $success;
		}
		
		private function validateValue($annotation,$postValue,$propertyName){
			$fieldValue = null;
			$minValue = $annotation->getMin()?$annotation->getMin():0;
			$maxValue = $annotation->getMax()?$annotation->getMax():PHP_INT_MAX;
			
			if($annotation->isBoolean()){
				$fieldValue = $postValue?true:false;
			}else if(!$annotation->isRequired() || $postValue){
				if($annotation->isDate()){
					if(!$postValue){
						$fieldValue = new WadadliDate();
					}else if(preg_match("/^[0-9]{4}-[0-9]{2}-[0-9]{2}( [0-9]{2}:[0-9]{2}:[0-9]{2})?$/",$postValue)){
						$datePieces = preg_split("/[-:\s]/",$postValue);
						
						$hour = (sizeof($datePieces) > 3)?intval($datePieces[3]):0;
						$minute = (sizeof($datePieces) > 3)?intval($datePieces[4]):0;
						$second = (sizeof($datePieces) > 3)?intval($datePieces[5]):0;
						
						$fieldValue = new WadadliDate(intval($datePieces[0]),intval($datePieces[1]),intval($datePieces[2]),
											$hour,$minute,$second);
					}
				}else if($annotation->isURL()){
					if(!$postValue){
						$fieldValue = "";
					}else if(filter_var((preg_match("/^http:\/\//",$postValue)?"":"http://").$postValue,FILTER_VALIDATE_URL)){
						$fieldValue = $postValue;
					}
				}else if($annotation->isEmail()){
					if(!$postValue){
						$fieldValue = "";
					}else if(filter_var($postValue,FILTER_VALIDATE_EMAIL)){
						$fieldValue = $postValue;
					}
				}else if($annotation->isPhone()){
					if(!$postValue){
						$fieldValue = "";
					}else if(preg_match("/^\+?[0-9\(\)\-\s]+$/",$postValue)){
						$fieldValue = $postValue;
					}
				}else if($annotation->isPassword()){
					if(!$postValue){
						$fieldValue = "";
					}else if(is_string($postValue)){
						$fieldValue = md5($postValue);
					}
				}else if($annotation->isString()){
					if(!$postValue){
						$fieldValue = "";
					}else if(is_string($postValue)){
						$fieldValue = htmlspecialchars_decode($postValue);
					}
				}else if($annotation->isNumeric()){
					if(!$postValue){
						$fieldValue = 0;
					}else if(is_numeric($postValue)){
						if($annotation->isInteger()){
							$fieldValue = intval($postValue);
						}else{
							$fieldValue = floatval($postValue);
						}
					}
				}else if($annotation->isObject()){
					$objectClass = Mirror::reflectClass($annotation->getObjectClass());
					$searcher = new Searcher();
					$searcher->addCriterion('id',Criterion::EQUAL,$postValue);
					
					if(!$postValue){
						$fieldValue = $objectClass->newInstance();
					}else if($object = $this->sqlGateway->findUnique($objectClass->getName(),$searcher)){
						$fieldValue = $object;
					}
				}else if($annotation->isCollection()){
					$fieldValue = array();
					if($postValue){
						if(!is_array($postValue)){
							$postValue = array($postValue);
						}
						
						foreach(array_slice($postValue,0,null,true) as $key => $element){
							if(!$element){
								continue;
							}
							
							$validatedValue = $this->validateValue($annotation->getContainedType(), $element,$propertyName);
							if(!is_null($validatedValue)){
								$fieldValue[$key] = $validatedValue;
							}else{
								$fieldValue[$key] = null;
								break;
							}
						}
					}
					
					if(sizeof($fieldValue) < $minValue || sizeof($fieldValue) > $maxValue){
						if($maxValue == PHP_INT_MAX){
							$expectsText = "$minValue or more";
						}else{
							$expectsText = "between $minValue and $maxValue";
						}
					
						warning("{$this->getObjectClass()} expects $expectsText $propertyName. ".sizeof($fieldValue)." given.","");
						$fieldValue = null;
					}
				}
			}
			
			//Ensure valued properties are validateValue
			if($fieldValue && $annotation->isValued() && $annotation->getValues() && !in_array($fieldValue,$annotation->getValues())){
				$fieldValue = null;
			}
			
			//Ensure ranged properties are not out-of-bounds
			if($fieldValue && $annotation->isRanged()){
				if(($annotation->isString() && (strlen($fieldValue) < $minValue || strlen($fieldValue) > $maxValue)) ||
					($annotation->isNumeric() && ($fieldValue < $minValue || $fieldValue > $maxValue)) ||
					($annotation->isCollection() && (sizeof($fieldValue) < $minValue || sizeof($fieldValue) > $maxValue))){
					
					$fieldValue = null;
				}
			}
			
			return $fieldValue;
		}
		
		protected function addSearchCriterion($field,$condition,$value){
			if(!is_array($value)){
				$value = array($value);
			}
			
			$this->appendToSearchCriteria(new Criterion($field,$condition,$value));
		}
		
		protected function clearSearchCriteria(){
			$this->setSearchCriteria(array());
		}
		
		protected function addSortCriterion($field,$condition){
			$this->appendToSortCriteria(new Criterion($field,$condition));
		}
		
		protected function clearSortCriteria(){
			$this->setSortCriteria(array());
		}
		
		protected function getPageCount(){
			if(DatabaseAdministrator::tableExists($this->getObjectClass())){
				$results = DatabaseAdministrator::execute("SELECT COUNT(id) AS count FROM {$this->getObjectClass()}");
				$pageCount = (int)ceil($results[0]['count']/$this->getPageLength());
			}else{
				$pageCount = 0;
			}
			
			return $pageCount;
		}
		
		abstract protected function initialiseObjectClass();
		abstract protected function onSearch($objectList);
		abstract protected function onRetrieve($object);
		abstract protected function onModify($object,$success,$new);
		abstract protected function onDelete($object,$success);
	}
?>