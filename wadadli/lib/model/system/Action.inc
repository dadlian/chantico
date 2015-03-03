<?php
	class Action extends PersistentClass{
		/*
		 * The unique name that identifies this action
		 */
		/** @String(required=true) */
		protected $name;
		
		/*
		 * The URL that invokes this particular action
		 */
		/** @URL */
		protected $path;
		
		/*
		 * The name that identifies this action and the controller that handles it
		 */
		/** @String(required=true) */
		protected $controller;
		
		/*
		 * A list of user types that have permission to execute this action
		 */
		/** @Collection(type=@String) */
		protected $roles;
		
		/*
		 * A list of the expected parameters to this action
		 */
		/** @Collection(type=@String) */
		protected $parameters;
		
		public function __toString(){
			return $this->getPath();
		}
	}
?>