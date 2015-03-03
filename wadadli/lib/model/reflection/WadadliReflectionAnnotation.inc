<?php
	class WadadliReflectionAnnotation{
		const COLLECTION = "Collection";
		const OBJECT = "Object";
		const STRING = "String";
		const BOOLEAN = "Boolean";
		const DATE = "Date";
		const INTEGER = "Integer";
		const FLOAT = "Float";
		const MONETARY = "Monetary";
		const URL = "URL";
		const EMAIL = "Email";
		const PHONE = "Phone";
		const PASSWORD = "Password";
		const FILE = "File";
		const IMAGE = "Image";
		const TEXT = "Text";
		
		//The annotation's type
		private $type;
		
		//Stores whether the annotation is required or not
		private $required;
		
		//Annotation's default value (if applicable)
		private $default;
		
		//Annotation's class (if applicable)
		private $objectClass;
		
		//The annotation type to be contained in a collection
		private $containedType;
		
		//The annotation values array
		private $values;
		
		//The annotation max value
		private $max;
		
		//The annotation min value
		private $min;
		
		//The annotation height (for images)
		private $height;
		
		//The annotation width (for images)
		private $width;
		
		//Contains the project path for easy reference
		private static $projectPath;
		
		public function __construct($annotation){
			$annotationClass = null;
			if($annotation){
				$annotationClass = Mirror::reflectClass($annotation);
			}else{
				fatal_error(CONFIGURATION_ERROR, "A class property is annotated with a non-existant class or is not well-formed. ".
									"Please check your class annotations and try again.");
				return;
			}
			
			$settingsFile = simplexml_load_file(dirname(__FILE__)."/../../../conf/settings.xml");
			$projectPath = $settingsFile->settingslist->settings[0]->values->projectpath;
			
			$this->type = $annotationClass;
			
			//Initialise annotation object class property
			$this->objectClass = null;
			if($this->isObject()){
				if($annotation->class && class_exists($annotation->class)){
					$this->objectClass = $annotation->class;
				}else{
					fatal_error(CONFIGURATION_ERROR, "The specified class for an Object annotation is not defined or does not exist. ".
										"Please ensure defined classes are used in annotations.");
					return;
				}
			}
			
			//Initialise annotation collection type property
			$this->containedType = null;
			if($this->isCollection()){
				if($annotation->type){
					$this->containedType = new WadadliReflectionAnnotation($annotation->type);
				}else{
					fatal_error(CONFIGURATION_ERROR, "The specified type for a Collection annotation is invalid or does not exist. ".
										"Please ensure valid annotations are used as collection types.");
					return;
				}
			}
			
			//Initialise annotation required property
			$this->required = false;
			if($annotation->required){
				if(is_bool($annotation->required)){
					$this->required = $annotation->required;
				}else{
					fatal_error(CONFIGURATION_ERROR, "An annotation's 'required' value is not a boolean, as required.");
					return;
				}
			}
			
			//Initialise Annotation default property
			$this->default = null;
			if($this->isDefaulted() && ($annotation->default || is_bool($annotation->default))){
				if($this->isString() && !is_string($annotation->default) ||
				   $this->isUrl() && !filter_var((preg_match("/^http:\/\//",$annotation->default)?"":"http://").$annotation->default,
									FILTER_VALIDATE_URL) ||
				   $this->isEmail() && !filter_var($annotation->default,FILTER_VALIDATE_EMAIL) ||
				   $this->isPhone() && !preg_match("/\+?[0-9\s\-\)\(]+/",$annotation->default) ||
				   $this->isFile() && !file_exists(self::$projectPath.$annotation->default) ||
				   $this->isInteger() && !is_int($annotation->default) ||
				   $this->isBoolean() && !is_bool($annotation->default) ||
				   ($this->isFloat() || $this->isMonetary()) && !is_numeric($annotation->default)){
					fatal_error(CONFIGURATION_ERROR,"An annotation's default value must match its declared type. ".
									"Default value is ".gettype($annotation->default).
									" but declared type is {$this->type->getName()}.");
					return;
				}
				
				$this->default = $annotation->default;
			}
			
			//Initialise annotation value array property
			$this->values = array();
			if($this->isValued() && $annotation->values){
				if(!is_array($annotation->values)){
					fatal_error(CONFIGURATION_ERROR, 
							"An annotation's enumerated values must be given as an array, ".gettype($annotation->values)." given.");
					return;
				}
				
				foreach($annotation->values as $value){
					if($this->isString() && !is_string($value) ||
					   $this->isUrl() && !filter_var((preg_match("/^http:\/\//",$value)?"":"http://").$value,FILTER_VALIDATE_URL) ||
					   $this->isEmail() && !filter_var($value,FILTER_VALIDATE_EMAIL) ||
					   $this->isPhone() && !preg_match("/\+?[0-9\s\-\)\(]+/",$value) ||
					   $this->isFile() && !file_exists(self::$projectPath.$value) ||
					   $this->isInteger() && !is_int($value) ||
					   $this->isBoolean() && !is_bool($value) ||
					   ($this->isFloat() || $this->isMonetary()) && !is_numeric($value)){
						fatal_error(CONFIGURATION_ERROR,
								"One or more of a {$this->type->getName()} annotation's enumerated values does not match its ".
								"declared type, ".gettype($value)." value given.");
						return;
					}		
				}
				
				$this->values = $annotation->values;
			}
			
			//Initialise annotation min and max properties
			$this-> max = null;
			if($this->isRanged() && $annotation->max){
				if(!is_numeric($annotation->max)){
					fatal_error(CONFIGURATION_ERROR, "An annotation's max value must be numeric, ".gettype($annotation->max)." given.");
					return;
				}
				
				$this->max = $annotation->max;
			}
			
			$this-> min = null;
			if($this->isRanged() && $annotation->min){
				if(!is_numeric($annotation->min)){
					fatal_error(CONFIGURATION_ERROR, "An annotation's min value must be numeric, ".gettype($annotation->min)." given.");
					return;
				}
				
				$this->min = $annotation->min;
			}
			
			//Initialise annotation width and height properties
			$this->height = null;
			if($this->isImage() && $annotation->height){
				if(!is_numeric($annotation->height)){
					fatal_error(CONFIGURATION_ERROR, "An image annotation's height must be numeric, ".gettype($annotation->height)." given.");
					return;
				}
				
				$this->height = $annotation->height;
			}
			
			$this->width = null;
			if($this->isImage() && $annotation->width){
				if(!is_numeric($annotation->width)){
					fatal_error(CONFIGURATION_ERROR, "An image annotation's width must be numeric, ".gettype($annotation->width)." given.");
					return;
				}
				
				$this->width = $annotation->width;
			}
			
			if(!self::$projectPath){
				$settingsFile = simplexml_load_file(dirname(__FILE__)."/../../../conf/settings.xml");
				self::$projectPath = $settingsFile->settingslist->settings[0]->values->projectpath;
			}
		}
		
		public function getType(){
			return $this->type->getName();
		}
		
		public function isRequired(){
			return $this->required;
		}
		
		public function getDefault(){
			return $this->default;
		}
		
		public function getObjectClass(){
			return $this->objectClass;
		}
		
		public function getContainedType(){			
			return $this->containedType;
		}
		
		public function getValues(){			
			return $this->values;
		}
		
		public function getMax(){			
			return $this->max;
		}
		
		public function getMin(){			
			return $this->min;
		}
		
		public function getHeight(){			
			return $this->height;
		}
		
		public function getWidth(){			
			return $this->width;
		}
		
		public function isCollection(){
			return $this->getType() == self::COLLECTION;	
		}
		
		public function isObject(){
			return $this->getType() == self::OBJECT;	
		}
		
		public function isBoolean(){
			return $this->getType() == self::BOOLEAN;	
		}
		
		public function isDate(){
			return $this->getType() == self::DATE;	
		}
		
		public function isInteger(){
			return $this->getType() == self::INTEGER;	
		}
		
		public function isFloat(){
			return $this->getType() == self::FLOAT;	
		}
		
		public function isMonetary(){
			return $this->getType() == self::MONETARY;
		}
		
		public function isUrl(){
			return $this->getType() == self::URL;
		}
		
		public function isEmail(){
			return $this->getType() == self::EMAIL;
		}
		
		public function isPhone(){
			return $this->getType() == self::PHONE;
		}
		
		public function isPassword(){
			return $this->getType() == self::PASSWORD;
		}
		
		public function isFile(){
			return $this->type->descendsFrom(self::FILE);
		}
		
		public function isImage(){
			return $this->getType() == self::IMAGE;
		}
		
		public function isText(){
			return $this->getType() == self::TEXT;
		}
		
		public function isNumeric(){
			return ($this->isInteger() || $this->isFloat() || $this->isMonetary());
		}
		
		public function isString(){
			return $this->type->descendsFrom(self::STRING);
		}
		
		public function isDefaulted(){
			return $this->type->descendsFrom('DefaultedAnnotation');
		}
		
		public function isValued(){
			return $this->type->descendsFrom('ValuedAnnotation');
		}
		
		public function isRanged(){
			return $this->type->descendsFrom('RangedAnnotation');
		}
	}
?>