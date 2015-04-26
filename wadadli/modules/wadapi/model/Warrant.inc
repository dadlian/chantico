<?php
	class Warrant extends PersistentClass{
		/** @String */
		protected $code;
		
		/** @String */
		protected $action;
		
		/** @Boolean(default=false) */
		protected $executed;
	}
?>