<?php
	class Session extends PersistentClass{
		/** @String */
		protected $sessionId;
		
		/** @String */
		protected $ipAddress;
		
		/** @Object(class='User') */
		protected $activeUser;
	}
?>