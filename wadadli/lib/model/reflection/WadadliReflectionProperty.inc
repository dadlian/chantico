<?php
	class WadadliReflectionProperty extends ReflectionAnnotatedProperty{		
		//Contains a class representation of the object's annotation
		private $annotation;
		
		//Contains the project path for easy reference
		private static $projectPath;
		
		/*
		 * Initialises class level members and then passes handling to parent
		 */
		public function __construct(){
			@call_user_func_array(array("parent", "__construct"), func_get_args());
			$annotations = $this->getAllAnnotations();
		 	
		 	if(sizeof($annotations) > 0){
				$this->annotation = new WadadliReflectionAnnotation($annotations[0]);
		 	}else{
				fatal_error(CONFIGURATION_ERROR, "Property '{$this->getName()}' of class ".
								"'{$this->getDeclaringClass()->getName()}' does not have a valid annotation.");
				return;
			}
			
			if(!self::$projectPath){
				$settingsFile = simplexml_load_file(dirname(__FILE__)."/../../../conf/settings.xml");
				self::$projectPath = $settingsFile->settingslist->settings[2]->values->projectpath;
			}
		}
		
		public function getAnnotation($annotation=""){
			return $this->annotation;
		}
		 
		//Return true if $value is among the properties $values array
		public function isEnumeratedValue($value, $annotation = null){
			if(!$annotation || !is_object($annotation) || get_class($annotation) != 'WadadliReflectionAnnotation'){
				$annotation = $this->annotation;
			}
			
			$validValues = $annotation->getValues();
			
			//If the property does not support value enumeration, or there are no values specified, all $values are valid.
			if(sizeof($validValues) == 0){
				return true;
			}else{
				return in_array($value, $validValues, true);
			}
		}
		
		public function inRange($value, $annotation = null){
			if(!$annotation || !is_object($annotation) || get_class($annotation) != 'WadadliReflectionAnnotation'){
				$annotation = $this->annotation;
			}
			
			//If the property does not support ranges all $values are in range. Return true.
			if(!$annotation->isRanged()){
				return true;
			}
			
			//If the value is not numeric we can't perform a comparison
			if(is_object($value) || is_bool($value)){
				return false;
			}
			
			$min = $annotation->getMin();
			$max = $annotation->getMax();
			
			//If the $value is for a String or Collection property, assess its length.
			if($annotation->isString()){
				$value = strlen($value);
			}else if($annotation->isCollection()){
				$value = sizeof($value);
				
				if(!is_int($value)){
					$min = $max = 0;
				}
			}

			if(is_null($min) || $value >= $min){
				$aboveMin = true;
			}else{
				$aboveMin = false;
			}
			
			if(is_null($max) || $value <= $max){
				$belowMax = true;
			}else{
				$belowMax = false;
			}
			
			
			return $aboveMin && $belowMax;
		}
		
		/*
		 * Given a $value determines whether it is the right type for the property
		 */
		public function isValidType($value, $annotation = null){
			if(!$annotation || !is_object($annotation) || get_class($annotation) != 'WadadliReflectionAnnotation'){
				$annotation = $this->annotation;
			}
			
			if(($annotation->isCollection() && !is_null($value) && !$this->isValidCollection($value, $annotation->getContainedType()))
				|| ($annotation->isObject() && !is_null($value) && (!is_object($value)
						|| strtolower(get_class($value)) != strtolower($annotation->getObjectClass())))
				|| ($annotation->isString() && !is_string($value))
				|| ($annotation->isUrl() && $value && !filter_var((preg_match("/^http:\/\//",$value)?"":"http://").$value,FILTER_VALIDATE_URL))
				|| ($annotation->isEmail() && $value && !filter_var($value,FILTER_VALIDATE_EMAIL))
				|| ($annotation->isPhone() && $value && !preg_match("/^\+?[0-9\(\)\-\s]+$/",$value))
				|| ($annotation->isFile() && $value && !file_exists(self::$projectPath."$value"))
				|| ($annotation->isInteger() && !is_int($value))
				|| (($annotation->isFloat() || $annotation->isMonetary()) && !is_numeric($value))
				|| ($annotation->isBoolean() && !is_bool($value))
				|| ($annotation->isDate() && !is_null($value) && (!is_object($value) || get_class($value) != "WadadliDate"))  
					&& $value != null){
					if($annotation->isObject() && !is_null($value) && is_object($value)){
						$valueObject = Mirror::reflectClass($value);
						if($valueObject->descendsFrom($annotation->getObjectClass())){
							return true;
						}
					}
					
					return false;
			}
			
			return true;
		}
		
		/*
		 * Checks whether the given value meets all the validity criteria for this property
		 */
		public function isValidValue($value, $annotation = null){			
			return $this->isValidType($value,$annotation) && $this->isEnumeratedValue($value,$annotation) && $this->inRange($value,$annotation);
		}
		
		/*
		 * Given an array, verify that all elements match the collection type
		 */
		private function isValidCollection($collection, $annotation){
			if(!is_array($collection)){
				return false;
			}
			
			foreach($collection as $element){
				//Check individual elements for constraint criteria
				if(!is_null($element) && !$this->isValidValue($element, $annotation)){
					return false;
				}
			}
			
			return true;
		}
	}
?>