<?php
	class Mason extends Worker{
		public static function buildObjectTable($class,$objects,$actions = array(),$excludes = array(),$actionArguments = array(),$asynchronise=false,
								$target="",$callback="",$callbackArg=""){
			if(!is_string($class)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"Mason buildObjectTable expects string class, ".gettype($class)." given");
				return;
			}else if(!class_exists($class)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"Mason buildObjectTable cannot find class $class.");
				return;
			}
		
			if(!is_array($objects)){
				$objects = array($objects);
			}
			
			if(!is_array($actions)){
				$actions = array($actions);
			}
			
			if(!is_array($excludes)){
				$excludes = array($excludes);
			}
			
			$classProperties = array_diff(Mirror::reflectClass($class)->getProperties(),Mirror::reflectClass('PersistentClass')->getProperties());
			
			$filteredProperties = array();
			$headings = array();
			foreach($classProperties as $property){
				if(!in_array($property->getName(), $excludes)){
					$filteredProperties[] = $property;
					$headings[] = capitalise(preg_replace("/([A-Z])/"," $1",$property->getName()));
				}
			}
			$classProperties = $filteredProperties;
			
			$rows = sizeof($objects);
			$columns = sizeof($classProperties) + sizeof($actions);
			$objectTable = new WadadliTable("{$class}ObjectTable", $rows, $columns, $headings);
			
			$rowIndex = 0;
			$columnClasses = array();
			
			foreach($objects as $object){
				if(!is_object($object) || get_class($object) != $class){
					continue;
				}
				
				$columnIndex = 0;
				
				foreach($classProperties as $property){
					$getter = "get".capitalise($property->getName());
					$formattedProperty = self::getPropertyDisplayComponent($property->getAnnotation(),$object->$getter());
					$objectTable->addContent($rowIndex,$columnIndex,$formattedProperty);
					$columnClasses[$columnIndex] = $property->getName();
					
					$columnIndex++;
				}
				
				foreach($actions as $actionName => $action){
					$actionLink = new WadadliActionLink("",$actionName,$action);
					$linkArguments = array_key_exists($actionName,$actionArguments)?$actionArguments[$actionName]:array();
					
					foreach($linkArguments as $argument){
						$actionLink->addArgument($argument);
					}
					
					$actionLink->addArgument($object->getId());
					
					if($asynchronise){
						$actionLink->asynchronise($target,$callback,$callbackArg);
					}
					
					$objectTable->addContent($rowIndex, $columnIndex, $actionLink);
					$columnClasses[$columnIndex] = decapitalise($actionName);
					$columnIndex++;
				}
				
				$rowIndex++;
			}
			
			foreach($columnClasses as $columnIndex => $class){
				$objectTable->addColumnClass($class,array($columnIndex));
			}
			
			return $objectTable;
		}
		
		public static function buildObjectListing($object,$excludes=array()){
			if(!is_object($object)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"Mason buildObjectListing expects WadadliClass object, ".gettype($object)." given");
				return;
			}
			
			$class = Mirror::reflectClass($object);
			if(!$class->descendsFrom('WadadliClass')){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"Mason buildObjectListing expects WadadliClass object, "
										.getclass($object)." is not a WadadliClass.");
				return;
			}
			
			if(!is_array($excludes)){
				$excludes = array($excludes);
			}
			
			$objectListing = new WadadliListing("{$class->getName()}{$object->getId()}Listing");
			$objectListing->addClass('objectListing');
			
			foreach(array_diff($class->getProperties(),Mirror::reflectClass('PersistentClass')->getProperties()) as $property){
				if(in_array($property->getName(),$excludes)){
					continue;
				}
			
				$annotation = $property->getAnnotation();
				$getter = "get".capitalise($property->getName());
			
				$fieldTitle = decamelise($property->getName(),true);
				$fieldValue = self::getPropertyDisplayComponent($annotation, $object->$getter());
				
				$objectListing->addItem($fieldTitle,$fieldValue);
			}
			
			return $objectListing;
		}
		
		public static function buildObjectForm($object,$submitAction="",$cancelAction="",$sections=array(),$excludes=array(),
							$submitArguments=array(),$cancelArguments=array(),$submitText='Save',$cancelText='Cancel'){
			if(!is_object($object)){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"Mason buildObjectForm expects WadadliClass object, ".gettype($object)." given");
				return;
			}
			
			$class = Mirror::reflectClass($object);
			if(!$class->descendsFrom('PersistentClass')){
				fatal_error(UNEXPECTED_ARGUMENT_WARNING,"Mason buildObjectForm expects WadadliClass object, "
										.get_class($object)." is not a WadadliClass.");
				return;
			}
			
			if(!is_string($submitAction)){
				$submitAction = "";
			}
			
			if(!is_string($cancelAction)){
				$cancelAction = "";
			}
			
			if(!is_array($sections)){
				$sections = array($sections);
			}
			
			if(!is_array($excludes)){
				$excludes = array($excludes);
			}
			
			if(!is_array($submitArguments)){
				$submitArguments = array($submitArguments);
			}
			
			if(!is_array($cancelArguments)){
				$cancelArguments = array($cancelArguments);
			}
			
			$sectionedProperties = array();
			foreach($sections as $section => $properties){
				foreach($properties as $property){
					if(is_string($property)){
						$sectionedProperties[$property] = $section;
					}
				}
			}
			
			$formTitle = "New ".decamelise($class->getName(),true);
			if((string)$object){
				$formTitle = (string)$object;
			}
			
			$objectForm = new WadadliForm("{$object->getId()}Form",$formTitle,decamelise("$submitText {$class->getName()}", true),
							$submitAction,$cancelText,$cancelAction);
			$objectForm->addInput(new WadadliHiddenField(decapitalise($class->getName()),$object->getId()));
			
			foreach(array_diff($class->getProperties(),Mirror::reflectClass('PersistentClass')->getProperties()) as $property){
				if(in_array($property->getName(),$excludes)){
					continue;
				}
			
				$annotation = $property->getAnnotation();
				$getter = "get".capitalise($property->getName());
			
				$fieldTitle = decamelise($property->getName(),true);
				$fieldValue = $object->$getter();
				
				$inputSection = "";
				if(array_key_exists($property->getName(),$sectionedProperties)){
					$inputSection = $sectionedProperties[$property->getName()];
				}
				$objectForm->addInput(self::getPropertyInputComponent($annotation,$fieldTitle,$fieldValue),$inputSection);
			}
			
			foreach($submitArguments as $submitArgument){
				$objectForm->addSubmitArgument($submitArgument);
			}
			
			foreach($cancelArguments as $cancelArgument){
				$objectForm->addCancelArgument($cancelArgument);
			}
			
			return $objectForm;
		}
		
		private static function getPropertyDisplayComponent($annotation, $value){
			$propertyComponent = new WadadliStringComponent("");
			if(!is_bool($value) && !$value){
				return $propertyComponent;
				
			}
			
			if($annotation->isBoolean()){
				if($value){
					$propertyComponent = "Yes";
				}else{
					$propertyComponent = "No";
				}
			}else if($annotation->isMonetary()){
				$countryCode = SettingsManager::getSetting('localisation','country');
				
				if($countryCode == "GB"){
					$currency = "&pound;";
				}else{
					$currency = "\$";
				}
				
				$propertyComponent = sprintf("$currency%.2f",$value);
			}else if($annotation->isDate()){
				$propertyComponent = $value->getDatestamp();
			}else if($annotation->isUrl()){
				$propertyComponent = new WadadliAnchor("",$value,$value,false);
			}else if($annotation->isEmail()){
				$propertyComponent = new WadadliEmailAnchor("",$value,$value);
			}else if($annotation->isImage()){
				$propertyComponent = new WadadliImageComponent("",$value);
			}else if($annotation->isFile()){
				$propertyComponent = new WadadliAnchor("",$file,$value);
			}else if($annotation->isText()){
				$propertyComponent = new WadadliContainer();
				foreach(explode("\n",$value) as $contentPiece){
					$paragraph = new WadadliContainer();
					$paragraph->setTag('p');
					$paragraph->addContent(new WadadliStringComponent($contentPiece));
					$propertyComponent->addContent($paragraph);
				}
			}else if($annotation->isCollection()){
				$propertyComponent = new WadadliListComponent();
				foreach($value as $element){
					$propertyComponent->addItem(self::getPropertyDisplayComponent($annotation->getContainedType(),$element));
				}
			}else{
				$propertyComponent = (string)$value;
			}
			
			if(is_string($propertyComponent)){
				return new WadadliStringComponent($propertyComponent);
			}
			
			return $propertyComponent;
		}
		
		private static function getPropertyInputComponent($annotation, $title, $value="",$dropDownList=array()){
			$isRequired = $annotation->isRequired();
			
			if($annotation->isDate()){
				$default = $value;
			}else if(!$annotation->isCollection()){
				$default = (string)($value?$value:($annotation->isDefaulted()?$annotation->getDefault():""));
			}
			
			if($annotation->isBoolean()){
				$propertyInput = new WadadliCheckbox($title,camelise($title),$isRequired,(bool)$default);
			}else if($annotation->isDate()){
				$propertyInput = new WadadliDatePicker($title,$default->getDatestamp(),$isRequired,date('Y-m-d'),new WadadliDate(),new WadadliDate(),false);
			}else if($annotation->isImage()){
				if(!$default){
					$default = "wadadli/img/default.png";
				}
				$propertyInput = new WadadliImageField($title,$default,$isRequired);
			}else if($annotation->isFile()){				
				$propertyInput = new WadadliFileField($title,$default,$isRequired);
			}else if($annotation->isText()){
				$optional = $isRequired?"":"n optional";
				$maxLength = $annotation->getMax()?$annotation->getMax():0;
				$propertyInput = new WadadliTextBox($title,$default,$isRequired,"Enter a$optional ".strtolower($title)."...",$maxLength);
			}else if($annotation->isPassword()){
				$propertyInput = new WadadliPasswordConfirmationField($title,$default,$isRequired);
			}else if($annotation->isObject()){
				$containedClass = Mirror::reflectClass($annotation->getObjectClass());
				if($containedClass->descendsFrom('PersistentClass')){
					if(!$dropDownList){
						$sqlGateway = new SQLGateway();
						foreach($sqlGateway->find($containedClass->getName()) as $containedObject){
							$dropDownList[$containedObject->getId()] = "$containedObject";
						}
					}
				
					$propertyInput = new WadadliDropDown($title,$default,$isRequired,$dropDownList);
				}else{
					$propertyInput = new WadadliTextField($title,$default,$isRequired,"Enter ".strtolower($containedClass->getName())." ID.");
				}
			}else if($annotation->isCollection()){
				$propertyInput = new WadadliInputContainer($title);
				
				if($annotation->getContainedType()->isCollection()){
					$elementTitle = "$title Element";
				}else{
					$elementTitle = "{$title}[]";
				}
				
				$templateField = self::getPropertyInputComponent($annotation->getContainedType(),camelise($elementTitle));
				$templateField->setComponentId(decapitalise("{$title}Template"));
				$templateField->toggleLabel();
				$templateField->addClass("hidden");
				$propertyInput->addContent($templateField);
				
				if($value){
					$dropDownList = array();
					if($annotation->getContainedType()->isObject()){
						$sqlGateway = new SQLGateway();
						foreach($sqlGateway->find($annotation->getContainedType()->getObjectClass()) as $containedObject){
							$dropDownList[$containedObject->getId()] = "$containedObject";
						}
					}
				
					$counter = 0;
					foreach($value as $element){
						$fieldContainer = new WadadliContainer();
						$fieldContainer->addClass('objectFormCollectionElement');
						
						$nextField = self::getPropertyInputComponent($annotation->getContainedType(),$elementTitle,$element,$dropDownList);
						$nextField->toggleLabel();
						$fieldContainer->addContent($nextField);
						
						if($counter > 0 || !$isRequired){
							$removeElementLink = new WadadliAnchor("","Remove");
							$removeElementLink->addClass("removeObjectCollectionElementLink");
							$removeElementLink->onClick('removeObjectFormCollectionElement');
							$fieldContainer->addContent($removeElementLink);
						}
						
						$counter++;
						$propertyInput->addContent($fieldContainer);
					}
				}else if($isRequired){
					$fieldContainer = new WadadliContainer();
					$fieldContainer->addClass('objectFormCollectionElement');
					
					$nextField = self::getPropertyInputComponent($annotation->getContainedType(),$elementTitle);
					$nextField->toggleLabel();
					
					$fieldContainer->addContent($nextField);
					$propertyInput->addContent($fieldContainer);
				}
				
				$newElementLink = new WadadliAnchor("","+ Add to $title");
				$newElementLink->addClass('addObjectCollectionElementLink');
				$newElementLink->onClick('addObjectFormCollectionElement',array(camelise($title)));
				$propertyInput->addContent($newElementLink);
			}else if($annotation->isValued() && $annotation->getValues()){
				$propertyInput = new WadadliDropDown($title,$default,$isRequired);
				foreach($annotation->getValues() as $annotationValue){
					$propertyInput->addOption($annotationValue);
				}
			}else{
				$propertyInput = new WadadliTextField($title,$default,$isRequired);
			}
			
			$propertyInput->addClass(decapitalise($annotation->getType())."FieldInput");
			return $propertyInput;
		}
	}
?>