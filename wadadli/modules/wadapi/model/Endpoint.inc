<?php
	class Endpoint extends PersistentClass{
		/*
		 * The unique name that identifies this endpoint
		 */
		/** @String(required=true) */
		protected $name;
		
		/*
		 * The URL that invokes this particular endpoint
		 */
		/** @URL */
		protected $path;
		
		/*
		 * The name that identifies this endpoint and the controller that handles it
		 */
		/** @String(required=true) */
		protected $controller;
		
		/*
		 * A list of user types that have permission to execute this endpoint
		 */
		/** @Collection(type=@String) */
		protected $roles;
		
		/*
		 * A list of the expected parameters to this endpoint
		 */
		/** @Collection(type=@String) */
		protected $parameters;
		
		/*
		 * A list of the required arguments to this endpoint
		 */
		/** @Collection(type=@String) */
		protected $requirements;
		
		public function __toString(){
			return $this->getPath();
		}
	}
?>