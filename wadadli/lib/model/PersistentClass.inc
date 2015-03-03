<?php
	abstract class PersistentClass extends WadadliClass{
		/*
		 * Keeps track of IDs dispensed to new objects to avoid duplicates
		 */
		private static $dispensedIDs = array();
		
		/*
		 * A unique integer identifying this object from other instances of the same class.
		 */
		/** @String(required=true, min=0, max=20)*/
		protected $id;
		
		/*
		 * A unix timestamp representing when this element was created.
		 */
		/** @String */
		protected $created;
		
		/*
		 * A unix timestamp representing when this element last had one of its values changed.
		 */
		/** @String */
		protected $modified;
		
		/*
		 * An instance of the Gateway type used to load this object
		 */
		/** @Object(class='Gateway') */
		protected $gateway;

		/*
		 * Boolean flag indicating whether or not this object's data has been completed loaded.
		 */
		/** @Boolean(required=true, default=true) */
		protected $loaded;

		/*
		 * Constructer returns unbound instance of the object. An ID is automatically generated and will be
		 * used to refer to the object going forward.
		 */		
		public function __construct(){
			$arguments = func_get_args();
			
			//Add default gateway and loaded values to constructor arguments
			$arguments = array_merge(array(self::generateID(), strval(time()), strval(time()), null, true), $arguments);
			call_user_func_array(array('parent','__construct'), $arguments);
			
			QuarterMaster::store($this);
		}
		
		/*
		 * Pseudo-constructor that returns a bound version of the object with all its fields null. This
		 * class will be loaded when one of its fields is accessed.
		 */
		public static function bindInstance($id, $gateway){
			$currentClass = get_called_class();
			$instance = new $currentClass();
			
			$instance->setId($id);
			$instance->setGateway($gateway);
			$instance->setLoaded(false);
			
			return $instance;
		}
		  
		  /*
		   * Default persistent level behaviour that returns a PersistentCLass ID as its string representation
		   */
		  public function __toString(){
			return $this->getId();
		  }
		
		/*
		 * Overridden getX method to account for lazy loading
		 */
		 protected function getX($searchProperty){
			if(!in_array($searchProperty,array('id','gateway','loaded'))){
				$this->loadData();
			}
			
		 	return parent::getX($searchProperty);
		 }
		
		/*
		 * Overridden setX method to update last modified time
		 */
		 protected function setX($propertyName, $newValue, $calledMethod){
			$oldValue = $this->getX($propertyName);
			if($oldValue && $newValue != $oldValue){
				$this->modified = strval(time());
			}
			
			if(!$this->modified){
				$this->modified = strval(time());
			}
			
			return parent::setX($propertyName, $newValue, $calledMethod);
			
		 }
		 
		 /*
		  * Returns an object's creation time object based on its ID
		  */
		 protected function getCreationTime(){
			$createTime = $this->getCreated();
			return new WadadliDate(intval(date('Y',$createTime)),intval(date('m',$createTime)),intval(date('d',$createTime)),
						intval(date('G',$createTime)),intval(date('i',$createTime)),intval(date('s',$createTime)));
		 }
		 
		 /*
		  * Generates a unique ID number for the current object based on the current Unix timestamp
		  */
		  private static function generateID(){
			$id = number_format(microtime(true)*10000,0,'','');
			
			do{
				$id++;
			}while(in_array($id, self::$dispensedIDs));
			
			self::$dispensedIDs[] = $id;
			return strval($id);
		  }

		/*
		 * If an instance of the object has been bound but not loaded.
		 */
		 private function loadData(){
			if($this->loaded || !$this->getGateway()){
				return;
			}
			
			$className = get_class($this);
			$object = $this->gateway->findUnique($className, 'id', $this->getId());
			
			if(!$object){
				fatal_error(DATA_ACCESS_ERROR, "Instance of class ".get_class($this)." has an invalid binding ".
								"(Gateway: ".get_class($this->getGateway()).", ID: {$this->getId()}).");
				return;
			}
			
			$class = Mirror::reflectClass($className);
			foreach($class->getProperties(false) as $property){
				$propertyName = capitalise($property->getName());
				
				$setMethod = "set$propertyName";
				$getMethod = "get$propertyName";
				
				$this->$setMethod($object->$getMethod());	
			}
			
			$this->loaded = true;
		}
	}
?>