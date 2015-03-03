<?php
	class User extends PersistentClass{
		/** @String(required=true) */
		protected $firstName;
		
		/** @String(required=true) */
		protected $lastName;
		
		/** @String(required=true) */
		protected $username;
		
		/** @Password(required=true) */
		protected $password;
		
		/** @String(required=true) */
		protected $role;
		
		/** @Boolean(required=true) */
		protected $disabled;
		
		public function __toString(){
			return "{$this->getUsername()}";
		}
	}
?>